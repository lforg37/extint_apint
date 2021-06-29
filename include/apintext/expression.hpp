#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "aliases.hpp"

namespace apintext {

template <uint32_t width, bool signedness> class ValueExpr {
 private:
  using repr_t = ap_repr<width, signedness>;
  repr_t repr;

 public:
  constexpr ValueExpr(repr_t const & src_repr)
      : repr { src_repr } {}
  constexpr repr_t compute() { return repr; }
};

template <std::integral T> constexpr uint32_t getWidth() {
  return std::numeric_limits<std::make_unsigned_t<T>>::digits;
}

template <std::integral T>
constexpr ValueExpr<getWidth<T>(), std::is_signed<T>::value>
toExpr(T const& value) {
  return { value };
}

} // namespace apintext

#endif
