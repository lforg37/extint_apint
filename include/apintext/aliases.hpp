#ifndef ALIASES_HPP
#define ALIASES_HPP

#include <cstdint>
#include <type_traits>

namespace apintext {

using Signedness = bool;
namespace signedness {
    static constexpr Signedness Signed = true;
    static constexpr Signedness Unsigned = false;
}

template <std::uint32_t width, Signedness signedness>
using ap_repr = typename std::conditional<static_cast<bool>(signedness), signed _ExtInt(width),
                                          unsigned _ExtInt(width)>::type;
} // namespace apintext
#endif
