#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace pocket {

bool build_http_page_command(std::string_view book, std::string_view page,
                             std::string_view crc, std::size_t length,
                             std::string& command);

}  // namespace pocket
