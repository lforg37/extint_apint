#include "apintext.hpp"
#include <cstdint>

using namespace apintext;

template <typename T> struct IsExpr { static constexpr bool res = false; };

template <ExprType ET> struct IsExpr<ET> { static constexpr bool res = true; };

struct Dummy {};

int main() {
  static_assert(IsExpr<ConstantExpr<32, false>>::res,
                "ConstantExpr is not an expression");
  static_assert(!IsExpr<Dummy>::res,
                "Dummy should not be considered an expression");
  static_assert(IsExpr<
    ReinterpretSignExpr<
    true, 
    SliceExpr<
        0, 0, ConstantExpr<
            32, true>
        >
    >>::res,
                "ReinterpretSignExpr is not an expression");
  Value<32, false> { uint32_t { 14 } };
  return 0;
}
