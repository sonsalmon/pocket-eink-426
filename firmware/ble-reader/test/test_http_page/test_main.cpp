#include <unity.h>

#include <string>

#include "core/http_page.h"
#include "core/protocol.h"

void setUp() {}
void tearDown() {}

void test_http_page_produces_valid_protocol_command() {
  std::string command;
  TEST_ASSERT_TRUE(pocket::build_http_page_command(
      "sample", "0", "4294967295", 3, command));
  const auto parsed = pocket::parse_control_json(command);
  TEST_ASSERT_TRUE(parsed.ok);
  TEST_ASSERT_EQUAL_STRING("sample", parsed.command.id.c_str());
  TEST_ASSERT_EQUAL_UINT32(0, parsed.command.page);
  TEST_ASSERT_EQUAL_UINT32(3, parsed.command.length);
  TEST_ASSERT_EQUAL_HEX32(0xFFFFFFFFU, parsed.command.crc32);
}

void test_http_page_rejects_non_numeric_and_overflow_arguments() {
  for (const char* value : {"", "-1", "+1", "1.0", " 1", "1x",
                            "0,\"n\":2", "18446744073709551616"}) {
    std::string command;
    TEST_ASSERT_FALSE(
        pocket::build_http_page_command("sample", value, "0", 3, command));
    TEST_ASSERT_FALSE(
        pocket::build_http_page_command("sample", "0", value, 3, command));
  }
  std::string command;
  TEST_ASSERT_FALSE(pocket::build_http_page_command(
      "sample", "0", "4294967296", 3, command));
}

void test_http_page_rejects_unsafe_books_and_empty_body() {
  for (const char* book : {"../sample", "x\"y", "x\\y", "", "x\n"}) {
    std::string command;
    TEST_ASSERT_FALSE(
        pocket::build_http_page_command(book, "0", "0", 3, command));
  }
  std::string command;
  TEST_ASSERT_FALSE(
      pocket::build_http_page_command("sample", "0", "0", 0, command));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_http_page_produces_valid_protocol_command);
  RUN_TEST(test_http_page_rejects_non_numeric_and_overflow_arguments);
  RUN_TEST(test_http_page_rejects_unsafe_books_and_empty_body);
  return UNITY_END();
}
