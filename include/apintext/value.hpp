#ifndef VALUE_HPP
#define VALUE_HPP

#include <concepts>

#include "aliases.hpp"
#include "expression.hpp"

namespace apintext {
template <uint32_t width, bool signedness>
using ap_repr = typename std::conditional<signedness, signed _ExtInt(width),
                                          unsigned _ExtInt(width)>::type;

template <uint32_t width, bool signedness> class Value {
 private:
  using repr_t = ap_repr<width, signedness>;
  repr_t repr;

  constexpr Value(repr_t src_repr)
      : repr { src_repr } {}

 public:
  template <template <uint32_t, bool> typename Expr, uint32_t src_width,
            bool src_signedness>
  constexpr Value(Expr<src_width, src_signedness> expr)
      : Value(expr.compute()) {}

  template <std::integral I>
  constexpr Value(I const & val):Value{toExpr(val)}{}
};

} //  namespace apintext

#endif // VALUE_HPP
