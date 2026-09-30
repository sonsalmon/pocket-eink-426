#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "core/window.h"

namespace pocket {

struct BookInfo {
  std::string id;
  std::string title;
  std::size_t pages = 0;
  std::vector<PageRange> have;
  std::size_t last = 0;
};

enum class PagePrepareResult {
  Ready,
  Space,
  Storage,
};

struct WifiInfo {
  std::string ssid;
  std::string password;
  std::string ip;
  std::size_t port = 80;
};

class WifiControl {
 public:
  virtual ~WifiControl() = default;
  virtual WifiInfo enable() = 0;
  virtual void disable() = 0;
};

class ProtocolBackend {
 public:
  virtual ~ProtocolBackend() = default;

  virtual std::size_t free_bytes() const = 0;
  virtual std::vector<BookInfo> list_books() const = 0;
  virtual bool create_book(const BookInfo& book) = 0;
  virtual bool finish_book(std::string_view id) = 0;
  virtual bool delete_book(std::string_view id) = 0;
  virtual bool open_book(std::string_view id, std::size_t page) = 0;
  virtual bool configure_window(std::string_view id, std::size_t from,
                                std::size_t to) = 0;
  virtual PagePrepareResult prepare_page(std::string_view id,
                                         std::size_t page,
                                         std::size_t length) = 0;
  virtual bool append_page(const std::uint8_t* data, std::size_t length) = 0;
  virtual bool commit_page() = 0;
  virtual void abort_page() = 0;
};

struct ServiceResult {
  bool notify = false;
  std::string json;
};

class ProtocolService {
 public:
  ProtocolService(ProtocolBackend& backend, WifiControl& wifi);

  ServiceResult handle_control(std::string_view json);
  ServiceResult handle_data(const std::uint8_t* data, std::size_t length);
  void abort_transfer();

 private:
  ProtocolBackend& backend_;
  WifiControl& wifi_;
  bool receiving_ = false;
  std::size_t page_ = 0;
  std::size_t expected_length_ = 0;
  std::size_t received_length_ = 0;
  std::uint32_t expected_crc_ = 0;
  std::uint32_t crc_state_ = 0xFFFFFFFFU;
};

}  // namespace pocket
