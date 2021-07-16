#include "apintext.hpp"
#include <cstdint>

using namespace apintext;

template <typename T> struct IsExpr { static constexpr bool res = false; };

template <ExprType ET> struct IsExpr<ET> { static constexpr bool res = true; };

struct Dummy {};

int main() {
  static_assert(IsExpr<ConstantExpr<32, signedness::Unsigned>>::res,
                "ConstantExpr is not an expression");
  static_assert(!IsExpr<Dummy>::res,
                "Dummy should not be considered an expression");
  static_assert(IsExpr<ZExtExpr<32, ConstantExpr<1, signedness::Unsigned>>>::res, "ZExtExpr is not an expression");
  static_assert(IsExpr<
    ReinterpretSignExpr<
    signedness::Signed, 
    SliceExpr<
        0, 0, ConstantExpr<
            32, signedness::Signed>
        >
    >>::res,
                "ReinterpretSignExpr is not an expression");
  Value<32, signedness::Unsigned> { uint32_t { 14 } };
  return 0;
}
