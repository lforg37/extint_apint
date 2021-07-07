#ifndef ALIASES_HPP
#define ALIASES_HPP

#include <cstdint>
#include <type_traits>

namespace apintext {
template <std::uint32_t width, bool signedness>
using ap_repr = typename std::conditional<signedness, signed _ExtInt(width),
                                          unsigned _ExtInt(width)>::type;
} // namespace apintext
#endif
