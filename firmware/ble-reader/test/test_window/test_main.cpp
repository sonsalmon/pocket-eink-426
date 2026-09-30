#include <unity.h>

#include <string>
#include <vector>

#include "core/window.h"

void setUp() {}
void tearDown() {}

void test_whole_book_is_kept_when_incoming_page_fits() {
  const std::vector<pocket::CachedPage> cached{
      {"current", 0, 100, 10},
      {"current", 1, 100, 10},
      {"other", 50, 100, 50},
  };

  const pocket::EvictionPlan plan =
      pocket::plan_eviction(cached, 100, 100, "current", 10);

  TEST_ASSERT_TRUE(plan.enough);
  TEST_ASSERT_TRUE(plan.pages.empty());
  TEST_ASSERT_FALSE(plan.current_book_shrunk);
}

void test_current_book_shrinks_to_window_when_book_does_not_fit() {
  const std::vector<pocket::CachedPage> cached{
      {"current", 10, 100, 100},
      {"current", 100, 100, 100},
      {"current", 260, 100, 100},
  };

  const pocket::EvictionPlan plan =
      pocket::plan_eviction(cached, 0, 100, "current", 100);

  TEST_ASSERT_TRUE(plan.enough);
  TEST_ASSERT_TRUE(plan.current_book_shrunk);
  TEST_ASSERT_EQUAL_UINT32(260, plan.pages.front().page);
}

void test_other_book_is_evicted_before_current_book() {
  const std::vector<pocket::CachedPage> cached{
      {"current", 260, 100, 100},
      {"other", 0, 100, 50},
  };

  const pocket::EvictionPlan plan =
      pocket::plan_eviction(cached, 0, 100, "current", 100);

  TEST_ASSERT_TRUE(plan.enough);
  TEST_ASSERT_FALSE(plan.current_book_shrunk);
  TEST_ASSERT_EQUAL_STRING("other", plan.pages.front().book.c_str());
}

void test_need_is_emitted_only_in_window_mode_below_threshold() {
  std::vector<std::size_t> pages;
  for (std::size_t page = 40; page <= 55; ++page) {
    pages.push_back(page);
  }

  const pocket::NeedRange full_mode =
      pocket::needed_pages(pages, 40, 300, false);
  const pocket::NeedRange window_mode =
      pocket::needed_pages(pages, 40, 300, true);

  TEST_ASSERT_FALSE(full_mode.needed);
  TEST_ASSERT_TRUE(window_mode.needed);
  TEST_ASSERT_EQUAL_UINT32(56, window_mode.from);
  TEST_ASSERT_EQUAL_UINT32(190, window_mode.to);
}

void test_missing_current_page_requests_it_only_in_window_mode() {
  const pocket::NeedRange full_mode =
      pocket::needed_pages({41, 42}, 40, 300, false);
  const pocket::NeedRange window_mode =
      pocket::needed_pages({41, 42}, 40, 300, true);

  TEST_ASSERT_FALSE(full_mode.needed);
  TEST_ASSERT_TRUE(window_mode.needed);
  TEST_ASSERT_EQUAL_UINT32(40, window_mode.from);
  TEST_ASSERT_EQUAL_UINT32(190, window_mode.to);
}

void test_have_pages_are_compacted_into_ranges() {
  const std::vector<pocket::PageRange> ranges =
      pocket::page_ranges({5, 1, 2, 3, 8, 9, 9});

  TEST_ASSERT_EQUAL_UINT32(3, ranges.size());
  TEST_ASSERT_EQUAL_UINT32(1, ranges[0].from);
  TEST_ASSERT_EQUAL_UINT32(3, ranges[0].to);
  TEST_ASSERT_EQUAL_UINT32(5, ranges[1].from);
  TEST_ASSERT_EQUAL_UINT32(8, ranges[2].from);
  TEST_ASSERT_EQUAL_UINT32(9, ranges[2].to);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_whole_book_is_kept_when_incoming_page_fits);
  RUN_TEST(test_current_book_shrinks_to_window_when_book_does_not_fit);
  RUN_TEST(test_other_book_is_evicted_before_current_book);
  RUN_TEST(test_need_is_emitted_only_in_window_mode_below_threshold);
  RUN_TEST(test_missing_current_page_requests_it_only_in_window_mode);
  RUN_TEST(test_have_pages_are_compacted_into_ranges);
  return UNITY_END();
}
