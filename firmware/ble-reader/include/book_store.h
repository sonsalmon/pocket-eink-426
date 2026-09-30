#pragma once

#include <FS.h>

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include "core/protocol_service.h"

namespace pocket {

using OpenBookHandler =
    std::function<void(const std::string& id, std::size_t page)>;

class BookStore final : public ProtocolBackend {
 public:
  explicit BookStore(OpenBookHandler open_handler);

  bool begin();
  std::size_t free_bytes() const override;
  std::vector<BookInfo> list_books() const override;
  bool create_book(const BookInfo& book) override;
  bool finish_book(std::string_view id) override;
  bool delete_book(std::string_view id) override;
  bool open_book(std::string_view id, std::size_t page) override;
  bool configure_window(std::string_view id, std::size_t from,
                        std::size_t to) override;
  PagePrepareResult prepare_page(std::string_view id, std::size_t page,
                                 std::size_t length) override;
  bool append_page(const std::uint8_t* data, std::size_t length) override;
  bool commit_page() override;
  void abort_page() override;

  bool load_last(std::string& id, std::size_t& page) const;
  bool has_page(std::string_view id, std::size_t page) const;
  NeedRange need_request(std::string_view id, std::size_t page) const;

 private:
  bool save_last(std::string_view id, std::size_t page) const;
  bool page_exists(std::string_view id, std::size_t page) const;
  bool open_page_file(std::string_view id, std::size_t page);
  bool set_window_mode(std::string_view id, bool enabled) const;
  bool read_window_mode(std::string_view id) const;
  std::vector<std::size_t> list_pages(std::string_view id) const;
  std::vector<CachedPage> cached_pages() const;

  OpenBookHandler open_handler_;
  fs::File page_file_;
  std::string temporary_path_;
  std::string final_path_;
};

}  // namespace pocket
