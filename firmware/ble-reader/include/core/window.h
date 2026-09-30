#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace pocket {

struct PageRange {
  std::size_t from = 0;
  std::size_t to = 0;
};

struct CachedPage {
  std::string book;
  std::size_t page = 0;
  std::size_t bytes = 0;
  std::size_t last = 0;
};

struct EvictionPlan {
  std::vector<CachedPage> pages;
  bool enough = false;
  bool current_book_shrunk = false;
};

struct NeedRange {
  bool needed = false;
  std::size_t from = 0;
  std::size_t to = 0;
};

std::vector<PageRange> page_ranges(std::vector<std::size_t> pages);
EvictionPlan plan_eviction(const std::vector<CachedPage>& cached,
                           std::size_t free_bytes,
                           std::size_t required_bytes,
                           std::string_view current_book,
                           std::size_t current_page);
NeedRange needed_pages(const std::vector<std::size_t>& pages,
                       std::size_t current_page, std::size_t total_pages,
                       bool window_mode);

}  // namespace pocket
