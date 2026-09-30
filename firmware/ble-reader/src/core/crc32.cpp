#include "core/crc32.h"

namespace pocket {

std::uint32_t crc32_update(std::uint32_t state, const std::uint8_t* data,
                           const std::size_t length) {
  for (std::size_t index = 0; index < length; ++index) {
    state ^= data[index];
    for (int bit = 0; bit < 8; ++bit) {
      const std::uint32_t mask = 0U - (state & 1U);
      state = (state >> 1U) ^ (0xEDB88320U & mask);
    }
  }
  return state;
}

std::uint32_t crc32(const std::uint8_t* data, const std::size_t length) {
  return ~crc32_update(0xFFFFFFFFU, data, length);
}

}  // namespace pocket
