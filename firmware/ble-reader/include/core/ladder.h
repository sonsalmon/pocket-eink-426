#pragma once

namespace pocket {

enum class Key {
  None,
  Left,
  Right,
  Center,
};

Key decode_ladder_mv(int millivolts);

}  // namespace pocket
