#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "aliases.hpp"
#include "arith_prop.hpp"

namespace apintext {

template <typename T>
concept ExprType = requires(T const& val) {
  { val.compute() } -> std::same_as<ap_repr<T::width, T::signedness>>;
};

//*************** Constant expression **************************************//
template <uint32_t w, bool s> class ConstantExpr {
 private:
  using val_t = ap_repr<w, s>;

 public:
  static constexpr uint32_t width = w;
  static constexpr bool signedness = s;

  constexpr ConstantExpr(val_t const& src_repr)
      : value { src_repr } {}
  constexpr val_t compute() const { return value; };

 private:
  const val_t value;
};

template <std::integral T> constexpr uint32_t getWidth() {
  return std::numeric_limits<std::make_unsigned_t<T>>::digits;
}

template <std::integral T>
constexpr ConstantExpr<getWidth<T>(), std::is_signed<T>::value>
toExpr(T const& value) {
  return { value };
}

template <uint32_t w>
constexpr ConstantExpr<w, false> toExpr(unsigned _ExtInt(w) const& value) {
  return { value };
}

template <uint32_t w>
constexpr ConstantExpr<w, true> toExpr(signed _ExtInt(w) const& value) {
  return { value };
}

//*************** Arithmetic expression ***********************************//
template <ExprType ET1, ExprType ET2>
using ExprArithProp =
    ArithmeticProp<ET1::width, ET2::width, ET1::signedness, ET2::signedness>;

template <ExprType ET1, ExprType ET2> class ExprProd {
 private:
  using prop = ExprArithProp<ET1, ET2>;

 public:
  static constexpr uint32_t width = prop::prodWidth;
  static constexpr bool signedness = prop::prodSigned;
  using res_t = ap_repr<width, signedness>;
  ET1 const leftOp;
  ET2 const rightOp;

 public:
  constexpr ExprProd(ET1 const& val1, ET2 const& val2)
      : leftOp { val1 }
      , rightOp { val2 } {}

  constexpr res_t compute() const {
    auto lExt = static_cast<res_t>(leftOp.compute());
    auto rExt = static_cast<res_t>(rightOp.compute());
    return { lExt * rExt };
  }
};

template <ExprType ET1, ExprType ET2>
constexpr ExprProd<ET1, ET2> operator*(ET1 const& expr1, ET2 const& expr2) {
  return { expr1, expr2 };
}

template <ExprType ET1, ExprType ET2, bool sub> class ExprSumBase {
 private:
  using prop = ExprArithProp<ET1, ET2>;

 public:
  static constexpr uint32_t width = prop::sumWidth;
  static constexpr bool signedness = prop::sumSigned;

 private:
  using res_t = ap_repr<width, signedness>;
  ET1 const leftOp;
  ET2 const rightOp;

 public:
  constexpr ExprSumBase(ET1 const& val1, ET2 const& val2)
      : leftOp { val1 }
      , rightOp { val2 } {}
  constexpr res_t compute() const {
    auto lExt = static_cast<res_t>(leftOp.compute());
    auto rExt = static_cast<res_t>(rightOp.compute());
    if constexpr (sub) {
      return { lExt - rExt };
    } else {
      return { lExt + rExt };
    }
  }
};

template <ExprType ET1, ExprType ET2>
using ExprSum = ExprSumBase<ET1, ET2, false>;

template <ExprType ET1, ExprType ET2>
constexpr ExprSum<ET1, ET2> operator+(ET1 const& expr1, ET2 const& expr2) {
  return { expr1, expr2 };
}

template <ExprType ET1, ExprType ET2>
using ExprSub = ExprSumBase<ET1, ET2, true>;

template <ExprType ET1, ExprType ET2>
constexpr ExprSub<ET1, ET2> operator-(ET1 const& expr1, ET2 const& expr2) {
  return { expr1, expr2 };
}

template <bool targetSignedness, ExprType SourceType>
class ReinterpretSignExpr {
 public:
  static_assert(targetSignedness != SourceType::signedness,
                "Attempt to insert useless ReinterpretSignExpr");
  static constexpr uint32_t width = SourceType::width;
  static constexpr bool signedness = targetSignedness;

 private:
  SourceType const source;
  using res_t = ap_repr<width, signedness>;

 public:
  constexpr ReinterpretSignExpr(SourceType const& src)
      : source { src } {}
  constexpr res_t compute() const {
    return static_cast<res_t>(source.compute());
  }
};

template <uint32_t targetWidth, ExprType SourceType> class ZExtExpr {
 public:
  static_assert(targetWidth > SourceType::width,
                "Attempt to perform zero extension with target width smaller "
                "than source width.");
  static constexpr uint32_t width = targetWidth;
  static constexpr bool signedness = SourceType::signedness;

 private:
  SourceType const source;
  using res_t = ap_repr<width, signedness>;

 public:
  constexpr ZExtExpr(SourceType const& src)
      : source { src } {}
  constexpr res_t compute() const {
    return static_cast<res_t>(
        static_cast<ap_repr<SourceType::width, false>>(source.compute()));
  };
};

template <uint32_t targetWidth, ExprType ET>
constexpr auto zeroExtendToWidth(ET const& source) {
  return ZExtExpr<targetWidth, ET> { source };
}

template <uint32_t targetWidth, ExprType SourceType> class SignExtExpr {
 public:
  static_assert(targetWidth > SourceType::width,
                "Attempt to perform sign extension with target width smaller "
                "than source width.");
  static constexpr uint32_t width = targetWidth;
  static constexpr bool signedness = SourceType::signedness;

 private:
  SourceType const source;
  using res_t = ap_repr<width, signedness>;

 public:
  constexpr SignExtExpr(SourceType const& src)
      : source { src } {}
  constexpr res_t compute() const {
    return static_cast<res_t>(source.compute());
  };
};

template <uint32_t targetWidth, ExprType ET>
constexpr auto signExtendToWidth(ET const& source) {
  return SignExtExpr<targetWidth, ET>(source);
}

//**************** Operation on bit vector ********************************//

template <uint32_t highBit, uint32_t lowBit, ExprType SourceType>
class SliceExpr {
 public:
  static_assert(highBit >= lowBit,
                "Slicing high index should be greater than low index");
  static_assert(SourceType::width > highBit,
                "Trying to slice out of input bounds");
  static constexpr uint32_t width = highBit - lowBit + 1;
  static constexpr bool signedness = false;

 private:
  SourceType const source;
  using res_t = ap_repr<width, signedness>;

 public:
  constexpr SliceExpr(SourceType const& sourceExpr)
      : source { sourceExpr } {}
  constexpr res_t compute() const {
    return static_cast<res_t>(
        static_cast<ap_repr<highBit + 1, false>>(source.compute()) >> lowBit);
  }
};

template <unsigned int bitIdx, ExprType ET> class GetBitExpr {
 public:
  static_assert(bitIdx < ET::width,
                "Trying to access bit outside of input range");
  static constexpr uint32_t width = 1;
  static constexpr bool signedness = false;

 private:
  using res_t = ap_repr<width, signedness>;
  using intermediate_t = ap_repr<bitIdx + 1, false>;
  ET const source;

 public:
  constexpr GetBitExpr(ET const& src)
      : source { src } {};
  constexpr res_t compute() const {
    constexpr intermediate_t mask = intermediate_t { 1 } << bitIdx;
    return { (static_cast<intermediate_t>(source.compute()) & mask) != intermediate_t{0} };
  }
};

template <uint32_t idx, ExprType ET> constexpr auto getBit(ET const& src) {
  return GetBitExpr<idx, ET> { src };
}

} // namespace apintext

#endif
