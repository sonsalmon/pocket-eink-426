#include "core/http_page.h"

#include <ArduinoJson.h>

#include <cstdint>
#include <limits>

#include "core/page_path.h"

namespace pocket {
namespace {

bool read_decimal(const std::string_view text, std::uint32_t& value) {
  if (text.empty()) return false;
  value = 0;
  for (const char digit : text) {
    if (digit < '0' || digit > '9') return false;
    const auto next = static_cast<std::uint32_t>(digit - '0');
    if (value > (std::numeric_limits<std::uint32_t>::max() - next) / 10) {
      return false;
    }
    value = value * 10 + next;
  }
  return true;
}

}  // namespace

bool build_http_page_command(const std::string_view book,
                             const std::string_view page,
                             const std::string_view crc,
                             const std::size_t length,
                             std::string& command) {
  std::uint32_t number = 0;
  std::uint32_t checksum = 0;
  command.clear();
  if (!is_valid_book_id(book) || length == 0 ||
      !read_decimal(page, number) || !read_decimal(crc, checksum)) {
    return false;
  }
  JsonDocument document;
  document["op"] = "begin_page";
  document["book"] = std::string(book);
  document["n"] = number;
  document["len"] = length;
  document["crc32"] = checksum;
  serializeJson(document, command);
  return true;
}

}  // namespace pocket
