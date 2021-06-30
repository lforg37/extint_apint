#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "aliases.hpp"

namespace apintext {

template <typename EType> class Expression;

template <uint32_t w, bool s, template <uint32_t, bool> typename ExprType>
class Expression<ExprType<w, s>> {
  /// CRTP base class for expressions
  constexpr static uint32_t width = w;
  constexpr static bool signedness = s;

 public:
  using res_t = ap_repr<width, signedness>;
  constexpr auto compute() const {
    return reinterpret_cast<ExprType<w, s> const&>(*this).do_compute();
  }
};

template<typename T> concept ExprType = std::is_base_of<Expression<T>, T>::value;

template <uint32_t w, bool s>
class ConstantExpr : public Expression<ConstantExpr<w, s>> {
 private:
    using res_t = typename Expression<ConstantExpr>::res_t;
 public:
  constexpr ConstantExpr(res_t const& src_repr)
      : value { src_repr } {}
  constexpr res_t do_compute() const { return value; };

 private:
  const res_t value;
};

template <std::integral T> constexpr uint32_t getWidth() {
  return std::numeric_limits<std::make_unsigned_t<T>>::digits;
}

template <std::integral T>
constexpr ConstantExpr<getWidth<T>(), std::is_signed<T>::value>
toExpr(T const& value) {
  return { value };
}

} // namespace apintext

#endif
