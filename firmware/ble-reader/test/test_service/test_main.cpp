#include <unity.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "core/crc32.h"
#include "core/protocol_service.h"

class FakeBackend final : public pocket::ProtocolBackend {
 public:
  std::size_t free = 4096;
  std::vector<pocket::BookInfo> books;
  std::vector<std::uint8_t> page_data;
  bool page_started = false;
  bool page_committed = false;
  bool page_aborted = false;
  std::string opened_id;
  std::size_t opened_page = 0;
  bool window_configured = false;

  std::size_t free_bytes() const override { return free; }
  std::vector<pocket::BookInfo> list_books() const override { return books; }
  bool create_book(const pocket::BookInfo& book) override {
    books.push_back(book);
    return true;
  }
  bool finish_book(std::string_view id) override { return !id.empty(); }
  bool delete_book(std::string_view id) override {
    return !id.empty();
  }
  bool open_book(std::string_view id, std::size_t page) override {
    opened_id = id;
    opened_page = page;
    return true;
  }
  bool configure_window(std::string_view id, std::size_t from,
                        std::size_t to) override {
    window_configured = !id.empty() && from <= to;
    return window_configured;
  }
  pocket::PagePrepareResult prepare_page(std::string_view id,
                                         std::size_t page,
                                         std::size_t length) override {
    if (length > free) {
      return pocket::PagePrepareResult::Space;
    }
    page_started = !id.empty() || page == 0;
    page_data.clear();
    return pocket::PagePrepareResult::Ready;
  }
  bool append_page(const std::uint8_t* data,
                   const std::size_t length) override {
    page_data.insert(page_data.end(), data, data + length);
    return true;
  }
  bool commit_page() override {
    page_committed = true;
    return true;
  }
  void abort_page() override { page_aborted = true; }
};

class FakeWifi final : public pocket::WifiControl {
 public:
  pocket::WifiInfo enable() override {
    return {"pocket-1234", "password", "192.168.4.1", 80};
  }
  void disable() override {}
};

void setUp() {}
void tearDown() {}

void test_info_returns_device_shape_and_books() {
  FakeBackend backend;
  FakeWifi wifi;
  backend.free = 1234;
  backend.books.push_back({"book-1", "Title", 8, {{0, 2}}, 1});
  pocket::ProtocolService service(backend, wifi);

  const pocket::ServiceResult result =
      service.handle_control(R"({"op":"info"})");

  TEST_ASSERT_TRUE(result.notify);
  TEST_ASSERT_NOT_EQUAL(std::string::npos, result.json.find(R"("fw":"1.0")"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, result.json.find(R"("free":1234)"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, result.json.find(R"("book-1")"));
}

void test_page_stream_commits_after_exact_length_and_crc() {
  FakeBackend backend;
  FakeWifi wifi;
  pocket::ProtocolService service(backend, wifi);
  const std::vector<std::uint8_t> payload{1, 2, 3, 4, 5};
  const std::uint32_t checksum =
      pocket::crc32(payload.data(), payload.size());
  const std::string command =
      R"({"op":"begin_page","book":"book","n":2,"len":5,"crc32":)" +
      std::to_string(checksum) + "}";

  const pocket::ServiceResult begin = service.handle_control(command);
  const pocket::ServiceResult first = service.handle_data(payload.data(), 2);
  const pocket::ServiceResult final =
      service.handle_data(payload.data() + 2, 3);

  TEST_ASSERT_FALSE(begin.notify);
  TEST_ASSERT_FALSE(first.notify);
  TEST_ASSERT_TRUE(final.notify);
  TEST_ASSERT_EQUAL_STRING(R"({"ok":true,"n":2})", final.json.c_str());
  TEST_ASSERT_TRUE(backend.page_committed);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(payload.data(), backend.page_data.data(),
                                payload.size());
}

void test_page_stream_rejects_crc_mismatch() {
  FakeBackend backend;
  FakeWifi wifi;
  pocket::ProtocolService service(backend, wifi);
  constexpr std::uint8_t payload[]{7, 8};
  service.handle_control(
      R"({"op":"begin_page","book":"book","n":0,"len":2,"crc32":0})");

  const pocket::ServiceResult result =
      service.handle_data(payload, sizeof(payload));

  TEST_ASSERT_EQUAL_STRING(R"({"ok":false,"err":"crc"})",
                           result.json.c_str());
  TEST_ASSERT_TRUE(backend.page_aborted);
  TEST_ASSERT_FALSE(backend.page_committed);
}

void test_page_stream_rejects_length_overrun() {
  FakeBackend backend;
  FakeWifi wifi;
  pocket::ProtocolService service(backend, wifi);
  constexpr std::uint8_t payload[]{1, 2, 3};
  service.handle_control(
      R"({"op":"begin_page","book":"book","n":0,"len":2,"crc32":0})");

  const pocket::ServiceResult result =
      service.handle_data(payload, sizeof(payload));

  TEST_ASSERT_EQUAL_STRING(R"({"ok":false,"err":"len"})",
                           result.json.c_str());
  TEST_ASSERT_TRUE(backend.page_aborted);
}

void test_begin_page_rejects_insufficient_space() {
  FakeBackend backend;
  FakeWifi wifi;
  backend.free = 4;
  pocket::ProtocolService service(backend, wifi);

  const pocket::ServiceResult result = service.handle_control(
      R"({"op":"begin_page","book":"book","n":0,"len":5,"crc32":0})");

  TEST_ASSERT_EQUAL_STRING(R"({"ok":false,"err":"space"})",
                           result.json.c_str());
  TEST_ASSERT_FALSE(backend.page_started);
}

void test_control_during_stream_reports_short_length() {
  FakeBackend backend;
  FakeWifi wifi;
  pocket::ProtocolService service(backend, wifi);
  service.handle_control(
      R"({"op":"begin_page","book":"book","n":0,"len":5,"crc32":0})");

  const pocket::ServiceResult result =
      service.handle_control(R"({"op":"info"})");

  TEST_ASSERT_EQUAL_STRING(R"({"ok":false,"err":"len"})",
                           result.json.c_str());
  TEST_ASSERT_TRUE(backend.page_aborted);
}

void test_wifi_op_uses_transport_control_without_ble() {
  FakeBackend backend;
  FakeWifi wifi;
  pocket::ProtocolService service(backend, wifi);

  const pocket::ServiceResult result =
      service.handle_control(R"({"op":"wifi","on":true})");

  TEST_ASSERT_TRUE(result.notify);
  TEST_ASSERT_NOT_EQUAL(std::string::npos,
                        result.json.find(R"("ssid":"pocket-1234")"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos,
                        result.json.find(R"("ip":"192.168.4.1")"));
}

void test_window_op_uses_storage_backend_without_transport() {
  FakeBackend backend;
  FakeWifi wifi;
  pocket::ProtocolService service(backend, wifi);

  const pocket::ServiceResult result = service.handle_control(
      R"({"op":"window","book":"book","from":20,"to":170})");

  TEST_ASSERT_EQUAL_STRING(R"({"ok":true})", result.json.c_str());
  TEST_ASSERT_TRUE(backend.window_configured);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_info_returns_device_shape_and_books);
  RUN_TEST(test_page_stream_commits_after_exact_length_and_crc);
  RUN_TEST(test_page_stream_rejects_crc_mismatch);
  RUN_TEST(test_page_stream_rejects_length_overrun);
  RUN_TEST(test_begin_page_rejects_insufficient_space);
  RUN_TEST(test_control_during_stream_reports_short_length);
  RUN_TEST(test_wifi_op_uses_transport_control_without_ble);
  RUN_TEST(test_window_op_uses_storage_backend_without_transport);
  return UNITY_END();
}
