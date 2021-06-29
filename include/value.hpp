#ifndef VALUE_HPP
#define VALUE_HPP

#include <cstdint>
#include <type_traits>

namespace apintext {
template <uint32_t width, bool signedness>
using _ap_repr = std::conditionnal<signedness, signed _ExtInt(width), unsigned _ExtInt(width)>::type;
}

#endif // VALUE_HPP
