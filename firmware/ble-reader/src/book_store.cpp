#include "book_store.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

#include <cstdint>
#include <vector>

#include "core/inflate.h"
#include "core/page_path.h"

namespace pocket {
namespace {

constexpr char kStatePath[] = "/state.json";

bool read_book_info(const std::string& path, BookInfo& book) {
  fs::File file = LittleFS.open(path.c_str(), FILE_READ);
  if (!file) {
    return false;
  }
  JsonDocument document;
  if (deserializeJson(document, file)) {
    return false;
  }
  if (!document["id"].is<const char*>() ||
      !document["title"].is<const char*>() ||
      !document["pages"].is<std::size_t>()) {
    return false;
  }
  book = BookInfo{document["id"].as<const char*>(),
                  document["title"].as<const char*>(),
                  document["pages"].as<std::size_t>(),
                  {},
                  document["last"] | 0U};
  return true;
}

}  // namespace

BookStore::BookStore(OpenBookHandler open_handler,
                     std::function<void()> change_handler)
    : open_handler_(std::move(open_handler)),
      change_handler_(std::move(change_handler)) {}

bool BookStore::begin() {
  if (!LittleFS.begin(true)) {
    return false;
  }
  return LittleFS.exists("/book") || LittleFS.mkdir("/book");
}

std::size_t BookStore::free_bytes() const {
  return LittleFS.totalBytes() - LittleFS.usedBytes();
}

std::vector<BookInfo> BookStore::list_books() const {
  std::vector<BookInfo> books;
  fs::File root = LittleFS.open("/book");
  if (!root || !root.isDirectory()) {
    return books;
  }
  fs::File entry = root.openNextFile();
  while (entry) {
    if (entry.isDirectory()) {
      BookInfo book;
      const std::string meta =
          std::string(entry.path()) + std::string("/meta.json");
      if (read_book_info(meta, book)) {
        book.have = page_ranges(list_pages(book.id));
        books.push_back(std::move(book));
      }
    }
    entry = root.openNextFile();
  }
  return books;
}

bool BookStore::create_book(const BookInfo& book) {
  const std::string directory = book_directory(book.id);
  if (!LittleFS.exists(directory.c_str()) &&
      !LittleFS.mkdir(directory.c_str())) {
    return false;
  }
  fs::File file = LittleFS.open(book_meta_path(book.id).c_str(), FILE_WRITE);
  if (!file) {
    return false;
  }
  JsonDocument document;
  document["id"] = book.id;
  document["title"] = book.title;
  document["pages"] = book.pages;
  document["last"] = 0;
  document["window"] = false;
  const bool saved = serializeJson(document, file) > 0;
  file.close();
  if (saved && change_handler_) {
    change_handler_();
  }
  return saved;
}

bool BookStore::finish_book(const std::string_view id) {
  const bool exists = LittleFS.exists(book_meta_path(id).c_str());
  if (exists && change_handler_) {
    change_handler_();
  }
  return exists;
}

bool BookStore::delete_book(const std::string_view id) {
  const std::string directory = book_directory(id);
  fs::File root = LittleFS.open(directory.c_str());
  if (!root || !root.isDirectory()) {
    return false;
  }
  fs::File entry = root.openNextFile();
  while (entry) {
    const std::string path = entry.path();
    entry.close();
    if (!LittleFS.remove(path.c_str())) {
      return false;
    }
    entry = root.openNextFile();
  }
  root.close();
  return LittleFS.rmdir(directory.c_str());
}

bool BookStore::open_book(const std::string_view id, const std::size_t page) {
  if (!save_last(id, page)) {
    return false;
  }
  current_book_ = id;
  current_page_ = page;
  open_handler_(std::string(id), page);
  return page_exists(id, page);
}

bool BookStore::open_page_file(const std::string_view id,
                               const std::size_t page) {
  abort_page();
  final_path_ = page_path(id, page);
  temporary_path_ = final_path_ + ".part";
  LittleFS.remove(temporary_path_.c_str());
  page_file_ = LittleFS.open(temporary_path_.c_str(), FILE_WRITE);
  return static_cast<bool>(page_file_);
}

bool BookStore::append_page(const std::uint8_t* data,
                            const std::size_t length) {
  return page_file_ && page_file_.write(data, length) == length;
}

bool BookStore::commit_page() {
  if (!page_file_) {
    return false;
  }
  page_file_.close();
  fs::File compressed_file =
      LittleFS.open(temporary_path_.c_str(), FILE_READ);
  if (!compressed_file) {
    return false;
  }
  std::vector<std::uint8_t> compressed(compressed_file.size());
  if (compressed_file.read(compressed.data(), compressed.size()) !=
      compressed.size()) {
    return false;
  }
  compressed_file.close();
  std::vector<std::uint8_t> page(800U * 480U / 8U);
  if (!inflate_raw(compressed.data(), compressed.size(), page.data(),
                   page.size())) {
    return false;
  }
  LittleFS.remove(final_path_.c_str());
  const bool renamed =
      LittleFS.rename(temporary_path_.c_str(), final_path_.c_str());
  temporary_path_.clear();
  final_path_.clear();
  return renamed;
}

void BookStore::abort_page() {
  if (page_file_) {
    page_file_.close();
  }
  if (!temporary_path_.empty()) {
    LittleFS.remove(temporary_path_.c_str());
  }
  temporary_path_.clear();
  final_path_.clear();
}

bool BookStore::load_last(std::string& id, std::size_t& page) const {
  fs::File file = LittleFS.open(kStatePath, FILE_READ);
  if (!file) {
    return false;
  }
  JsonDocument document;
  if (deserializeJson(document, file) || !document["id"].is<const char*>() ||
      !document["page"].is<std::size_t>()) {
    return false;
  }
  id = document["id"].as<const char*>();
  page = document["page"].as<std::size_t>();
  return page_exists(id, page);
}

bool BookStore::save_last(const std::string_view id,
                          const std::size_t page) const {
  fs::File file = LittleFS.open(kStatePath, FILE_WRITE);
  if (!file) {
    return false;
  }
  JsonDocument document;
  document["id"] = id;
  document["page"] = page;
  if (serializeJson(document, file) == 0) {
    return false;
  }

  BookInfo book;
  const std::string meta_path = book_meta_path(id);
  if (!read_book_info(meta_path, book)) {
    return false;
  }
  fs::File meta = LittleFS.open(meta_path.c_str(), FILE_READ);
  JsonDocument metadata;
  if (!meta || deserializeJson(metadata, meta)) {
    return false;
  }
  meta.close();
  metadata["last"] = page;
  meta = LittleFS.open(meta_path.c_str(), FILE_WRITE);
  return meta && serializeJson(metadata, meta) > 0;
}

bool BookStore::page_exists(const std::string_view id,
                            const std::size_t page) const {
  return LittleFS.exists(page_path(id, page).c_str());
}

bool BookStore::has_page(const std::string_view id,
                         const std::size_t page) const {
  return page_exists(id, page);
}

}  // namespace pocket
