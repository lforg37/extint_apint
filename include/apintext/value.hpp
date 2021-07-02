#ifndef VALUE_HPP
#define VALUE_HPP

#include <concepts>
#include <type_traits>

#include "aliases.hpp"
#include "expression.hpp"
#include "policies.hpp"

namespace apintext {

template <uint32_t w, bool s, typename ExtensionPolicy = SignExtension,
          typename TruncationPolicy = Truncation,
          typename WrongSignPolicy = ReinterpretSign>
class Value {
 public:
  static constexpr uint32_t width = w;
  static constexpr bool signedness = s;

 private:
  using val_t = ap_repr<w, s>;
  val_t value;

 public:
  constexpr Value(val_t src_repr)
      : value { src_repr } {}

  /// Construct a value from an expression with target signedness and width
  template <ExprType SrcType>
  constexpr Value(SrcType const& expr,
                  typename std::enable_if<SrcType::signedness == signedness &&
                                          SrcType::width == width>::type* = 0)
      : Value(expr.compute()) {}

  /// Construct a value from an expression which has target width but inexact
  /// signedness
  template <ExprType SrcType>
  constexpr Value(
      SrcType const& expr,
      typename std::enable_if<SrcType::width == width &&
                              SrcType::signedness != signedness>::type* = 0)
      : Value { WrongSignPolicy::handleSign(expr) } {}

  /// Construct a value from an expression which is too wide
  template <ExprType SrcType>
  constexpr Value(SrcType const& expr,
                  typename std::enable_if<(SrcType::width > width)>::type* = 0)
      : Value { TruncationPolicy::template truncate<width>(expr) } {}

  /// Construct a value from an expression which is too small
  template <ExprType SrcType>
  constexpr Value(SrcType const& expr,
                  typename std::enable_if<((SrcType::width < width))>::type* = 0)
      : Value { ExtensionPolicy::template extend<width>(expr) } {}

  /// Constructor from an integer literal, which should be converted
  /// to an expression before being assigned
  template <std::integral I>
  constexpr Value(I const& val)
      : Value { toExpr(val) } {}

  template <uint32_t ws>
  constexpr Value(unsigned _ExtInt(ws) const& val)
      : Value { toExpr(val) } {}

  template <uint32_t ws>
  constexpr Value(signed _ExtInt(ws) const& val)
      : Value { toExpr(val) } {}

  constexpr val_t compute() const { return value; }
};

template <std::integral IT, typename ExtensionPolicy = SignExtension,
          typename TruncationPolicy = Truncation,
          typename WrongSignPolicy = ReinterpretSign>
constexpr IT getAs(ExprType auto const& expression) {
  return static_cast<IT>(
      Value<getWidth<IT>(), std::is_signed<IT>::value, ExtensionPolicy,
            TruncationPolicy, WrongSignPolicy> { expression }
          .compute());
}

} //  namespace apintext

#endif // VALUE_HPP
