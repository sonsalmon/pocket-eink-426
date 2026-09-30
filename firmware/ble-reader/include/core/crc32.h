#pragma once

#include <cstddef>
#include <cstdint>

namespace pocket {

std::uint32_t crc32_update(std::uint32_t state, const std::uint8_t* data,
                           std::size_t length);
std::uint32_t crc32(const std::uint8_t* data, std::size_t length);

}  // namespace pocket
