#ifndef ALIASES_HPP
#define ALIASES_HPP

#include <cstdint>
#include <type_traits>

namespace apintext {

// Signedness is represented as bool
struct Signedness {
  const bool signedness;
  constexpr Signedness(bool s)
      : signedness { s } {}
  constexpr operator bool() const { return signedness; }
};
namespace signedness {
// Some signedness aliases
static constexpr Signedness Signed { true };
static constexpr Signedness Unsigned { false };
} // namespace signedness

template <std::uint32_t width, Signedness signedness>
using ap_repr =
    typename std::conditional<static_cast<bool>(signedness), signed _ExtInt(width),
                              unsigned _ExtInt(width)>::type;
} // namespace apintext
#endif
