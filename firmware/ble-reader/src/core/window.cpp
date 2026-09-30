#include "core/window.h"

#include <algorithm>
#include <cstdlib>

namespace pocket {
namespace {

constexpr std::size_t kBackPages = 20;
constexpr std::size_t kAheadPages = 150;
constexpr std::size_t kNeedThreshold = 30;

std::size_t distance(const std::size_t left, const std::size_t right) {
  return left > right ? left - right : right - left;
}

bool inside_current_window(const std::size_t page,
                           const std::size_t current) {
  const std::size_t first = current > kBackPages ? current - kBackPages : 0;
  return page >= first && page <= current + kAheadPages;
}

}  // namespace

std::vector<PageRange> page_ranges(std::vector<std::size_t> pages) {
  if (pages.empty()) {
    return {};
  }
  std::sort(pages.begin(), pages.end());
  pages.erase(std::unique(pages.begin(), pages.end()), pages.end());
  std::vector<PageRange> ranges;
  PageRange range{pages.front(), pages.front()};
  for (std::size_t index = 1; index < pages.size(); ++index) {
    if (pages[index] == range.to + 1) {
      range.to = pages[index];
    } else {
      ranges.push_back(range);
      range = PageRange{pages[index], pages[index]};
    }
  }
  ranges.push_back(range);
  return ranges;
}

EvictionPlan plan_eviction(const std::vector<CachedPage>& cached,
                           const std::size_t free_bytes,
                           const std::size_t required_bytes,
                           const std::string_view current_book,
                           const std::size_t current_page) {
  if (required_bytes <= free_bytes) {
    return EvictionPlan{{}, true, false};
  }
  const std::size_t target = required_bytes - free_bytes;
  std::vector<CachedPage> other;
  std::vector<CachedPage> current;
  for (const CachedPage& page : cached) {
    if (page.book != current_book) {
      other.push_back(page);
    } else if (!inside_current_window(page.page, current_page)) {
      current.push_back(page);
    }
  }
  const auto farthest_first = [](const CachedPage& left,
                                 const CachedPage& right) {
    return distance(left.page, left.last) > distance(right.page, right.last);
  };
  std::sort(other.begin(), other.end(), farthest_first);
  std::sort(current.begin(), current.end(), farthest_first);

  EvictionPlan plan;
  std::size_t freed = 0;
  for (const CachedPage& page : other) {
    plan.pages.push_back(page);
    freed += page.bytes;
    if (freed >= target) {
      plan.enough = true;
      return plan;
    }
  }
  for (const CachedPage& page : current) {
    plan.pages.push_back(page);
    plan.current_book_shrunk = true;
    freed += page.bytes;
    if (freed >= target) {
      plan.enough = true;
      return plan;
    }
  }
  return plan;
}

NeedRange needed_pages(const std::vector<std::size_t>& pages,
                       const std::size_t current_page,
                       const std::size_t total_pages,
                       const bool window_mode) {
  if (!window_mode || current_page >= total_pages) {
    return {};
  }
  std::vector<std::size_t> sorted = pages;
  std::sort(sorted.begin(), sorted.end());
  if (!std::binary_search(sorted.begin(), sorted.end(), current_page)) {
    return NeedRange{true, current_page,
                     std::min(total_pages - 1, current_page + kAheadPages)};
  }
  std::size_t contiguous_to = current_page;
  for (const std::size_t page : sorted) {
    if (page == contiguous_to + 1) {
      contiguous_to = page;
    } else if (page > contiguous_to + 1) {
      break;
    }
  }
  if (contiguous_to - current_page >= kNeedThreshold ||
      contiguous_to + 1 >= total_pages) {
    return {};
  }
  return NeedRange{true, contiguous_to + 1,
                   std::min(total_pages - 1, current_page + kAheadPages)};
}

}  // namespace pocket
