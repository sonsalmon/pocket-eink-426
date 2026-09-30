// Pocket426 bench test: same GPIOs as the real board (see hardware/gen/design.py).
//   < / >  : previous / next page with a partial refresh (time shown on screen)
//   o short: full refresh      o long: ladder calibration screen
#include <Arduino.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <GxEPD2_BW.h>
#include <SPI.h>

namespace pins {
constexpr int SCLK = 8, MOSI = 10, CS = 21, DC = 4, RST = 5, BUSY = 6;
constexpr int LADDER = 1, POWER = 3;
}  // namespace pins

GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> display(
    GxEPD2_426_GDEQ0426T82(pins::CS, pins::DC, pins::RST, pins::BUSY));

enum class Key { None, Left, Confirm, Right };

// Same windows as the FreeInk SDK OnePageAdcLadder decoder used by the firmware.
Key decode(int mv) {
  if (mv >= 1780 && mv <= 2140) return Key::Left;
  if (mv >= 1140 && mv <= 1500) return Key::Right;
  if (mv >= 0 && mv <= 250) return Key::Confirm;
  return Key::None;
}

constexpr unsigned long DEBOUNCE_MS = 5;
constexpr unsigned long HOLD_MS = 650;

int page = 1;
unsigned long lastPartialMs = 0, lastFullMs = 0;

void drawPage(bool partial) {
  display.setRotation(0);
  if (partial) {
    display.setPartialWindow(0, 0, display.width(), display.height());
  } else {
    display.setFullWindow();
  }
  const unsigned long t0 = millis();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeSans18pt7b);
    display.setCursor(30, 60);
    display.printf("Pocket426 bench  -  page %d", page);
    display.setFont(&FreeSans9pt7b);
    for (int line = 0; line < 14; line++) {
      display.setCursor(30, 110 + line * 24);
      display.printf("%02d  The quick brown fox jumps over the lazy dog. 0123456789", line + 1);
    }
    display.setCursor(30, 460);
    display.printf("last partial %lu ms   last full %lu ms", lastPartialMs, lastFullMs);
  } while (display.nextPage());
  (partial ? lastPartialMs : lastFullMs) = millis() - t0;
  Serial.printf("page %d %s refresh %lu ms\n", page, partial ? "partial" : "full", millis() - t0);
}

void drawCalibration() {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.setCursor(30, 40);
    display.print("Ladder calibration: hold each key; values go to the serial log.");
    display.setCursor(30, 70);
    display.print("Expected: < 1.98 V   > 1.34 V   o 0 V   idle 3.3 V");
  } while (display.nextPage());
  for (int i = 0; i < 100; i++) {
    const int mv = analogReadMilliVolts(pins::LADDER);
    Serial.printf("ladder %4d mV -> %d\n", mv, static_cast<int>(decode(mv)));
    delay(100);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(pins::LADDER, INPUT);
  pinMode(pins::POWER, INPUT_PULLUP);
  analogSetAttenuation(ADC_11db);
  SPI.begin(pins::SCLK, -1, pins::MOSI, pins::CS);
  display.init(115200, true, 2, false);
  drawPage(false);
}

void loop() {
  static Key stable = Key::None, sampled = Key::None;
  static unsigned long sampledAt = 0, pressedAt = 0;
  static bool holdFired = false;

  const Key now = decode(analogReadMilliVolts(pins::LADDER));
  if (now != sampled) {
    sampled = now;
    sampledAt = millis();
  }
  if (millis() - sampledAt <= DEBOUNCE_MS) return;

  if (sampled != stable) {
    const Key released = stable;
    stable = sampled;
    if (stable != Key::None) {
      pressedAt = millis();
      holdFired = false;
      return;
    }
    if (holdFired) return;
    if (released == Key::Left && page > 1) {
      page--;
      drawPage(true);
    } else if (released == Key::Right) {
      page++;
      drawPage(true);
    } else if (released == Key::Confirm) {
      drawPage(false);
    }
    return;
  }

  if (stable == Key::Confirm && !holdFired && millis() - pressedAt >= HOLD_MS) {
    holdFired = true;
    drawCalibration();
    drawPage(false);
  }
}
