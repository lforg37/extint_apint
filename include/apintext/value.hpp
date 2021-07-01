#ifndef VALUE_HPP
#define VALUE_HPP

#include <concepts>
#include <type_traits>

#include "aliases.hpp"
#include "expression.hpp"

namespace apintext {

struct Truncation {
  template <uint32_t targetWidth, ExprType ET>
  static constexpr auto truncate(ET const& source) {
    static_assert(ET::width > targetWidth,
                  "Trying to truncate expression to a bigger target width.");
    return SliceExpr<targetWidth - 1, 0, ET> { source };
  }
};

struct ReinterpretSign {
  template <ExprType ET> static constexpr auto handleSign(ET const& source) {
    return ReinterpretSignExpr<!ET::signedness, ET>(source);
  }
};

struct Forbid {
  template <uint32_t targetWidth, ExprType ET>
  static constexpr auto truncate(ET const& source) {
    static_assert(ET::width > targetWidth,
                  "Trying to truncate expression to a bigger target width.");
    static_assert(ET::width < 0, "Trying to perform forbidden truncation");
  }

  template <ExprType ET> static constexpr auto handleSign(ET const& source) {
    return ReinterpretSignExpr<!ET::signedness, ET>(source);
  }
};

template <uint32_t w, bool s, typename TruncationPolicy = Truncation,
          typename WrongSignPolicy = ReinterpretSign>
class Value {
 public:
  static constexpr uint32_t width = w;
  static constexpr bool signedness = s;

 private:
  using val_t = ap_repr<w, s>;
  val_t value;

  constexpr Value(val_t src_repr)
      : value { src_repr } {}

 public:
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

  /// Constructor from an integer literal, which should be converted
  /// to an expression before being assigned
  template <std::integral I>
  constexpr Value(I const& val)
      : Value { toExpr(val) } {}

  constexpr val_t compute() const { return value; }
};

} //  namespace apintext

#endif // VALUE_HPP
