#ifndef VALUE_HPP
#define VALUE_HPP

#include <concepts>

#include "aliases.hpp"
#include "expression.hpp"

namespace apintext {

template <uint32_t width, bool signedness> class Value : public Expression<Value<width, signedness>>{
 private:
  using res_t = typename Expression<Value>::res_t;
  res_t value;

  constexpr Value(res_t src_repr)
      : value { src_repr } {}

 public:
  /// Construct a value from an expression
  constexpr Value(ExprType auto const & expr)
      : Value(expr.compute()) {}

  /// Constructor from an integer literal, which should be converted 
  /// to an expression before being assigned
  template <std::integral I>
  constexpr Value(I const & val):Value{toExpr(val)}{}
};

} //  namespace apintext

#endif // VALUE_HPP
