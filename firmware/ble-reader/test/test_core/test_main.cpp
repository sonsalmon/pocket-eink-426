#include <unity.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <string>

#include "core/crc32.h"
#include "core/inflate.h"
#include "core/ladder.h"
#include "core/page_path.h"
#include "core/protocol.h"

using pocket::Key;
using pocket::Operation;

void setUp() {}
void tearDown() {}

void test_parses_all_control_operations() {
  const std::array<std::pair<const char*, Operation>, 8> cases{{
      {R"({"op":"info"})", Operation::Info},
      {R"({"op":"begin_book","id":"novel-1","title":"책","pages":12})",
       Operation::BeginBook},
      {R"({"op":"begin_page","book":"novel-1","n":0,"len":42,"crc32":7})",
       Operation::BeginPage},
      {R"({"op":"end_book","id":"novel-1"})", Operation::EndBook},
      {R"({"op":"delete_book","id":"novel-1"})", Operation::DeleteBook},
      {R"({"op":"open","id":"novel-1","page":3})", Operation::Open},
      {R"({"op":"wifi","on":true})", Operation::Wifi},
      {R"({"op":"window","book":"novel-1","from":2,"to":9})",
       Operation::Window},
  }};

  for (const auto& item : cases) {
    const pocket::ParseResult parsed = pocket::parse_control_json(item.first);
    TEST_ASSERT_TRUE(parsed.ok);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(item.second),
                          static_cast<int>(parsed.command.operation));
  }
}

void test_rejects_bad_json_and_unsafe_book_id() {
  TEST_ASSERT_FALSE(pocket::parse_control_json("{").ok);
  TEST_ASSERT_FALSE(
      pocket::parse_control_json(
          R"({"op":"begin_book","id":"../bad","title":"x","pages":1})")
          .ok);
}

void test_parses_page_fields_without_losing_crc_bits() {
  const pocket::ParseResult parsed = pocket::parse_control_json(
      R"({"op":"begin_page","book":"book_1","n":9,"len":1234,"crc32":4294967295})");
  TEST_ASSERT_TRUE(parsed.ok);
  TEST_ASSERT_EQUAL_UINT32(9, parsed.command.page);
  TEST_ASSERT_EQUAL_UINT32(1234, parsed.command.length);
  TEST_ASSERT_EQUAL_HEX32(0xFFFFFFFFU, parsed.command.crc32);
}

void test_crc32_matches_standard_vector() {
  constexpr char input[] = "123456789";
  TEST_ASSERT_EQUAL_HEX32(
      0xCBF43926U,
      pocket::crc32(reinterpret_cast<const std::uint8_t*>(input), 9));
}

void test_inflates_python_zlib_raw_stream() {
  // zlib.compressobj(level=9, wbits=-15) over "raw deflate round trip".
  constexpr std::array<std::uint8_t, 24> compressed{
      0x2b, 0x4a, 0x2c, 0x57, 0x48, 0x49, 0x4d, 0xcb,
      0x49, 0x2c, 0x49, 0x55, 0x28, 0xca, 0x2f, 0xcd,
      0x4b, 0x51, 0x28, 0x29, 0xca, 0x2c, 0x00, 0x00};
  constexpr char expected[] = "raw deflate round trip";
  std::array<std::uint8_t, sizeof(expected) - 1> output{};

  TEST_ASSERT_TRUE(pocket::inflate_raw(compressed.data(), compressed.size(),
                                       output.data(), output.size()));
  TEST_ASSERT_EQUAL_MEMORY(expected, output.data(), output.size());
}

void test_decodes_ladder_windows_and_edges() {
  TEST_ASSERT_EQUAL_INT(static_cast<int>(Key::Left),
                        static_cast<int>(pocket::decode_ladder_mv(1780)));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(Key::Left),
                        static_cast<int>(pocket::decode_ladder_mv(2140)));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(Key::Right),
                        static_cast<int>(pocket::decode_ladder_mv(1320)));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(Key::Center),
                        static_cast<int>(pocket::decode_ladder_mv(250)));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(Key::None),
                        static_cast<int>(pocket::decode_ladder_mv(251)));
}

void test_formats_book_and_page_paths() {
  TEST_ASSERT_EQUAL_STRING("/book/my_book",
                           pocket::book_directory("my_book").c_str());
  TEST_ASSERT_EQUAL_STRING("/book/my_book/meta.json",
                           pocket::book_meta_path("my_book").c_str());
  TEST_ASSERT_EQUAL_STRING("/book/my_book/p42.bin",
                           pocket::page_path("my_book", 42).c_str());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_parses_all_control_operations);
  RUN_TEST(test_rejects_bad_json_and_unsafe_book_id);
  RUN_TEST(test_parses_page_fields_without_losing_crc_bits);
  RUN_TEST(test_crc32_matches_standard_vector);
  RUN_TEST(test_inflates_python_zlib_raw_stream);
  RUN_TEST(test_decodes_ladder_windows_and_edges);
  RUN_TEST(test_formats_book_and_page_paths);
  return UNITY_END();
}
