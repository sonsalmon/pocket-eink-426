#include "core/protocol_service.h"

#include <ArduinoJson.h>

#include "core/crc32.h"
#include "core/protocol.h"

namespace pocket {
namespace {

ServiceResult json_result(const bool ok, const char* error = nullptr,
                          const std::size_t page = 0,
                          const bool include_page = false) {
  JsonDocument document;
  document["ok"] = ok;
  if (error != nullptr) {
    document["err"] = error;
  }
  if (include_page) {
    document["n"] = page;
  }
  std::string json;
  serializeJson(document, json);
  return ServiceResult{true, std::move(json)};
}

ServiceResult info_result(const ProtocolBackend& backend) {
  JsonDocument document;
  document["fw"] = "1.0";
  document["w"] = 800;
  document["h"] = 480;
  document["bpp"] = 1;
  document["free"] = backend.free_bytes();
  JsonArray books = document["books"].to<JsonArray>();
  for (const BookInfo& book : backend.list_books()) {
    JsonObject item = books.add<JsonObject>();
    item["id"] = book.id;
    item["title"] = book.title;
    item["pages"] = book.pages;
    item["last"] = book.last;
    JsonArray have = item["have"].to<JsonArray>();
    for (const PageRange& range : book.have) {
      JsonArray pair = have.add<JsonArray>();
      pair.add(range.from);
      pair.add(range.to);
    }
  }
  std::string json;
  serializeJson(document, json);
  return ServiceResult{true, std::move(json)};
}

ServiceResult wifi_result(const WifiInfo& wifi) {
  JsonDocument document;
  document["ok"] = true;
  document["ssid"] = wifi.ssid;
  document["pass"] = wifi.password;
  document["ip"] = wifi.ip;
  document["port"] = wifi.port;
  std::string json;
  serializeJson(document, json);
  return ServiceResult{true, std::move(json)};
}

}  // namespace

ProtocolService::ProtocolService(ProtocolBackend& backend, WifiControl& wifi)
    : backend_(backend), wifi_(wifi) {}

ServiceResult ProtocolService::handle_control(const std::string_view json) {
  if (receiving_) {
    abort_transfer();
    return json_result(false, "len");
  }

  const ParseResult parsed = parse_control_json(json);
  if (!parsed.ok) {
    return json_result(false, parsed.error.c_str());
  }
  const ControlCommand& command = parsed.command;

  switch (command.operation) {
    case Operation::Info:
      return info_result(backend_);
    case Operation::BeginBook:
      return json_result(backend_.create_book(
          BookInfo{command.id, command.title, command.pages}));
    case Operation::BeginPage:
      switch (backend_.prepare_page(command.id, command.page,
                                    command.length)) {
        case PagePrepareResult::Ready:
          break;
        case PagePrepareResult::Space:
          return json_result(false, "space");
        case PagePrepareResult::Storage:
          return json_result(false, "storage");
      }
      receiving_ = true;
      page_ = command.page;
      expected_length_ = command.length;
      received_length_ = 0;
      expected_crc_ = command.crc32;
      crc_state_ = 0xFFFFFFFFU;
      return ServiceResult{};
    case Operation::EndBook:
      return json_result(backend_.finish_book(command.id));
    case Operation::DeleteBook:
      return json_result(backend_.delete_book(command.id));
    case Operation::Open:
      return json_result(backend_.open_book(command.id, command.page));
    case Operation::Wifi:
      if (command.enabled) {
        return wifi_result(wifi_.enable());
      }
      wifi_.disable();
      return json_result(true);
    case Operation::Window:
      return json_result(
          backend_.configure_window(command.id, command.from, command.to));
  }
  return json_result(false, "op");
}

ServiceResult ProtocolService::handle_data(const std::uint8_t* data,
                                           const std::size_t length) {
  if (!receiving_) {
    return json_result(false, "state");
  }
  if (length > expected_length_ - received_length_) {
    abort_transfer();
    return json_result(false, "len");
  }
  if (!backend_.append_page(data, length)) {
    abort_transfer();
    return json_result(false, "storage");
  }

  crc_state_ = crc32_update(crc_state_, data, length);
  received_length_ += length;
  if (received_length_ < expected_length_) {
    return ServiceResult{};
  }

  receiving_ = false;
  if (~crc_state_ != expected_crc_) {
    backend_.abort_page();
    return json_result(false, "crc");
  }
  if (!backend_.commit_page()) {
    backend_.abort_page();
    return json_result(false, "storage");
  }
  return json_result(true, nullptr, page_, true);
}

void ProtocolService::abort_transfer() {
  if (receiving_) {
    backend_.abort_page();
  }
  receiving_ = false;
  expected_length_ = 0;
  received_length_ = 0;
}

}  // namespace pocket
