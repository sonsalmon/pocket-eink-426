#include "display_controller.h"

#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <LittleFS.h>
#include <SPI.h>

#include <cstdint>
#include <vector>

#include "core/inflate.h"
#include "core/page_path.h"

namespace pocket {
namespace {

constexpr int kSclkPin = 8;
constexpr int kMosiPin = 10;
constexpr int kCsPin = 21;
constexpr int kDcPin = 4;
constexpr int kResetPin = 5;
constexpr int kBusyPin = 6;
constexpr std::size_t kPageBytes = 800U * 480U / 8U;

}  // namespace

DisplayController::DisplayController()
    : display_(GxEPD2_426_GDEQ0426T82(kCsPin, kDcPin, kResetPin, kBusyPin)) {}

void DisplayController::begin() {
  SPI.begin(kSclkPin, -1, kMosiPin, kCsPin);
  display_.init(115200, true, 2, false);
  display_.setRotation(0);
}

bool DisplayController::draw_page(const std::string_view id,
                                  const std::size_t page,
                                  const bool partial) {
  fs::File file = LittleFS.open(page_path(id, page).c_str(), FILE_READ);
  if (!file) {
    return false;
  }
  std::vector<std::uint8_t> compressed(file.size());
  if (file.read(compressed.data(), compressed.size()) != compressed.size()) {
    return false;
  }
  std::vector<std::uint8_t> bitmap(kPageBytes);
  if (!inflate_raw(compressed.data(), compressed.size(), bitmap.data(),
                   bitmap.size())) {
    return false;
  }

  if (partial) {
    display_.setPartialWindow(0, 0, display_.width(), display_.height());
  } else {
    display_.setFullWindow();
  }
  display_.firstPage();
  do {
    display_.fillScreen(GxEPD_WHITE);
    display_.drawBitmap(0, 0, bitmap.data(), 800, 480, GxEPD_BLACK);
  } while (display_.nextPage());
  return true;
}

void DisplayController::draw_menu(const std::vector<BookInfo>& books,
                                  const std::size_t selected,
                                  const int battery_percent) {
  display_.setFullWindow();
  display_.firstPage();
  do {
    display_.fillScreen(GxEPD_WHITE);
    display_.setTextColor(GxEPD_BLACK);
    display_.setFont(&FreeSans18pt7b);
    display_.setCursor(28, 44);
    display_.print("Books");
    display_.setFont(&FreeSans12pt7b);
    display_.setCursor(665, 40);
    display_.printf("%d%%", battery_percent);

    if (books.empty()) {
      display_.setCursor(28, 100);
      display_.print("Send a book over Bluetooth.");
    } else {
      const std::size_t first = selected > 3 ? selected - 3 : 0;
      const std::size_t last =
          std::min<std::size_t>(books.size(), first + 8);
      for (std::size_t index = first; index < last; ++index) {
        const int y = 92 + static_cast<int>(index - first) * 46;
        display_.setCursor(28, y);
        display_.print(index == selected ? "> " : "  ");
        display_.print(books[index].id.c_str());
        display_.printf("  (%u pages)",
                        static_cast<unsigned>(books[index].pages));
      }
    }
  } while (display_.nextPage());
}

void DisplayController::draw_receiving() {
  display_.setFullWindow();
  display_.firstPage();
  do {
    display_.fillScreen(GxEPD_WHITE);
    display_.setTextColor(GxEPD_BLACK);
    display_.setFont(&FreeSans18pt7b);
    display_.setCursor(90, 220);
    display_.print("Receiving next pages from phone");
  } while (display_.nextPage());
}

void DisplayController::hibernate() { display_.hibernate(); }

}  // namespace pocket
