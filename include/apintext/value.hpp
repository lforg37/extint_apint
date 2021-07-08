#ifndef VALUE_HPP
#define VALUE_HPP

#include <concepts>
#include <type_traits>

#include "aliases.hpp"
#include "expression.hpp"

namespace apintext {

template <uint32_t w, Signedness s, typename ExtensionPolicy = SignExtension,
          typename TruncationPolicy = Truncation,
          typename WrongSignPolicy = ReinterpretSign>
class Value {
 public:
  static constexpr uint32_t width = w;
  static constexpr Signedness signedness = s;

 private:
  using val_t = ap_repr<w, s>;
  val_t value;
  using adaptor = Adaptor<ExtensionPolicy, TruncationPolicy, WrongSignPolicy>;

 public:
  constexpr Value(val_t src_repr)
      : value { src_repr } {}

  /// Construct a value from an expression with target signedness and width
  template <ExprType SrcType>
  constexpr Value(SrcType const& expr)
      : Value(adaptor::template adapt<width, signedness>(expr).compute()) {}

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

  template <std::integral IT> constexpr explicit operator IT() const {
    return static_cast<IT>(
        adaptor::template adapt<getWidth<IT>(), Signedness{std::is_signed<IT>::value}>(
            *this)
            .compute());
  }
};

template <std::integral IT, typename ExtensionPolicy = SignExtension,
          typename TruncationPolicy = Truncation,
          typename WrongSignPolicy = ReinterpretSign>
constexpr IT getAs(ExprType auto const& expression) {
  return static_cast<IT>(
      Value<getWidth<IT>(), Signedness { std::is_signed<IT>::value },
            ExtensionPolicy, TruncationPolicy, WrongSignPolicy> { expression }
          .compute());
}

} //  namespace apintext

#endif // VALUE_HPP
