#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#define FILE_READ "r"
#define FILE_WRITE "w"

namespace fs {

struct MemoryFS {
  std::map<std::string, std::shared_ptr<std::vector<std::uint8_t>>> files;
  std::set<std::string> directories;
  std::size_t capacity = 1024 * 1024;

  std::size_t used() const {
    std::size_t bytes = 0;
    for (const auto& entry : files) {
      bytes += entry.second->size();
    }
    return bytes;
  }
};

class File {
 public:
  File() = default;
  File(MemoryFS& fs, std::string path, bool directory, bool writable = false)
      : state_(std::make_shared<State>()) {
    state_->fs = &fs;
    state_->path = std::move(path);
    state_->directory = directory;
    state_->writable = writable;
    if (!directory) {
      state_->bytes = fs.files.at(state_->path);
    } else {
      const std::string prefix = state_->path + "/";
      for (const auto& entry : fs.files) {
        if (entry.first.compare(0, prefix.size(), prefix) == 0 &&
            entry.first.find('/', prefix.size()) == std::string::npos) {
          state_->children.push_back(entry.first);
        }
      }
      for (const auto& entry : fs.directories) {
        if (entry.compare(0, prefix.size(), prefix) == 0 &&
            entry.find('/', prefix.size()) == std::string::npos) {
          state_->children.push_back(entry);
        }
      }
    }
  }
  explicit operator bool() const { return state_ && !state_->closed; }
  bool isDirectory() const { return state_ && state_->directory; }
  const char* path() const { return state_->path.c_str(); }
  std::size_t size() const { return state_->bytes->size(); }
  void close() {
    if (state_) state_->closed = true;
  }
  File openNextFile() {
    if (state_->next == state_->children.size()) return {};
    const std::string child = state_->children[state_->next++];
    return File(*state_->fs, child, state_->fs->directories.count(child));
  }
  int read() {
    if (state_->position == size()) return -1;
    return (*state_->bytes)[state_->position++];
  }
  std::size_t read(std::uint8_t* out, std::size_t length) {
    length = std::min(length, size() - state_->position);
    std::memcpy(out, state_->bytes->data() + state_->position, length);
    state_->position += length;
    return length;
  }
  std::size_t readBytes(char* out, std::size_t length) {
    return read(reinterpret_cast<std::uint8_t*>(out), length);
  }
  std::size_t write(std::uint8_t byte) { return write(&byte, 1); }
  std::size_t write(const std::uint8_t* bytes, std::size_t length) {
    if (!*this || !state_->writable) return 0;
    const std::size_t available = state_->fs->capacity - state_->fs->used();
    length = std::min(length, available);
    state_->bytes->insert(state_->bytes->end(), bytes, bytes + length);
    return length;
  }

 private:
  struct State {
    MemoryFS* fs = nullptr;
    std::string path;
    std::shared_ptr<std::vector<std::uint8_t>> bytes;
    std::vector<std::string> children;
    std::size_t position = 0;
    std::size_t next = 0;
    bool directory = false;
    bool writable = false;
    bool closed = false;
  };
  std::shared_ptr<State> state_;
};

}  // namespace fs
