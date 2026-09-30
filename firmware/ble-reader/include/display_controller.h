#pragma once

#include <GxEPD2_BW.h>

#include <cstddef>
#include <string_view>
#include <vector>

#include "core/protocol_service.h"

namespace pocket {

class DisplayController {
 public:
  DisplayController();

  void begin();
  bool draw_page(std::string_view id, std::size_t page, bool partial);
  void draw_menu(const std::vector<BookInfo>& books, std::size_t selected,
                 int battery_percent);
  void draw_receiving();
  void hibernate();

 private:
  GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> display_;
};

}  // namespace pocket
