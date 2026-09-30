#pragma once

#include "FS.h"

class HostLittleFS {
 public:
  fs::MemoryFS state;
  bool begin(bool) { return true; }
  std::size_t totalBytes() const { return state.capacity; }
  std::size_t usedBytes() const { return state.used(); }
  bool exists(const char* path) const {
    return state.files.count(path) || state.directories.count(path);
  }
  bool mkdir(const char* path) { return state.directories.insert(path).second; }
  bool rmdir(const char* path) { return state.directories.erase(path) != 0; }
  bool remove(const char* path) { return state.files.erase(path) != 0; }
  bool rename(const char* from, const char* to) {
    const auto found = state.files.find(from);
    if (found == state.files.end()) return false;
    state.files[to] = found->second;
    state.files.erase(found);
    return true;
  }
  fs::File open(const char* path, const char* mode = FILE_READ) {
    if (state.directories.count(path)) return fs::File(state, path, true);
    const bool write = std::string(mode) == FILE_WRITE;
    if (write) {
      state.files[path] = std::make_shared<std::vector<std::uint8_t>>();
    }
    if (!state.files.count(path)) return {};
    return fs::File(state, path, false, write);
  }
};

inline HostLittleFS LittleFS;
