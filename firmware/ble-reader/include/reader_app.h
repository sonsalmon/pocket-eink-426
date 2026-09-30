#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "ble_transport.h"
#include "book_store.h"
#include "core/ladder.h"
#include "core/protocol_service.h"
#include "display_controller.h"
#include "wifi_transport.h"

namespace pocket {

class ReaderApp {
 public:
  ReaderApp();

  void begin();
  void loop();

 private:
  enum class Screen { Reading, Menu };

  void open_page(const std::string& id, std::size_t page);
  void show_menu();
  void handle_short_press(Key key);
  void handle_long_press(Key key);
  void change_page(int delta);
  void emit_need(const NeedRange& range);
  void sleep_light();
  void sleep_deep();
  int battery_percent() const;

  DisplayController display_;
  BookStore store_;
  WifiTransport wifi_;
  ProtocolService protocol_;
  BleTransport ble_;
  Screen screen_ = Screen::Menu;
  std::string current_book_;
  std::size_t current_page_ = 0;
  std::vector<BookInfo> books_;
  std::size_t selected_book_ = 0;
  Key sampled_key_ = Key::None;
  Key stable_key_ = Key::None;
  unsigned long sampled_at_ = 0;
  unsigned long pressed_at_ = 0;
  unsigned long last_activity_at_ = 0;
  unsigned long power_pressed_at_ = 0;
  bool center_hold_fired_ = false;
  bool power_pressed_ = false;
  bool light_sleep_pending_ = false;
};

}  // namespace pocket
