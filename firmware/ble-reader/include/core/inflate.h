#pragma once

#include <cstddef>
#include <cstdint>

namespace pocket {

bool inflate_raw(const std::uint8_t* compressed, std::size_t compressed_length,
                 std::uint8_t* output, std::size_t output_length);

}  // namespace pocket
