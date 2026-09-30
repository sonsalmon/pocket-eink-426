#include "core/protocol.h"

#include <ArduinoJson.h>

#include <limits>

#include "core/page_path.h"

namespace pocket {
namespace {

ParseResult failure(const char* error) {
  return ParseResult{false, ControlCommand{}, error};
}

bool read_id(JsonVariantConst value, std::string& destination) {
  if (!value.is<const char*>()) {
    return false;
  }
  destination = value.as<const char*>();
  return is_valid_book_id(destination);
}

bool read_size(JsonVariantConst value, std::size_t& destination,
               const bool allow_zero) {
  if (!value.is<std::uint64_t>()) {
    return false;
  }
  const std::uint64_t number = value.as<std::uint64_t>();
  if ((!allow_zero && number == 0) ||
      number > std::numeric_limits<std::size_t>::max()) {
    return false;
  }
  destination = static_cast<std::size_t>(number);
  return true;
}

}  // namespace

ParseResult parse_control_json(const std::string_view json) {
  JsonDocument document;
  const DeserializationError parse_error =
      deserializeJson(document, json.data(), json.size());
  if (parse_error) {
    return failure("json");
  }

  const JsonVariantConst op_value = document["op"];
  if (!op_value.is<const char*>()) {
    return failure("op");
  }
  const std::string_view op = op_value.as<const char*>();
  ControlCommand command;

  if (op == "info") {
    command.operation = Operation::Info;
  } else if (op == "begin_book") {
    command.operation = Operation::BeginBook;
    if (!read_id(document["id"], command.id) ||
        !document["title"].is<const char*>() ||
        !read_size(document["pages"], command.pages, false)) {
      return failure("args");
    }
    command.title = document["title"].as<const char*>();
  } else if (op == "begin_page") {
    command.operation = Operation::BeginPage;
    std::size_t crc = 0;
    if (!read_id(document["book"], command.id) ||
        !read_size(document["n"], command.page, true) ||
        !read_size(document["len"], command.length, false) ||
        !read_size(document["crc32"], crc, true) ||
        crc > std::numeric_limits<std::uint32_t>::max()) {
      return failure("args");
    }
    command.crc32 = static_cast<std::uint32_t>(crc);
  } else if (op == "end_book") {
    command.operation = Operation::EndBook;
    if (!read_id(document["id"], command.id)) {
      return failure("args");
    }
  } else if (op == "delete_book") {
    command.operation = Operation::DeleteBook;
    if (!read_id(document["id"], command.id)) {
      return failure("args");
    }
  } else if (op == "open") {
    command.operation = Operation::Open;
    if (!read_id(document["id"], command.id) ||
        !read_size(document["page"], command.page, true)) {
      return failure("args");
    }
  } else if (op == "wifi") {
    command.operation = Operation::Wifi;
    if (!document["on"].is<bool>()) {
      return failure("args");
    }
    command.enabled = document["on"].as<bool>();
  } else if (op == "window") {
    command.operation = Operation::Window;
    if (!read_id(document["book"], command.id) ||
        !read_size(document["from"], command.from, true) ||
        !read_size(document["to"], command.to, true) ||
        command.from > command.to) {
      return failure("args");
    }
  } else {
    return failure("op");
  }

  return ParseResult{true, std::move(command), {}};
}

}  // namespace pocket
