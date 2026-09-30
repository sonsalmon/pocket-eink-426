#pragma once

#include <string>
#include <vector>

#include "core/protocol_service.h"

namespace pocket {

class DisplayController {
 public:
  inline static std::vector<std::size_t> drawn_pages;
  void begin() {}
  bool draw_page(const std::string&, std::size_t page, bool) {
    drawn_pages.push_back(page);
    return true;
  }
  void draw_receiving() {}
  void draw_menu(const std::vector<BookInfo>&, std::size_t, int) {}
  void hibernate() {}
};

class WifiTransport : public WifiControl {
 public:
  void attach(ProtocolService&) {}
  void loop() {}
  WifiInfo enable() override { return {}; }
  void disable() override {}
};

class BleTransport {
 public:
  inline static std::vector<std::string> notifications;
  inline static ProtocolService* service = nullptr;
  explicit BleTransport(ProtocolService& protocol) { service = &protocol; }
  void begin() {}
  bool connected() const { return true; }
  void notify_json(const std::string& json) { notifications.push_back(json); }
};

}  // namespace pocket

struct NimBLEDevice {
  static void deinit(bool) {}
};
