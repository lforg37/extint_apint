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

template <ExprType ET> using res_t = ap_repr<ET::width, ET::signedness>;

template <ExprType E1, ExprType E2> struct TightOverset {
 private:
  static constexpr bool widestIsE1 = E1::width == E2::width;
  static constexpr uint32_t maxWidth =
      (E1::width > E2::width) ? E1::width : E2::width;
  static constexpr bool sameSignedness = E1::signedness == E2::signedness;
  static constexpr bool oneIsSigned = E1::signedness || E2::signedness;

 public:
  static constexpr bool signedness = oneIsSigned;
  static constexpr uint32_t width = (sameSignedness) ? maxWidth : maxWidth + 1;
};

//****************** Adaptor **********************************//

template <typename ExtensionPolicy, typename TruncationPolicy,
          typename WrongSignPolicy>
struct Adaptor {
  template <uint32_t targetWidth, bool targetSignedness, ExprType ET>
  static constexpr auto adapt(ET const& source) {
    constexpr uint32_t sourceWidth = ET::width;
    constexpr bool sourceSignedness = ET::signedness;
    if constexpr (targetWidth > sourceWidth) {
      return adapt<targetWidth, targetSignedness>(
          ExtensionPolicy::template extend<targetWidth>(source));
    } else if constexpr (targetWidth < sourceWidth) {
      return adapt<targetWidth, targetSignedness>(
          TruncationPolicy::template truncate<targetWidth>(source));
    } else if constexpr (targetSignedness != sourceSignedness) {
      return adapt<targetWidth, targetSignedness>(
          WrongSignPolicy::template setSignedness<targetSignedness>(source));
    } else {
      return source;
    }
  }
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

template <uint32_t highBit, uint32_t lowBit, ExprType ET>
constexpr auto slice(ET const& source) {
  return SliceExpr<highBit, lowBit, ET> { source };
}

template <uint32_t bitIdx, ExprType ET> class GetBitExpr {
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
    return { (static_cast<intermediate_t>(source.compute()) & mask) !=
             intermediate_t { 0 } };
  }
};

template <uint32_t idx, ExprType ET> constexpr auto getBit(ET const& src) {
  return GetBitExpr<idx, ET> { src };
}

template <ExprType ET1, ExprType ET2, typename Operation>
class BitwiseLogicExpr {
 public:
  static constexpr uint32_t width = ET1::width;
  static constexpr bool signedness = false;

 private:
  using res_t = ap_repr<width, signedness>;
  ET1 const leftOp;
  ET2 rightOp;

 public:
  constexpr BitwiseLogicExpr(ET1 const& left, ET1 const& right)
      : leftOp { left }
      , rightOp { right } {
    Operation::template check<ET1, ET2>();
  }
  constexpr res_t compute() const {
    return Operation::compute(leftOp, rightOp);
  }
};

struct BitwiseAND {
  template <ExprType ET1, ExprType ET2> static constexpr void check() {
    static_assert(
        ET1::width == ET2::width,
        "Trying to perform bitwise AND on operands of differrent widths.");
  }
  template <ExprType ET1, ExprType ET2>
  static constexpr ap_repr<ET1::width, false> compute(ET1 const& left,
                                                      ET2 const& right) {
    check<ET1, ET2>();
    return { left.compute() & right.compute() };
  }
};

template <ExprType ET1, ExprType ET2>
using BitwiseANDExpr = BitwiseLogicExpr<ET1, ET2, BitwiseAND>;

struct BitwiseOR {
  template <ExprType ET1, ExprType ET2> static constexpr void check() {
    static_assert(
        ET1::width == ET2::width,
        "Trying to perform bitwise OR on operands of differrent widths.");
  }
  template <ExprType ET1, ExprType ET2>
  static constexpr ap_repr<ET1::width, false> compute(ET1 const& left,
                                                      ET2 const& right) {
    check<ET1, ET2>();
    return { left.compute() | right.compute() };
  }
};

template <ExprType ET1, ExprType ET2>
using BitwiseORExpr = BitwiseLogicExpr<ET1, ET2, BitwiseOR>;

struct BitwiseXOR {
  template <ExprType ET1, ExprType ET2> static constexpr void check() {
    static_assert(
        ET1::width == ET2::width,
        "Trying to perform bitwise XOR on operands of differrent widths.");
  }
  template <ExprType ET1, ExprType ET2>
  static constexpr ap_repr<ET1::width, false> compute(ET1 const& left,
                                                      ET2 const& right) {
    check<ET1, ET2>();
    return { left.compute() ^ right.compute() };
  }
};

template <ExprType ET1, ExprType ET2>
using BitwiseXORExpr = BitwiseLogicExpr<ET1, ET2, BitwiseXOR>;

template <ExprType ET1, ExprType ET2>
constexpr auto operator|(ET1 const& left, ET2 const& right) {
  return BitwiseORExpr<ET1, ET2> { left, right };
}

template <ExprType ET1, ExprType ET2>
constexpr auto operator&(ET1 const& left, ET2 const& right) {
  return BitwiseANDExpr<ET1, ET2> { left, right };
}

template <ExprType ET1, ExprType ET2>
constexpr auto operator^(ET1 const& left, ET2 const& right) {
  return BitwiseXORExpr<ET1, ET2> { left, right };
}

template <ExprType ET> class BitInvertExpr {
 public:
  static constexpr uint32_t width = ET::width;
  static constexpr bool signedness = false;

 private:
  using res_t = ap_repr<width, signedness>;
  ET const source;

 public:
  constexpr BitInvertExpr(ET const& src)
      : source { src } {}

  constexpr res_t compute() const { return { ~source.compute() }; }
};

template <ExprType ET> constexpr auto operator~(ET const& src) {
  return BitInvertExpr<ET> { src };
}

template <ExprType ET, typename Reduction> class ReductionExpr {
 public:
  static constexpr uint32_t width = 1;
  static constexpr bool signedness = false;

 private:
  using res_t = ap_repr<width, signedness>;
  ET const source;

 public:
  constexpr ReductionExpr(ET const& src)
      : source { src } {}
  constexpr res_t compute() const { return Reduction::compute(source); }
};

struct ORReduction {
  template <ExprType ET>
  static constexpr ap_repr<1, false> compute(ET const& src) {
    constexpr res_t<ET> zero { 0 };
    return { src.compute() != zero };
  }
};

struct ANDReduction {
  template <ExprType ET>
  static constexpr ap_repr<1, false> compute(ET const& src) {
    constexpr res_t<ET> zero { 0 };
    constexpr res_t<ET> fullOne = ~zero;
    return { src.compute() == fullOne };
  }
};

struct NORReduction {
  template <ExprType ET>
  static constexpr ap_repr<1, false> compute(ET const& src) {
    constexpr res_t<ET> zero { 0 };
    return { src.compute() == zero };
  }
};

template <ExprType ET> using ORReductionExpr = ReductionExpr<ET, ORReduction>;

template <ExprType ET> using NORReductionExpr = ReductionExpr<ET, NORReduction>;

template <ExprType ET> using ANDReductionExpr = ReductionExpr<ET, ANDReduction>;

template <ExprType ET> constexpr auto orReduce(ET const& source) {
  return ORReductionExpr<ET> { source };
}

template <ExprType ET> constexpr auto norReduce(ET const& source) {
  return NORReductionExpr<ET> { source };
}

template <ExprType ET> constexpr auto andReduce(ET const& source) {
  return ANDReductionExpr<ET> { source };
}

//************* Policies *********************************************//
struct Truncation {
  template <uint32_t targetWidth, ExprType ET>
  static constexpr auto truncate(ET const& source) {
    static_assert(ET::width > targetWidth,
                  "Trying to truncate expression to a bigger target width.");
    return SliceExpr<targetWidth - 1, 0, ET> { source };
  }
};

struct ReinterpretSign {
  template <bool targetSignedness, ExprType ET>
  static constexpr auto setSignedness(ET const& source) {
    return ReinterpretSignExpr<targetSignedness, ET>(source);
  }
};

struct ZeroExtension {
  template <uint32_t targetWidth, ExprType ET>
  static constexpr auto extend(ET const& source) {
    return zeroExtendToWidth<targetWidth>(source);
  }
};

struct SignExtension {
  template <uint32_t targetWidth, ExprType ET>
  static constexpr auto extend(ET const& source) {
    return signExtendToWidth<targetWidth>(source);
  }
};

struct Forbid {
  template <uint32_t targetWidth, ExprType ET>
  static constexpr auto truncate(ET const& source) {
    static_assert(ET::width > targetWidth,
                  "Trying to truncate expression to a bigger target width.");
    static_assert(ET::width < 0, "Trying to perform forbidden truncation");
  }

  template <bool, ExprType ET>
  static constexpr auto setSignedness(ET const& source) {
    static_assert(ET::width < 0, "Trying to perform forbidden sign conversion");
  }

  template <uint32_t targetWidth, ExprType ET>
  static constexpr auto extend(ET const&) {
    static_assert(ET::width < 0, "Trying to perform forbiden width extension");
  }
};

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

template <ExprType ET1, ExprType ET2> class ExprDiv {
  using prop = ExprArithProp<ET1, ET2>;

 public:
  static constexpr uint32_t width = prop::divWidth;
  static constexpr bool signedness = prop::divSigned;
  using res_t = ap_repr<width, signedness>;
  ET1 const leftOp;
  ET2 const rightOp;

 public:
  constexpr ExprDiv(ET1 const& val1, ET2 const& val2)
      : leftOp { val1 }
      , rightOp { val2 } {}

  constexpr res_t compute() const {
    using tightOverset = TightOverset<ET1, ET2>;
    constexpr bool bothSigned = ET1::signedness && ET2::signedness;
    constexpr uint32_t toWidth =
        (bothSigned && (tightOverset::width == ET1::width))
            ? tightOverset::width + 1
            : tightOverset::width;
    constexpr bool toSign = tightOverset::signedness;
    using adaptor = Adaptor<SignExtension, Forbid, ReinterpretSign>;
    return static_cast<res_t>(
        adaptor::template adapt<toWidth, toSign>(leftOp).compute() /
        adaptor::template adapt<toWidth, toSign>(rightOp).compute());
  }
};

template <ExprType ET1, ExprType ET2>
constexpr ExprDiv<ET1, ET2> operator/(ET1 const& expr1, ET2 const& expr2) {
  return { expr1, expr2 };
}

template <ExprType ET1, ExprType ET2> class ExprMod {
  using prop = ExprArithProp<ET1, ET2>;

 public:
  static constexpr uint32_t width = prop::modWidth;
  static constexpr bool signedness = prop::modSigned;
  using res_t = ap_repr<width, signedness>;
  ET1 const leftOp;
  ET2 const rightOp;

 public:
  constexpr ExprMod(ET1 const& val1, ET2 const& val2)
      : leftOp { val1 }
      , rightOp { val2 } {}

  constexpr res_t compute() const {
    using tightOverset = TightOverset<ET1, ET2>;
    constexpr bool bothSigned = ET1::signedness && ET2::signedness;
    constexpr uint32_t toWidth =
        (bothSigned && (tightOverset::width == ET1::width))
            ? tightOverset::width + 1
            : tightOverset::width;
    constexpr bool toSign = tightOverset::signedness;

    using adaptor = Adaptor<SignExtension, Forbid, ReinterpretSign>;
    return static_cast<res_t>(
        adaptor::template adapt<toWidth, toSign>(leftOp).compute() %
        adaptor::template adapt<toWidth, toSign>(rightOp).compute());
  }
};

template <ExprType ET1, ExprType ET2>
constexpr ExprMod<ET1, ET2> operator%(ET1 const& expr1, ET2 const& expr2) {
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

//*************** Comparisons *********************************************//

template <ExprType ET1, ExprType ET2>
constexpr auto operator<=>(ET1 const& lhs, ET2 const& rhs) {
  using tightOverset = TightOverset<ET1, ET2>;
  constexpr uint32_t toWidth = tightOverset::width;
  constexpr bool toSign = tightOverset::signedness;
  using adaptor = Adaptor<SignExtension, Forbid, ReinterpretSign>;
  return adaptor::template adapt<toWidth, toSign>(lhs).compute() <=>
         adaptor::template adapt<toWidth, toSign>(rhs).compute();
}

template <ExprType ET1, ExprType ET2>
constexpr bool operator==(ET1 const& lhs, ET2 const& rhs) {
  using tightOverset = TightOverset<ET1, ET2>;
  constexpr uint32_t toWidth = tightOverset::width;
  constexpr bool toSign = tightOverset::signedness;
  using adaptor = Adaptor<SignExtension, Forbid, ReinterpretSign>;
  return adaptor::template adapt<toWidth, toSign>(lhs).compute() ==
         adaptor::template adapt<toWidth, toSign>(rhs).compute();
}
/*
template<ExprType Shifted, ExprType Shift, bool rightShift>
class ShiftExpr {
public:
static constexpr uint32_t width = Shifted::width;
static constexpr bool signedness = false;
private:
using res_t = ap_repr<width, signedness>;
};*/

} // namespace apintext

#endif
