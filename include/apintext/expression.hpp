#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <concepts>
#include <cstdint>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#include "aliases.hpp"
#include "arith_prop.hpp"
#include "static_math.hpp"

namespace apintext {

/// An expression is a type which has width and signedness members
/// and a compute() const method that returns an integer of the
/// corresponding width and signedness
template <typename T>
concept ExprType = requires(T const& val) {
  { val.compute() } -> std::same_as<ap_repr<T::width, T::signedness>>;
};

/// Get the result type of the compute method for a given expression type
template <ExprType ET> using res_t = ap_repr<ET::width, ET::signedness>;

/// Compute the required width and signedness to be able to store all values
/// from all expressions
template <ExprType E1, ExprType E2> struct TightOverset {
 private:
  static constexpr bool widestIsE1 = E1::width == E2::width;
  static constexpr std::uint32_t maxWidth =
      (E1::width > E2::width) ? E1::width : E2::width;
  static constexpr bool sameSignedness = E1::signedness == E2::signedness;
  static constexpr bool oneIsSigned = (E1::signedness == signedness::Signed) ||
                                      (E2::signedness == signedness::Signed);

 public:
  static constexpr Signedness signedness = static_cast<Signedness>(oneIsSigned);
  static constexpr std::uint32_t width =
      (sameSignedness) ? maxWidth : maxWidth + 1;
};

//****************** Adaptor **********************************//

template <typename ExtensionPolicy, typename TruncationPolicy,
          typename WrongSignPolicy>
struct Adaptor {

  /// Create the expression tree to adapt the source expression to the target
  /// width and signedness, according to policies.
  template <std::uint32_t targetWidth, Signedness targetSignedness, ExprType ET>
  static constexpr auto adapt(ET const& source) {
    constexpr std::uint32_t sourceWidth = ET::width;
    constexpr Signedness sourceSignedness = ET::signedness;
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
template <std::uint32_t w, Signedness s> class ConstantExpr {
 private:
  using val_t = ap_repr<w, s>;

 public:
  static constexpr std::uint32_t width = w;
  static constexpr Signedness signedness = s;

  constexpr ConstantExpr(val_t const& src_repr)
      : value { src_repr } {}
  constexpr val_t compute() const { return value; };

 private:
  const val_t value;
};

template <std::integral T> constexpr std::uint32_t getWidth() {
  return std::numeric_limits<std::make_unsigned_t<T>>::digits;
}

template <std::integral T>
constexpr ConstantExpr<getWidth<T>(),
                       static_cast<Signedness>(std::is_signed<T>::value)>
toExpr(T const& value) {
  return { value };
}

template <std::uint32_t w>
constexpr ConstantExpr<w, signedness::Unsigned> toExpr(unsigned _ExtInt(w)
                                                           const& value) {
  return { value };
}

template <std::uint32_t w>
constexpr ConstantExpr<w, signedness::Signed> toExpr(signed _ExtInt(w)
                                                         const& value) {
  return { value };
}

template <Signedness targetSignedness, ExprType SourceType>
class ReinterpretSignExpr {
 public:
  static_assert(targetSignedness != SourceType::signedness,
                "Attempt to insert useless ReinterpretSignExpr");
  static constexpr std::uint32_t width = SourceType::width;
  static constexpr Signedness signedness = targetSignedness;

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

template <std::uint32_t targetWidth, ExprType SourceType> class ZExtExpr {
 public:
  static_assert(targetWidth > SourceType::width,
                "Attempt to perform zero extension with target width smaller "
                "than source width.");
  static constexpr std::uint32_t width = targetWidth;
  static constexpr Signedness signedness = SourceType::signedness;

 private:
  SourceType const source;
  using res_t = ap_repr<width, signedness>;

 public:
  constexpr ZExtExpr(SourceType const& src)
      : source { src } {}
  constexpr res_t compute() const {
    return static_cast<res_t>(
        static_cast<ap_repr<SourceType::width, signedness::Unsigned>>(
            source.compute()));
  };
};

template <std::uint32_t targetWidth, ExprType ET>
constexpr auto zeroExtendToWidth(ET const& source) {
  return ZExtExpr<targetWidth, ET> { source };
}

template <std::uint32_t targetWidth, ExprType SourceType> class SignExtExpr {
 public:
  static_assert(targetWidth > SourceType::width,
                "Attempt to perform sign extension with target width smaller "
                "than source width.");
  static constexpr std::uint32_t width = targetWidth;
  static constexpr Signedness signedness = SourceType::signedness;

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

template <std::uint32_t targetWidth, ExprType ET>
constexpr auto signExtendToWidth(ET const& source) {
  return SignExtExpr<targetWidth, ET>(source);
}

//**************** Operation on bit vector ********************************//

template <std::uint32_t highBit, std::uint32_t lowBit, ExprType SourceType>
class SliceExpr {
 public:
  static_assert(highBit >= lowBit,
                "Slicing high index should be greater than low index");
  static_assert(SourceType::width > highBit,
                "Trying to slice out of input bounds");
  static constexpr std::uint32_t width = highBit - lowBit + 1;
  static constexpr Signedness signedness = signedness::Unsigned;

 private:
  SourceType const source;
  using res_t = ap_repr<width, signedness>;

 public:
  constexpr SliceExpr(SourceType const& sourceExpr)
      : source { sourceExpr } {}
  constexpr res_t compute() const {
    return static_cast<res_t>(
        static_cast<ap_repr<highBit + 1, signedness::Unsigned>>(
            source.compute()) >>
        lowBit);
  }
};

/**
 * @brief Slice the input
 *
 * @tparam highBit input msb to be included in the slice
 * @tparam lowbit input lsb to be included in the slice
 * @tparam ET source expression type
 * @param source
 * @return constexpr auto A SliceExpr representing the slice
 */
template <std::uint32_t highBit, std::uint32_t lowBit, ExprType ET>
constexpr auto slice(ET const& source) {
  return SliceExpr<highBit, lowBit, ET> { source };
}

template <std::uint32_t bitIdx, ExprType ET> class GetBitExpr {
 public:
  static_assert(bitIdx < ET::width,
                "Trying to access bit outside of input range");
  static constexpr std::uint32_t width = 1;
  static constexpr Signedness signedness = signedness::Unsigned;

 private:
  using res_t = ap_repr<width, signedness>;
  using intermediate_t = ap_repr<bitIdx + 1, signedness::Unsigned>;
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

/**
 * @brief Get one bit of the source expression
 *
 * @tparam idx index of the bit to get
 * @tparam ET source expression type
 * @param src source expression
 * @return a GetBitExpr that represents the output bit
 */
template <std::uint32_t idx, ExprType ET> constexpr auto getBit(ET const& src) {
  return GetBitExpr<idx, ET> { src };
}

template <ExprType... ET> class ConcatenateExpr {
 private:
  using storage_t = std::tuple<ET const...>;
  static constexpr std::size_t nbTypes = sizeof...(ET);

  template <ExprType First, ExprType... Remainder>
  static constexpr Signedness seqSignedness = First::signedness;

  template <ExprType... ETypes>
  static constexpr std::uint32_t seqWidth = (ETypes::width + ...);

 public:
  static constexpr std::uint32_t width = seqWidth<ET...>;
  static constexpr Signedness signedness = seqSignedness<ET...>;

 private:
  using res_t = ap_repr<width, signedness>;
  storage_t storage;

  template <ExprType Head, ExprType... Remainder>
  constexpr ap_repr<seqWidth<Head, Remainder...>, signedness::Unsigned>
  getSubConcat() const {
    using res_t = ap_repr<seqWidth<Head, Remainder...>, signedness::Unsigned>;
    if constexpr (sizeof...(Remainder) == 0) {
      return static_cast<res_t>(std::get<nbTypes - 1>(storage).compute());
    } else {
      constexpr auto toShift = seqWidth<Remainder...>;
      constexpr auto toGet = nbTypes - 1 - sizeof...(Remainder);
      auto highBits = static_cast<res_t>(std::get<toGet>(storage).compute())
                      << toShift;
      auto lowBits = static_cast<res_t>(getSubConcat<Remainder...>());
      return highBits | lowBits;
    }
  }

 public:
  constexpr ConcatenateExpr(ET const&... in)
      : storage { in... } {}
  constexpr res_t compute() const {
    return static_cast<res_t>(getSubConcat<ET...>());
  }
};

/**
 * @brief Concatenate all the input, from left to right
 *
 * @tparam ET source types
 * @param subexpr sub expression to concatenate
 * @return ConcatenateExpr representing the concatenation
 */
template <ExprType... ET>
constexpr ConcatenateExpr<ET...> concatenate(ET const&... subexpr) {
  return { subexpr... };
}

template <ExprType ET1, ExprType ET2, typename Operation>
class BitwiseLogicExpr {
 public:
  static constexpr std::uint32_t width = ET1::width;
  static constexpr Signedness signedness = signedness::Unsigned;

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
        "Trying to perform bitwise AND on operands of different widths.");
  }
  template <ExprType ET1, ExprType ET2>
  static constexpr ap_repr<ET1::width, signedness::Unsigned>
  compute(ET1 const& left, ET2 const& right) {
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
        "Trying to perform bitwise OR on operands of different widths.");
  }
  template <ExprType ET1, ExprType ET2>
  static constexpr ap_repr<ET1::width, signedness::Unsigned>
  compute(ET1 const& left, ET2 const& right) {
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
        "Trying to perform bitwise XOR on operands of different widths.");
  }
  template <ExprType ET1, ExprType ET2>
  static constexpr ap_repr<ET1::width, signedness::Unsigned>
  compute(ET1 const& left, ET2 const& right) {
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
  static constexpr std::uint32_t width = ET::width;
  static constexpr Signedness signedness = signedness::Unsigned;

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
  static constexpr std::uint32_t width = 1;
  static constexpr Signedness signedness = signedness::Unsigned;

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
  static constexpr ap_repr<1, signedness::Unsigned> compute(ET const& src) {
    constexpr res_t<ET> zero { 0 };
    return { src.compute() != zero };
  }
};

struct XORReduction {
 private:
  template <size_t width>
  static constexpr auto
  performXor(ap_repr<width, signedness::Unsigned> const& in) {
    if constexpr (width == 1) {
      return in;
    } else {
      constexpr auto log = log2<width>();
      constexpr auto pow = std::uint32_t { 1 } << log;
      auto low = static_cast<ap_repr<(pow / 2), signedness::Unsigned>>(in);
      auto high = static_cast<ap_repr<width - (pow / 2), signedness::Unsigned>>(
          in >> (pow / 2));
      if constexpr (pow == width) {
        ap_repr<pow / 2, signedness::Unsigned> reduced = high ^ low;
        return performXor<pow / 2>(reduced);
      } else {
        return performXor<width - (pow / 2)>(high) ^ performXor<pow / 2>(low);
      }
    }
  }

 public:
  template <ExprType ET>
  static constexpr ap_repr<1, signedness::Unsigned> compute(ET const& src) {
    return performXor<ET::width>(
        static_cast<ap_repr<ET::width, ET::signedness>>(src.compute()));
  }
};

struct ANDReduction {
  template <ExprType ET>
  static constexpr ap_repr<1, signedness::Unsigned> compute(ET const& src) {
    constexpr res_t<ET> zero { 0 };
    constexpr res_t<ET> fullOne = ~zero;
    return { src.compute() == fullOne };
  }
};

struct NANDReduction {
  template <ExprType ET>
  static constexpr ap_repr<1, signedness::Unsigned> compute(ET const& src) {
    constexpr res_t<ET> zero { 0 };
    constexpr res_t<ET> fullOne = ~zero;
    return { src.compute() != fullOne };
  }
};

struct NORReduction {
  template <ExprType ET>
  static constexpr ap_repr<1, signedness::Unsigned> compute(ET const& src) {
    constexpr res_t<ET> zero { 0 };
    return { src.compute() == zero };
  }
};

struct XNORReduction {
  template <ExprType ET>
  static constexpr ap_repr<1, signedness::Unsigned> compute(ET const& src) {
    return ~XORReduction::compute(src);
  }
};

template <ExprType ET> using ORReductionExpr = ReductionExpr<ET, ORReduction>;

template <ExprType ET> using XORReductionExpr = ReductionExpr<ET, XORReduction>;

template <ExprType ET> using NORReductionExpr = ReductionExpr<ET, NORReduction>;

template <ExprType ET>
using XNORReductionExpr = ReductionExpr<ET, XNORReduction>;

template <ExprType ET> using ANDReductionExpr = ReductionExpr<ET, ANDReduction>;

template <ExprType ET>
using NANDReductionExpr = ReductionExpr<ET, NANDReduction>;

/**
 * @brief Return the or reduction of the bit vector
 *
 * @tparam ET source expression type
 * @param source source expression
 * @return An expression representing the reduction
 */
template <ExprType ET> constexpr auto orReduce(ET const& source) {
  return ORReductionExpr<ET> { source };
}

/**
 * @brief Return the xor reduction of the bit vector
 *
 * @tparam ET source expression type
 * @param source source expression
 * @return An expression representing the reduction
 */
template <ExprType ET> constexpr auto xorReduce(ET const& source) {
  return XORReductionExpr<ET> { source };
}

/**
 * @brief Return the xnor reduction of the bit vector
 *
 * @tparam ET source expression type
 * @param source source expression
 * @return An expression representing the reduction
 */
template <ExprType ET> constexpr auto xnorReduce(ET const& source) {
  return XNORReductionExpr<ET> { source };
}

/**
 * @brief Return the nor reduction of the bit vector
 *
 * @tparam ET source expression type
 * @param source source expression
 * @return An expression representing the reduction
 */
template <ExprType ET> constexpr auto norReduce(ET const& source) {
  return NORReductionExpr<ET> { source };
}

/**
 * @brief Return the and reduction of the bit vector
 *
 * @tparam ET source expression type
 * @param source source expression
 * @return An expression representing the reduction
 */
template <ExprType ET> constexpr auto andReduce(ET const& source) {
  return ANDReductionExpr<ET> { source };
}

/**
 * @brief Return the nand reduction of the bit vector
 *
 * @tparam ET source expression type
 * @param source source expression
 * @return An expression representing the reduction
 */
template <ExprType ET> constexpr auto nandReduce(ET const& source) {
  return NANDReductionExpr<ET> { source };
}

//************* Policies *********************************************//

/**
 * @brief Truncate and discard extra high weight bits.
 */
struct Truncation {
  template <std::uint32_t targetWidth, ExprType ET>
  static constexpr auto truncate(ET const& source) {
    static_assert(ET::width > targetWidth,
                  "Trying to truncate expression to a bigger target width.");
    return SliceExpr<targetWidth - 1, 0, ET> { source };
  }
};

/**
 * @brief Change signedness by reinterpreting the value
 */
struct ReinterpretSign {
  template <Signedness targetSignedness, ExprType ET>
  static constexpr auto setSignedness(ET const& source) {
    return ReinterpretSignExpr<targetSignedness, ET>(source);
  }
};

/**
 * @brief Left pad numbers with zeros
 */
struct ZeroExtension {
  template <std::uint32_t targetWidth, ExprType ET>
  static constexpr auto extend(ET const& source) {
    return zeroExtendToWidth<targetWidth>(source);
  }
};

/**
 * @brief Left pad number with original sign bit (or zero for unsigned numbers)
 */
struct SignExtension {
  template <std::uint32_t targetWidth, ExprType ET>
  static constexpr auto extend(ET const& source) {
    return signExtendToWidth<targetWidth>(source);
  }
};

/**
 * @brief Used to forbid sign reinterpretation, padding or truncation.
 */
struct Forbid {
  template <std::uint32_t targetWidth, ExprType ET>
  static constexpr auto truncate(ET const& source) {
    static_assert(ET::width > targetWidth,
                  "Trying to truncate expression to a bigger target width.");
    static_assert(ET::width < 0, "Trying to perform forbidden truncation");
  }

  template <bool, ExprType ET>
  static constexpr auto setSignedness(ET const& source) {
    static_assert(ET::width < 0, "Trying to perform forbidden sign conversion");
  }

  template <std::uint32_t targetWidth, ExprType ET>
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
  static constexpr std::uint32_t width = prop::prodWidth;
  static constexpr Signedness signedness = prop::prodSignedness;
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
  static constexpr std::uint32_t width = prop::divWidth;
  static constexpr Signedness signedness = prop::divSignedness;
  using res_t = ap_repr<width, signedness>;
  ET1 const leftOp;
  ET2 const rightOp;

 public:
  constexpr ExprDiv(ET1 const& val1, ET2 const& val2)
      : leftOp { val1 }
      , rightOp { val2 } {}

  constexpr res_t compute() const {
    using tightOverset = TightOverset<ET1, ET2>;
    constexpr bool bothSigned = (ET1::signedness == signedness::Signed) &&
                                (ET2::signedness == signedness::Signed);
    constexpr std::uint32_t toWidth =
        (bothSigned && (tightOverset::width == ET1::width))
            ? tightOverset::width + 1
            : tightOverset::width;
    constexpr Signedness toSign = tightOverset::signedness;
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
  static constexpr std::uint32_t width = prop::modWidth;
  static constexpr Signedness signedness = prop::modSignedness;
  using res_t = ap_repr<width, signedness>;
  ET1 const leftOp;
  ET2 const rightOp;

 public:
  constexpr ExprMod(ET1 const& val1, ET2 const& val2)
      : leftOp { val1 }
      , rightOp { val2 } {}

  constexpr res_t compute() const {
    using tightOverset = TightOverset<ET1, ET2>;
    constexpr bool bothSigned = (ET1::signedness == signedness::Signed) &&
                                (ET2::signedness == signedness::Signed);
    constexpr std::uint32_t toWidth =
        (bothSigned && (tightOverset::width == ET1::width))
            ? tightOverset::width + 1
            : tightOverset::width;
    constexpr Signedness toSign = tightOverset::signedness;

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
  static constexpr std::uint32_t width = prop::sumWidth;
  static constexpr Signedness signedness = prop::sumSignedness;

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

template <ExprType ET1, ExprType ET2> class LeftShiftExpr {
 public:
  static constexpr std::uint32_t width = ET1::width;
  static constexpr Signedness signedness = ET1::signedness;

 private:
  using res_t = ap_repr<width, signedness>;
  ET1 const lhs;
  ET2 const rhs;

 public:
  constexpr LeftShiftExpr(ET1 const& lhsSrc, ET2 const& rhsSrc)
      : lhs { lhsSrc }
      , rhs { rhsSrc } {}

  constexpr res_t compute() const { return lhs.compute() << rhs.compute(); }
};

template <ExprType ET1, ExprType ET2>
constexpr LeftShiftExpr<ET1, ET2> operator<<(ET1 const& lhs, ET2 const& rhs) {
  return { lhs, rhs };
}

template <ExprType ET1, ExprType ET2> class RightShiftExpr {
 public:
  static constexpr std::uint32_t width = ET1::width;
  static constexpr Signedness signedness = ET1::signedness;

 private:
  using res_t = ap_repr<width, signedness>;
  ET1 const lhs;
  ET2 const rhs;

 public:
  constexpr RightShiftExpr(ET1 const& lhsSrc, ET2 const& rhsSrc)
      : lhs { lhsSrc }
      , rhs { rhsSrc } {}

  constexpr res_t compute() const { return lhs.compute() >> rhs.compute(); }
};

template <ExprType ET1, ExprType ET2>
constexpr RightShiftExpr<ET1, ET2> operator>>(ET1 const& lhs, ET2 const& rhs) {
  return { lhs, rhs };
}

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
  constexpr std::uint32_t toWidth = tightOverset::width;
  constexpr Signedness toSign = tightOverset::signedness;
  using adaptor = Adaptor<SignExtension, Forbid, ReinterpretSign>;
  return adaptor::template adapt<toWidth, toSign>(lhs).compute() <=>
         adaptor::template adapt<toWidth, toSign>(rhs).compute();
}

template <ExprType ET1, ExprType ET2>
constexpr bool operator==(ET1 const& lhs, ET2 const& rhs) {
  using tightOverset = TightOverset<ET1, ET2>;
  constexpr std::uint32_t toWidth = tightOverset::width;
  constexpr Signedness toSign = tightOverset::signedness;
  using adaptor = Adaptor<SignExtension, Forbid, ReinterpretSign>;
  return adaptor::template adapt<toWidth, toSign>(lhs).compute() ==
         adaptor::template adapt<toWidth, toSign>(rhs).compute();
}
} // namespace apintext

#endif
