#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace pocket {

enum class Operation {
  Info,
  BeginBook,
  BeginPage,
  EndBook,
  DeleteBook,
  Open,
  Wifi,
  Window,
};

struct ControlCommand {
  Operation operation = Operation::Info;
  std::string id;
  std::string title;
  std::size_t pages = 0;
  std::size_t page = 0;
  std::size_t length = 0;
  std::uint32_t crc32 = 0;
  bool enabled = false;
  std::size_t from = 0;
  std::size_t to = 0;
};

struct ParseResult {
  bool ok = false;
  ControlCommand command;
  std::string error;
};

ParseResult parse_control_json(std::string_view json);

}  // namespace pocket
