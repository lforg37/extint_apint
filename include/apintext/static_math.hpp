#ifndef APINTEXT_STATIC_MATH_HPP
#define APINTEXT_STATIC_MATH_HPP
#include <cstdint>

namespace apintext {
template <std::uint32_t N> constexpr std::uint32_t log2() {
  if constexpr (N < 2) {
    static_assert(N == 1, "Trying to compute log2(0).");
    return 0;
  } else {
    return 1 + log2<(N >> 1)>();
  }
}
} // namespace apintext
#endif
