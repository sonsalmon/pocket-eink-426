#include "core/inflate.h"

#include <miniz.h>

namespace pocket {

bool inflate_raw(const std::uint8_t* compressed,
                 const std::size_t compressed_length, std::uint8_t* output,
                 const std::size_t output_length) {
  const std::size_t written = tinfl_decompress_mem_to_mem(
      output, output_length, compressed, compressed_length, 0);
  return written == output_length;
}

}  // namespace pocket
