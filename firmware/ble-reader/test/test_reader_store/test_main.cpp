#include <unity.h>

#include <Arduino.h>
#include <LittleFS.h>
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include <miniz.h>

#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

#include "book_store.h"
#include "core/crc32.h"
#include "core/http_page.h"
#include "core/page_path.h"
#include "reader_app.h"

using pocket::BookStore;

void setUp() {
  LittleFS.state = {};
  host_now = 0;
  host_ladder_mv = 3300;
  pocket::DisplayController::drawn_pages.clear();
  pocket::BleTransport::notifications.clear();
}
void tearDown() {}

void seed_page(const char* id, std::size_t page, std::size_t bytes = 100) {
  auto file = LittleFS.open(pocket::page_path(id, page).c_str(), FILE_WRITE);
  const std::vector<std::uint8_t> payload(bytes, 0);
  TEST_ASSERT_EQUAL(bytes, file.write(payload.data(), payload.size()));
}

void press(pocket::ReaderApp& app, int mv, unsigned long held = 10) {
  host_ladder_mv = mv;
  app.loop();
  host_now += 10;
  app.loop();
  host_now += held;
  app.loop();
  host_ladder_mv = 3300;
  app.loop();
  host_now += 10;
  app.loop();
}

void test_restore_turns_next_without_visiting_menu() {
  BookStore seed([](const std::string&, std::size_t) {});
  TEST_ASSERT_TRUE(seed.begin());
  TEST_ASSERT_TRUE(seed.create_book({"sample", "Sample", 10}));
  seed_page("sample", 3);
  seed_page("sample", 4);
  TEST_ASSERT_TRUE(seed.open_book("sample", 3));
  pocket::ReaderApp app;
  app.begin();
  press(app, 1335);
  TEST_ASSERT_EQUAL_UINT32(4, pocket::DisplayController::drawn_pages.back());
}

void test_transfer_open_and_metadata_changes_keep_navigation_live() {
  pocket::ReaderApp app;
  app.begin();
  auto& service = *pocket::BleTransport::service;
  service.handle_control(
      R"({"op":"begin_book","id":"sample","title":"Sample","pages":2})");
  seed_page("sample", 0);
  seed_page("sample", 1);
  seed_page("sample", 2);
  service.handle_control(R"({"op":"end_book","id":"sample"})");
  service.handle_control(R"({"op":"open","id":"sample","page":0})");
  press(app, 1335);
  TEST_ASSERT_EQUAL_UINT32(1, pocket::DisplayController::drawn_pages.back());
  service.handle_control(
      R"({"op":"begin_book","id":"sample","title":"Updated","pages":4})");
  service.handle_control(R"({"op":"end_book","id":"sample"})");
  service.handle_control(
      R"({"op":"window","book":"sample","from":0,"to":3})");
  press(app, 1335);
  TEST_ASSERT_EQUAL_UINT32(2, pocket::DisplayController::drawn_pages.back());
  press(app, 1980);
  TEST_ASSERT_EQUAL_UINT32(1, pocket::DisplayController::drawn_pages.back());
}

void test_menu_selection_uses_last_read_page() {
  BookStore seed([](const std::string&, std::size_t) {});
  seed.begin();
  seed.create_book({"sample", "Sample", 10});
  seed_page("sample", 0);
  seed_page("sample", 3);
  seed.open_book("sample", 3);
  pocket::ReaderApp app;
  app.begin();
  press(app, 0, 660);
  press(app, 0);
  TEST_ASSERT_EQUAL_UINT32(3, pocket::DisplayController::drawn_pages.back());
}

void test_upload_evicts_other_book_before_actual_reading_book() {
  BookStore store([](const std::string&, std::size_t) {});
  store.begin();
  store.create_book({"reading", "Reading", 400});
  store.create_book({"other", "Other", 400});
  store.create_book({"incoming", "Incoming", 400});
  seed_page("reading", 50);
  seed_page("reading", 260);
  seed_page("other", 0);
  seed_page("other", 49);
  seed_page("other", 50);
  seed_page("other", 51);
  store.open_book("other", 50);
  store.open_book("reading", 50);
  LittleFS.state.capacity = LittleFS.usedBytes();
  TEST_ASSERT_EQUAL_INT(static_cast<int>(pocket::PagePrepareResult::Ready),
      static_cast<int>(store.prepare_page("incoming", 0, 100)));
  TEST_ASSERT_FALSE(store.has_page("other", 0));
  TEST_ASSERT_TRUE(store.has_page("reading", 260));
  TEST_ASSERT_TRUE(store.has_page("reading", 50));
  TEST_ASSERT_TRUE(store.has_page("other", 49));
  TEST_ASSERT_TRUE(store.has_page("other", 50));
  TEST_ASSERT_TRUE(store.has_page("other", 51));
  TEST_ASSERT_TRUE(store.need_request("other", 0).needed);
}

void test_reading_book_shrinks_only_outside_20_back_150_ahead() {
  BookStore store([](const std::string&, std::size_t) {});
  store.begin();
  store.create_book({"reading", "Reading", 400});
  store.create_book({"incoming", "Incoming", 400});
  for (const auto page : {0, 29, 30, 49, 50, 51, 200, 201, 260}) {
    seed_page("reading", page);
  }
  store.open_book("reading", 50);
  LittleFS.state.capacity = LittleFS.usedBytes();
  TEST_ASSERT_EQUAL_INT(static_cast<int>(pocket::PagePrepareResult::Ready),
      static_cast<int>(store.prepare_page("incoming", 0, 100)));
  for (const auto page : {0, 29, 201, 260}) {
    TEST_ASSERT_FALSE(store.has_page("reading", page));
  }
  for (const auto page : {30, 49, 50, 51, 200}) {
    TEST_ASSERT_TRUE(store.has_page("reading", page));
  }
  const auto need = store.need_request("reading", 50);
  TEST_ASSERT_TRUE(need.needed);
  TEST_ASSERT_EQUAL_UINT32(52, need.from);
  TEST_ASSERT_EQUAL_UINT32(200, need.to);
}

void test_upload_cannot_evict_reading_page_or_other_last_neighborhood() {
  BookStore store([](const std::string&, std::size_t) {});
  store.begin();
  store.create_book({"reading", "Reading", 400});
  store.create_book({"other", "Other", 400});
  store.create_book({"incoming", "Incoming", 400});
  seed_page("reading", 50);
  for (const auto page : {48, 49, 50, 51, 52}) seed_page("other", page);
  store.open_book("other", 50);
  store.open_book("reading", 50);
  LittleFS.state.capacity = LittleFS.usedBytes();
  TEST_ASSERT_EQUAL_INT(static_cast<int>(pocket::PagePrepareResult::Space),
      static_cast<int>(store.prepare_page("incoming", 0, 100)));
  TEST_ASSERT_TRUE(store.has_page("reading", 50));
  for (const auto page : {48, 49, 50, 51, 52}) {
    TEST_ASSERT_TRUE(store.has_page("other", page));
  }
}

void test_eviction_emits_need_for_reading_book_without_page_turn() {
  BookStore seed([](const std::string&, std::size_t) {});
  seed.begin();
  seed.create_book({"reading", "Reading", 400});
  seed_page("reading", 50);
  seed_page("reading", 260);
  seed.open_book("reading", 50);
  pocket::ReaderApp app;
  app.begin();
  auto& service = *pocket::BleTransport::service;
  service.handle_control(
      R"({"op":"begin_book","id":"incoming","title":"Incoming","pages":2})");
  LittleFS.state.capacity = LittleFS.usedBytes();
  service.handle_control(
      R"({"op":"begin_page","book":"incoming","n":0,"len":100,"crc32":0})");
  TEST_ASSERT_FALSE(pocket::BleTransport::notifications.empty());
  TEST_ASSERT_NOT_EQUAL(std::string::npos,
      pocket::BleTransport::notifications.back().find(R"("book":"reading")"));
}

void test_http_command_streams_real_compressed_page_to_store() {
  BookStore store([](const std::string&, std::size_t) {});
  store.begin();
  pocket::WifiTransport wifi;
  pocket::ProtocolService service(store, wifi);
  service.handle_control(
      R"({"op":"begin_book","id":"sample","title":"Sample","pages":2})");
  const std::vector<std::uint8_t> raw(48000, 0);
  std::size_t length = 0;
  auto* compressed = static_cast<std::uint8_t*>(tdefl_compress_mem_to_heap(
      raw.data(), raw.size(), &length, TDEFL_DEFAULT_MAX_PROBES));
  TEST_ASSERT_NOT_NULL(compressed);
  std::string command;
  TEST_ASSERT_TRUE(pocket::build_http_page_command("sample", "0",
      std::to_string(pocket::crc32(compressed, length)), length, command));
  TEST_ASSERT_FALSE(service.handle_control(command).notify);
  const auto result = service.handle_data(compressed, length);
  std::free(compressed);
  TEST_ASSERT_NOT_EQUAL(std::string::npos, result.json.find(R"("ok":true)"));
  TEST_ASSERT_TRUE(store.has_page("sample", 0));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_restore_turns_next_without_visiting_menu);
  RUN_TEST(test_transfer_open_and_metadata_changes_keep_navigation_live);
  RUN_TEST(test_menu_selection_uses_last_read_page);
  RUN_TEST(test_upload_evicts_other_book_before_actual_reading_book);
  RUN_TEST(test_reading_book_shrinks_only_outside_20_back_150_ahead);
  RUN_TEST(test_upload_cannot_evict_reading_page_or_other_last_neighborhood);
  RUN_TEST(test_eviction_emits_need_for_reading_book_without_page_turn);
  RUN_TEST(test_http_command_streams_real_compressed_page_to_store);
  return UNITY_END();
}
