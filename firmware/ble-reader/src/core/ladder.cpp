#include "core/ladder.h"

namespace pocket {

Key decode_ladder_mv(const int millivolts) {
  if (millivolts >= 1780 && millivolts <= 2140) {
    return Key::Left;
  }
  if (millivolts >= 1140 && millivolts <= 1500) {
    return Key::Right;
  }
  if (millivolts >= 0 && millivolts <= 250) {
    return Key::Center;
  }
  return Key::None;
}

}  // namespace pocket
