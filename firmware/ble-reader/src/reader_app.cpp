#include "reader_app.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <driver/gpio.h>
#include <esp_sleep.h>

#include <algorithm>

namespace pocket {
namespace {

constexpr int kLadderPin = 1;
constexpr int kPowerPin = 3;
constexpr int kBatteryPin = 0;
constexpr unsigned long kDebounceMs = 5;
constexpr unsigned long kCenterHoldMs = 650;
constexpr unsigned long kDeepSleepMs = 5UL * 60UL * 1000UL;

}  // namespace

ReaderApp::ReaderApp()
    : store_([this](const std::string& id, const std::size_t page) {
        open_page(id, page);
      }, [this]() {
        books_ = store_.list_books();
        if (screen_ == Screen::Reading) {
          emit_need(store_.need_request(current_book_, current_page_));
        }
      }),
      protocol_(store_, wifi_),
      ble_(protocol_) {
  wifi_.attach(protocol_);
}

void ReaderApp::begin() {
  Serial.begin(115200);
  pinMode(kLadderPin, INPUT);
  pinMode(kPowerPin, INPUT_PULLUP);
  pinMode(kBatteryPin, INPUT);
  analogSetAttenuation(ADC_11db);

  display_.begin();
  if (!store_.begin()) {
    Serial.println("LittleFS mount failed");
  }
  ble_.begin();
  last_activity_at_ = millis();

  std::string id;
  std::size_t page = 0;
  if (store_.load_last(id, page)) {
    store_.open_book(id, page);
  } else {
    show_menu();
  }
}

void ReaderApp::loop() {
  const unsigned long now = millis();
  wifi_.loop();
  const bool power_down = digitalRead(kPowerPin) == LOW;
  if (power_down && !power_pressed_) {
    power_pressed_ = true;
    power_pressed_at_ = now;
  } else if (!power_down && power_pressed_) {
    power_pressed_ = false;
    if (now - power_pressed_at_ >= kDebounceMs) {
      sleep_deep();
    }
  }

  const Key current = decode_ladder_mv(analogReadMilliVolts(kLadderPin));
  if (current != sampled_key_) {
    sampled_key_ = current;
    sampled_at_ = now;
  }
  if (now - sampled_at_ >= kDebounceMs && sampled_key_ != stable_key_) {
    const Key released = stable_key_;
    stable_key_ = sampled_key_;
    if (stable_key_ != Key::None) {
      pressed_at_ = now;
      center_hold_fired_ = false;
      last_activity_at_ = now;
    } else if (!center_hold_fired_) {
      handle_short_press(released);
    }
  }
  if (stable_key_ == Key::Center && !center_hold_fired_ &&
      now - pressed_at_ >= kCenterHoldMs) {
    center_hold_fired_ = true;
    handle_long_press(Key::Center);
  }
  if (!ble_.connected() && now - last_activity_at_ >= kDeepSleepMs) {
    sleep_deep();
  }
  if (light_sleep_pending_ && !ble_.connected() &&
      stable_key_ == Key::None) {
    light_sleep_pending_ = false;
    sleep_light();
  }
  delay(2);
}

void ReaderApp::open_page(const std::string& id, const std::size_t page) {
  books_ = store_.list_books();
  if (!store_.has_page(id, page)) {
    current_book_ = id;
    current_page_ = page;
    screen_ = Screen::Reading;
    display_.draw_receiving();
    emit_need(store_.need_request(id, page));
    return;
  }
  const bool partial = screen_ == Screen::Reading && current_book_ == id;
  if (display_.draw_page(id, page, partial)) {
    current_book_ = id;
    current_page_ = page;
    screen_ = Screen::Reading;
    last_activity_at_ = millis();
    light_sleep_pending_ = true;
    emit_need(store_.need_request(id, page));
  }
}

void ReaderApp::show_menu() {
  books_ = store_.list_books();
  if (selected_book_ >= books_.size()) {
    selected_book_ = books_.empty() ? 0 : books_.size() - 1;
  }
  screen_ = Screen::Menu;
  display_.draw_menu(books_, selected_book_, battery_percent());
}

void ReaderApp::handle_short_press(const Key key) {
  if (key == Key::Left) {
    if (screen_ == Screen::Reading) {
      change_page(-1);
    } else if (selected_book_ > 0) {
      --selected_book_;
      show_menu();
    }
  } else if (key == Key::Right) {
    if (screen_ == Screen::Reading) {
      change_page(1);
    } else if (selected_book_ + 1 < books_.size()) {
      ++selected_book_;
      show_menu();
    }
  } else if (key == Key::Center) {
    if (screen_ == Screen::Reading) {
      display_.draw_page(current_book_, current_page_, false);
    } else if (!books_.empty()) {
      const BookInfo selected = books_[selected_book_];
      store_.open_book(selected.id, selected.last);
    }
  }
}

void ReaderApp::handle_long_press(const Key key) {
  if (key == Key::Center) {
    show_menu();
  }
}

void ReaderApp::change_page(const int delta) {
  books_ = store_.list_books();
  const auto selected =
      std::find_if(books_.begin(), books_.end(), [this](const BookInfo& book) {
        return book.id == current_book_;
      });
  const std::size_t pages =
      selected == books_.end() ? 0 : selected->pages;
  const long candidate = static_cast<long>(current_page_) + delta;
  if (candidate >= 0 && static_cast<std::size_t>(candidate) < pages) {
    store_.open_book(current_book_, static_cast<std::size_t>(candidate));
  }
}

void ReaderApp::emit_need(const NeedRange& range) {
  if (!range.needed) {
    return;
  }
  JsonDocument document;
  document["ev"] = "need";
  document["book"] = current_book_;
  document["from"] = range.from;
  document["to"] = range.to;
  std::string json;
  serializeJson(document, json);
  ble_.notify_json(json);
}

void ReaderApp::sleep_deep() {
  display_.hibernate();
  NimBLEDevice::deinit(true);
  esp_deep_sleep_enable_gpio_wakeup(1ULL << kPowerPin,
                                    ESP_GPIO_WAKEUP_GPIO_LOW);
  esp_deep_sleep_start();
}

void ReaderApp::sleep_light() {
  gpio_wakeup_enable(GPIO_NUM_1, GPIO_INTR_LOW_LEVEL);
  gpio_wakeup_enable(GPIO_NUM_3, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  esp_sleep_enable_timer_wakeup(250000);
  esp_light_sleep_start();
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO);
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
}

int ReaderApp::battery_percent() const {
  const int battery_mv = analogReadMilliVolts(kBatteryPin) * 2;
  return std::clamp((battery_mv - 3300) * 100 / 900, 0, 100);
}

}  // namespace pocket
