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
  /// Construct a value from an expression
  template <typename Expr>
  constexpr Value(Expr const & expr)
      : Value(expr.compute()) {}

  /// Constructor from an integer literal, which should be converted 
  /// to an expression before being assigned
  template <std::integral I>
  constexpr Value(I const & val):Value{toExpr(val)}{}
};

} //  namespace apintext

#endif // VALUE_HPP
