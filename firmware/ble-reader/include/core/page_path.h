#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace pocket {

bool is_valid_book_id(std::string_view id);
std::string book_directory(std::string_view id);
std::string book_meta_path(std::string_view id);
std::string page_path(std::string_view id, std::size_t page);

}  // namespace pocket
