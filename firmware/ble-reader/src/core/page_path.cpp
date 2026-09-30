#include "core/page_path.h"

#include <cctype>

namespace pocket {

bool is_valid_book_id(const std::string_view id) {
  if (id.empty() || id.size() > 24) {
    return false;
  }
  for (const char character : id) {
    const unsigned char byte = static_cast<unsigned char>(character);
    if (!std::isalnum(byte) && character != '-' && character != '_') {
      return false;
    }
  }
  return true;
}

std::string book_directory(const std::string_view id) {
  return "/book/" + std::string(id);
}

std::string book_meta_path(const std::string_view id) {
  return book_directory(id) + "/meta.json";
}

std::string page_path(const std::string_view id, const std::size_t page) {
  return book_directory(id) + "/p" + std::to_string(page) + ".bin";
}

}  // namespace pocket
