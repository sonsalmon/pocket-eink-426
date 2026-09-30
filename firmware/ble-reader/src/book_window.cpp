#include "book_store.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

#include <algorithm>
#include <cstdlib>

#include "core/page_path.h"

namespace pocket {
namespace {

bool parse_page_name(const char* name, std::size_t& page) {
  const std::string path(name);
  const std::size_t slash = path.find_last_of('/');
  const std::string file =
      slash == std::string::npos ? path : path.substr(slash + 1);
  if (file.size() < 6 || file.front() != 'p' ||
      file.substr(file.size() - 4) != ".bin") {
    return false;
  }
  char* end = nullptr;
  const unsigned long parsed = std::strtoul(file.c_str() + 1, &end, 10);
  if (end == nullptr || std::string_view(end) != ".bin") {
    return false;
  }
  page = parsed;
  return true;
}

}  // namespace

std::vector<std::size_t> BookStore::list_pages(
    const std::string_view id) const {
  std::vector<std::size_t> pages;
  fs::File root = LittleFS.open(book_directory(id).c_str());
  if (!root || !root.isDirectory()) {
    return pages;
  }
  fs::File entry = root.openNextFile();
  while (entry) {
    std::size_t page = 0;
    if (!entry.isDirectory() && parse_page_name(entry.path(), page)) {
      pages.push_back(page);
    }
    entry = root.openNextFile();
  }
  return pages;
}

std::vector<CachedPage> BookStore::cached_pages() const {
  std::vector<CachedPage> cached;
  for (const BookInfo& book : list_books()) {
    for (const std::size_t page : list_pages(book.id)) {
      fs::File file = LittleFS.open(page_path(book.id, page).c_str(), FILE_READ);
      if (file) {
        cached.push_back(
            CachedPage{book.id, page, file.size(), book.last});
      }
    }
  }
  return cached;
}

PagePrepareResult BookStore::prepare_page(const std::string_view id,
                                          const std::size_t page,
                                          const std::size_t length) {
  if (free_bytes() < length) {
    std::string reading = current_book_;
    std::size_t last = current_page_;
    if (reading.empty()) {
      reading = id;
      last = page;
      for (const BookInfo& book : list_books()) {
        if (book.id == id) {
          last = book.last;
          break;
        }
      }
    }
    const EvictionPlan plan =
        plan_eviction(cached_pages(), free_bytes(), length, reading, last);
    if (!plan.enough) {
      return PagePrepareResult::Space;
    }
    std::vector<std::string> affected;
    for (const CachedPage& victim : plan.pages) {
      if (std::find(affected.begin(), affected.end(), victim.book) ==
          affected.end()) {
        // Persist refill mode before removing any page, including on errors.
        if (!set_window_mode(victim.book, true)) {
          return PagePrepareResult::Storage;
        }
        affected.push_back(victim.book);
      }
    }
    for (const CachedPage& victim : plan.pages) {
      if (!LittleFS.remove(page_path(victim.book, victim.page).c_str())) {
        return PagePrepareResult::Storage;
      }
    }
    if (change_handler_) {
      change_handler_();
    }
  }
  return open_page_file(id, page) ? PagePrepareResult::Ready
                                  : PagePrepareResult::Storage;
}

bool BookStore::configure_window(const std::string_view id,
                                 const std::size_t from,
                                 const std::size_t to) {
  for (const std::size_t page : list_pages(id)) {
    if ((id != current_book_ || page != current_page_) &&
        (page < from || page > to) &&
        !LittleFS.remove(page_path(id, page).c_str())) {
      return false;
    }
  }
  if (!set_window_mode(id, true)) {
    return false;
  }
  if (change_handler_) {
    change_handler_();
  }
  return true;
}

NeedRange BookStore::need_request(const std::string_view id,
                                  const std::size_t page) const {
  for (const BookInfo& book : list_books()) {
    if (book.id == id) {
      return needed_pages(list_pages(id), page, book.pages,
                          read_window_mode(id));
    }
  }
  return {};
}

bool BookStore::set_window_mode(const std::string_view id,
                                const bool enabled) const {
  const std::string path = book_meta_path(id);
  fs::File file = LittleFS.open(path.c_str(), FILE_READ);
  JsonDocument document;
  if (!file || deserializeJson(document, file)) {
    return false;
  }
  file.close();
  document["window"] = enabled;
  file = LittleFS.open(path.c_str(), FILE_WRITE);
  return file && serializeJson(document, file) > 0;
}

bool BookStore::read_window_mode(const std::string_view id) const {
  fs::File file = LittleFS.open(book_meta_path(id).c_str(), FILE_READ);
  JsonDocument document;
  return file && !deserializeJson(document, file) &&
         (document["window"] | false);
}

}  // namespace pocket
