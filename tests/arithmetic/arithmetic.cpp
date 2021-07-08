#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SimpleArithmetic

#include <cstdint>
#include <type_traits>

#include <boost/test/unit_test.hpp>

#include "apintext.hpp"

namespace utf = boost::unit_test;

using namespace std;

using namespace apintext;

struct Dummy {};

template <uint32_t targetWidth, Signedness targetSignedness>
constexpr typename std::conditional<static_cast<bool>(targetSignedness),
                                    int32_t, uint32_t>::type
getValueForFormat(uint32_t val) {
  constexpr uint32_t mask = (1 << targetWidth) - 1;
  uint32_t masked = val & mask;
  if constexpr (targetSignedness == signedness::Unsigned) {
    return masked;
  } else {
    constexpr uint32_t s_mask = 1 << (targetWidth - 1);
    int32_t pos_bits = masked & (mask >> 1);
    return (masked & s_mask) ? pos_bits - s_mask : pos_bits;
  }
}

template <uint32_t a, uint32_t b, uint32_t res> constexpr void check_sum_8() {
  constexpr Value<8, signedness::Unsigned> x { a };
  constexpr Value<8, signedness::Unsigned> y { b };
  constexpr unsigned _ExtInt(8) expected_res { res };
  constexpr Value<8, signedness::Unsigned> sum = a + b;
  static_assert(sum.compute() == expected_res, "Unexpected sum result");
}

BOOST_AUTO_TEST_CASE(StaticSum) {
  check_sum_8<0, 0, 0>();
  check_sum_8<(1 << 8) - 2, 1, (1 << 8) - 1>();
  check_sum_8<(1 << 8) - 2, 3, 1>();
}

template <uint32_t wA, uint32_t wB, Signedness sA, Signedness sB>
bool sum_extensive_check() {
  constexpr uint32_t aUpperBound = 1 << wA;
  constexpr uint32_t bUpperBound = 1 << wB;
  using arithprop = ArithmeticProp<wA, wB, sA, sB>;
  using sumtype = ap_repr<arithprop::sumWidth, arithprop::sumSignedness>;
  for (uint32_t aRepr = 0; aRepr < aUpperBound; ++aRepr) {
    auto aVal = getValueForFormat<wA, sA>(aRepr);
    Value<wA, sA> aValue { aVal };
    for (uint32_t bRepr = 0; bRepr < bUpperBound; ++bRepr) {
      auto bVal = getValueForFormat<wB, sB>(bRepr);
      auto sum = aVal + bVal;
      Value<wB, sB> bValue { bVal };
      sumtype extintsum { static_cast<sumtype>(sum) };
      auto exprsum = aValue + bValue;
      if (extintsum != exprsum.compute()) {
        cerr << "Error in " << wA << " (" << (sA == signedness::Signed)
             << ") + " << wB << " (" << (sB == signedness::Signed)
             << ") product\n";
        cerr << "Op A - iter: " << aRepr << ", value: " << aVal << "\n";
        cerr << "Op B - iter: " << bRepr << ", value: " << bVal << "\n";
        return false;
      }
    }
  }
  return true;
}

BOOST_AUTO_TEST_CASE(DynamicSums) {
  bool res;
  res = sum_extensive_check<1, 1, signedness::Unsigned, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<1, 1, signedness::Signed, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<1, 1, signedness::Signed, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<1, 4, signedness::Unsigned, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<1, 4, signedness::Signed, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<1, 4, signedness::Signed, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<1, 4, signedness::Unsigned, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<5, 5, signedness::Unsigned, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<5, 5, signedness::Signed, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<5, 5, signedness::Signed, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<5, 5, signedness::Unsigned, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<5, 8, signedness::Unsigned, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<5, 8, signedness::Signed, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<5, 8, signedness::Signed, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<5, 8, signedness::Unsigned, signedness::Signed>();
  BOOST_REQUIRE(res);
}

template <uint32_t a, uint32_t b, uint32_t res> constexpr void check_sub_8() {
  constexpr Value<8, signedness::Unsigned> x { a };
  constexpr Value<8, signedness::Unsigned> y { b };
  constexpr unsigned _ExtInt(8) expected_res { res };
  constexpr Value<8, signedness::Unsigned> sum = a - b;
  static_assert(sum.compute() == expected_res, "Unexpected sum result");
}

BOOST_AUTO_TEST_CASE(StaticSub) {
  check_sub_8<0, 0, 0>();
  check_sub_8<0, 1, ((1 << 8) - 1)>();
  check_sub_8<157, 13, 144>();
  check_sub_8<42, 7, 35>();
}

template <uint32_t wA, uint32_t wB, Signedness sA, Signedness sB>
bool sub_extensive_check() {
  constexpr uint32_t aUpperBound = 1 << wA;
  constexpr uint32_t bUpperBound = 1 << wB;
  using arithprop = ArithmeticProp<wA, wB, sA, sB>;
  using sumtype = ap_repr<arithprop::sumWidth, arithprop::sumSignedness>;
  for (uint32_t aRepr = 0; aRepr < aUpperBound; ++aRepr) {
    auto aVal = getValueForFormat<wA, sA>(aRepr);
    Value<wA, sA> aValue { aVal };
    for (uint32_t bRepr = 0; bRepr < bUpperBound; ++bRepr) {
      auto bVal = getValueForFormat<wB, sB>(bRepr);
      auto sum = aVal - bVal;
      Value<wB, sB> bValue { bVal };
      sumtype extintsum { static_cast<sumtype>(sum) };
      auto exprsum = aValue - bValue;
      if (extintsum != exprsum.compute()) {
        cerr << "Error in " << wA << " (" << sA << ") - " << wB << " (" << sB
             << ") product\n";
        cerr << "Op A - iter: " << aRepr << ", value: " << aVal << "\n";
        cerr << "Op B - iter: " << bRepr << ", value: " << bVal << "\n";
        return false;
      }
    }
  }
  return true;
}

BOOST_AUTO_TEST_CASE(DynamicSubs) {
  bool res;
  res = sub_extensive_check<1, 1, signedness::Unsigned, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<1, 1, signedness::Signed, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<1, 1, signedness::Signed, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<1, 4, signedness::Unsigned, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<1, 4, signedness::Signed, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<1, 4, signedness::Signed, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<1, 4, signedness::Unsigned, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<5, 5, signedness::Unsigned, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<5, 5, signedness::Signed, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<5, 5, signedness::Signed, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<5, 5, signedness::Unsigned, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<5, 8, signedness::Unsigned, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<5, 8, signedness::Signed, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<5, 8, signedness::Signed, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<5, 8, signedness::Unsigned, signedness::Signed>();
  BOOST_REQUIRE(res);
}

template <uint32_t a, uint32_t b, uint32_t res> constexpr void check_prod_8() {
  constexpr Value<8, signedness::Unsigned> x { a };
  constexpr Value<8, signedness::Unsigned> y { b };
  constexpr unsigned _ExtInt(8) expected_res { res };
  constexpr Value<8, signedness::Unsigned> prod = a * b;
  static_assert(prod.compute() == expected_res, "Unexpected product result");
}

BOOST_AUTO_TEST_CASE(StaticProducts) {
  // Product in range
  check_prod_8<14, 2, 28>();
  // Product overflow
  check_prod_8<225, 2, 194>();
  // Operand overflow
  check_prod_8<257, 259, 3>();

  using ap = ArithmeticProp<1, 4, signedness::Signed, signedness::Unsigned>;
  static_assert(ap::prodWidth == 5, "Error in product width computation");
  static_assert(ap::prodSignedness == signedness::Signed,
                "Error in product sign computation");
}

template <uint32_t wA, uint32_t wB, Signedness sA, Signedness sB>
bool prod_extensive_check() {
  constexpr uint32_t aUpperBound = 1 << wA;
  constexpr uint32_t bUpperBound = 1 << wB;
  using arithprop = ArithmeticProp<wA, wB, sA, sB>;
  using prodtype = ap_repr<arithprop::prodWidth, arithprop::prodSignedness>;
  for (uint32_t aRepr = 0; aRepr < aUpperBound; ++aRepr) {
    auto aVal = getValueForFormat<wA, sA>(aRepr);
    Value<wA, sA> aValue { aVal };
    for (uint32_t bRepr = 0; bRepr < bUpperBound; ++bRepr) {
      auto bVal = getValueForFormat<wB, sB>(bRepr);
      auto prod = aVal * bVal;
      Value<wB, sB> bValue { bVal };
      prodtype extintprod { static_cast<prodtype>(prod) };
      auto exprprod = aValue * bValue;
      if (extintprod != exprprod.compute()) {
        cerr << "Error in " << wA << " (" << sA << ") x " << wB << " (" << sB
             << ") product\n";
        cerr << "Op A - iter: " << aRepr << ", value: " << aVal << "\n";
        cerr << "Op B - iter: " << bRepr << ", value: " << bVal << "\n";
        return false;
      }
    }
  }
  return true;
}

BOOST_AUTO_TEST_CASE(DynamicProducts) {
  bool res;
  res = prod_extensive_check<1, 1, signedness::Unsigned, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<1, 1, signedness::Signed, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<1, 1, signedness::Signed, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<1, 4, signedness::Unsigned, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<1, 4, signedness::Signed, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<1, 4, signedness::Signed, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<1, 4, signedness::Unsigned, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<5, 5, signedness::Unsigned, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<5, 5, signedness::Signed, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<5, 5, signedness::Signed, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<5, 5, signedness::Unsigned, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<5, 8, signedness::Unsigned, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<5, 8, signedness::Signed, signedness::Unsigned>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<5, 8, signedness::Signed, signedness::Signed>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<5, 8, signedness::Unsigned, signedness::Signed>();
  BOOST_REQUIRE(res);
}

BOOST_AUTO_TEST_CASE(StaticGetBit) {
  constexpr Value<4, signedness::Unsigned> in { char { 6 + 64 } };
  constexpr auto b0 = getBit<0>(in);
  constexpr auto b1 = getBit<1>(in);
  constexpr auto b2 = getBit<2>(in);
  constexpr auto b3 = getBit<3>(in);

  constexpr auto ib0 = getAs<int>(b0);
  constexpr auto ib1 = getAs<int>(b1);
  constexpr auto ib2 = getAs<int>(b2);
  constexpr auto ib3 = getAs<int>(b3);

  static_assert(ib0 == 0, "Error on bit 0");
  static_assert(ib1 == 1, "Error on bit 1");
  static_assert(ib2 == 1, "Error on bit 2");
  static_assert(ib3 == 0, "Error on bit 0");
}

BOOST_AUTO_TEST_CASE(StaticComparison) {
  constexpr Value<7, signedness::Unsigned> a { 14 };
  constexpr Value<7, signedness::Signed> b { -7 };
  static_assert(a > b);
  static_assert(!(b > a));
  static_assert(!(a > a));

  static_assert(a >= b);
  static_assert(!(b >= a));
  static_assert(a >= a);

  static_assert(!(a <= b));
  static_assert(b <= a);
  static_assert(a <= a);

  static_assert(!(a < b));
  static_assert(b < a);
  static_assert(!(a < a));

  static_assert(a != b);
  static_assert(!(a != a));
  static_assert(!(a == b));
  static_assert((a == a));
}

BOOST_AUTO_TEST_CASE(StaticShifts) {
  constexpr Value<4, signedness::Unsigned> twelve { 12 };
  constexpr Value<1, signedness::Unsigned> one { 1 };
  constexpr auto six = getAs<int>(twelve >> one);
  constexpr auto eight = getAs<int>(twelve << one);
  static_assert(six == 6);
  static_assert(eight == 8);

  constexpr Value<4, signedness::Signed> m_four { 0b1100 };
  constexpr auto m_2 = getAs<int>(m_four >> one);
  static_assert(m_2 == -2);

  constexpr auto m_8 = getAs<int>(m_four << one);
  static_assert(m_8 == -8);
}

template <uint32_t dividendWidth, Signedness dividendSignedness,
          uint32_t divisorWidth, Signedness divisorSignedness>
bool testAllMod() {
  uint32_t ofDividend = 1 << dividendWidth;
  uint32_t ofDivisor = 1 << divisorWidth;
  for (uint32_t dividendRepr = 0; dividendRepr < ofDividend; ++dividendRepr) {
    int32_t dividendIntVal =
        getValueForFormat<dividendWidth, dividendSignedness>(dividendRepr);
    Value<dividendWidth, dividendSignedness> dividend { dividendRepr };
    for (uint32_t divisorRepr = 1; divisorRepr < ofDivisor; ++divisorRepr) {
      int32_t divisorIntVal =
          getValueForFormat<divisorWidth, divisorSignedness>(divisorRepr);
      Value<divisorWidth, divisorSignedness> divisor { divisorRepr };
      auto modulus = dividend % divisor;
      auto modulusVal = getAs<int>(modulus);
      if (modulusVal != (dividendIntVal % divisorIntVal)) {
        cerr << dividendIntVal << " (" << dividendRepr << ") % "
             << divisorIntVal << " (" << divisorRepr << ") :  got "
             << modulusVal << " while expecting "
             << (dividendIntVal % divisorIntVal) << "\n";
        return false;
      }
    }
  }
  return true;
}

BOOST_AUTO_TEST_CASE(DynamicModulo) {
  auto testMod9_4_u_u = testAllMod<9, signedness::Unsigned, 4, signedness::Unsigned>();
  auto testMod9_4_u_s = testAllMod<9, signedness::Unsigned, 4, signedness::Signed>();
  auto testMod9_4_s_s = testAllMod<9, signedness::Signed, 4, signedness::Signed>();
  auto testMod9_4_s_u = testAllMod<9, signedness::Signed, 4, signedness::Unsigned>();
  BOOST_REQUIRE(testMod9_4_u_u);
  BOOST_REQUIRE(testMod9_4_u_s);
  BOOST_REQUIRE(testMod9_4_s_s);
  BOOST_REQUIRE(testMod9_4_s_u);

  auto testMod5_4_u_u = testAllMod<5, signedness::Unsigned, 4, signedness::Unsigned>();
  auto testMod5_4_u_s = testAllMod<5, signedness::Unsigned, 4, signedness::Signed>();
  auto testMod5_4_s_s = testAllMod<5, signedness::Signed, 4, signedness::Signed>();
  auto testMod5_4_s_u = testAllMod<5, signedness::Signed, 4, signedness::Unsigned>();
  BOOST_REQUIRE(testMod5_4_u_u);
  BOOST_REQUIRE(testMod5_4_u_s);
  BOOST_REQUIRE(testMod5_4_s_s);
  BOOST_REQUIRE(testMod5_4_s_u);

  auto testMod5_9_u_u = testAllMod<5, signedness::Unsigned, 9, signedness::Unsigned>();
  auto testMod5_9_u_s = testAllMod<5, signedness::Unsigned, 9, signedness::Signed>();
  auto testMod5_9_s_s = testAllMod<5, signedness::Signed, 9, signedness::Signed>();
  auto testMod5_9_s_u = testAllMod<5, signedness::Signed, 9, signedness::Unsigned>();
  BOOST_REQUIRE(testMod5_9_u_u);
  BOOST_REQUIRE(testMod5_9_u_s);
  BOOST_REQUIRE(testMod5_9_s_s);
  BOOST_REQUIRE(testMod5_9_s_u);

  auto testMod5_6_u_u = testAllMod<5, signedness::Unsigned, 6, signedness::Unsigned>();
  auto testMod5_6_u_s = testAllMod<5, signedness::Unsigned, 6, signedness::Signed>();
  auto testMod5_6_s_s = testAllMod<5, signedness::Signed, 6, signedness::Signed>();
  auto testMod5_6_s_u = testAllMod<5, signedness::Signed, 6, signedness::Unsigned>();
  BOOST_REQUIRE(testMod5_6_u_u);
  BOOST_REQUIRE(testMod5_6_u_s);
  BOOST_REQUIRE(testMod5_6_s_s);
  BOOST_REQUIRE(testMod5_6_s_u);

  auto testMod8_8_u_u = testAllMod<8, signedness::Unsigned, 8, signedness::Unsigned>();
  auto testMod8_8_u_s = testAllMod<8, signedness::Unsigned, 8, signedness::Signed>();
  auto testMod8_8_s_s = testAllMod<8, signedness::Signed, 8, signedness::Signed>();
  auto testMod8_8_s_u = testAllMod<8, signedness::Signed, 8, signedness::Unsigned>();
  BOOST_REQUIRE(testMod8_8_u_u);
  BOOST_REQUIRE(testMod8_8_u_s);
  BOOST_REQUIRE(testMod8_8_s_s);
  BOOST_REQUIRE(testMod8_8_s_u);
}

template <uint32_t dividendWidth, Signedness dividendSignedness,
          uint32_t divisorWidth, Signedness divisorSignedness>
bool testAllDiv() {
  uint32_t ofDividend = 1 << dividendWidth;
  uint32_t ofDivisor = 1 << divisorWidth;
  for (uint32_t dividendRepr = 0; dividendRepr < ofDividend; ++dividendRepr) {
    int32_t dividendIntVal =
        getValueForFormat<dividendWidth, dividendSignedness>(dividendRepr);
    Value<dividendWidth, dividendSignedness> dividend { dividendRepr };
    for (uint32_t divisorRepr = 1; divisorRepr < ofDivisor; ++divisorRepr) {
      int32_t divisorIntVal =
          getValueForFormat<divisorWidth, divisorSignedness>(divisorRepr);
      Value<divisorWidth, divisorSignedness> divisor { divisorRepr };
      auto div = dividend / divisor;
      auto divVal = getAs<int>(div);
      if (divVal != (dividendIntVal / divisorIntVal)) {
        cerr << dividendWidth << ", " << dividendSignedness << ", "
             << divisorWidth << ", " << divisorSignedness << "\n";
        cerr << decltype(div)::width << ", " << decltype(div)::signedness
             << "\n";
        cerr << dividendIntVal << " (" << dividendRepr << ") / "
             << divisorIntVal << " (" << divisorRepr << ") :  got " << divVal
             << " while expecting " << (dividendIntVal / divisorIntVal) << endl;
        return false;
      }
    }
  }
  return true;
}

BOOST_AUTO_TEST_CASE(DynamicDivision) {
  auto testDiv9_4_u_u = testAllDiv<9, signedness::Unsigned, 4, signedness::Unsigned>();
  auto testDiv9_4_u_s = testAllDiv<9, signedness::Unsigned, 4, signedness::Signed>();
  auto testDiv9_4_s_s = testAllDiv<9, signedness::Signed, 4, signedness::Signed>();
  auto testDiv9_4_s_u = testAllDiv<9, signedness::Signed, 4, signedness::Unsigned>();
  BOOST_REQUIRE(testDiv9_4_u_u);
  BOOST_REQUIRE(testDiv9_4_u_s);
  BOOST_REQUIRE(testDiv9_4_s_s);
  BOOST_REQUIRE(testDiv9_4_s_u);

  auto testDiv5_4_u_u = testAllDiv<5, signedness::Unsigned, 4, signedness::Unsigned>();
  auto testDiv5_4_u_s = testAllDiv<5, signedness::Unsigned, 4, signedness::Signed>();
  auto testDiv5_4_s_s = testAllDiv<5, signedness::Signed, 4, signedness::Signed>();
  auto testDiv5_4_s_u = testAllDiv<5, signedness::Signed, 4, signedness::Unsigned>();
  BOOST_REQUIRE(testDiv5_4_u_u);
  BOOST_REQUIRE(testDiv5_4_u_s);
  BOOST_REQUIRE(testDiv5_4_s_s);
  BOOST_REQUIRE(testDiv5_4_s_u);

  auto testDiv5_9_u_u = testAllDiv<5, signedness::Unsigned, 9, signedness::Unsigned>();
  auto testDiv5_9_u_s = testAllDiv<5, signedness::Unsigned, 9, signedness::Signed>();
  auto testDiv5_9_s_s = testAllDiv<5, signedness::Signed, 9, signedness::Signed>();
  auto testDiv5_9_s_u = testAllDiv<5, signedness::Signed, 9, signedness::Unsigned>();
  BOOST_REQUIRE(testDiv5_9_u_u);
  BOOST_REQUIRE(testDiv5_9_u_s);
  BOOST_REQUIRE(testDiv5_9_s_s);
  BOOST_REQUIRE(testDiv5_9_s_u);

  auto testDiv5_6_u_u = testAllDiv<5, signedness::Unsigned, 6, signedness::Unsigned>();
  auto testDiv5_6_u_s = testAllDiv<5, signedness::Unsigned, 6, signedness::Signed>();
  auto testDiv5_6_s_s = testAllDiv<5, signedness::Signed, 6, signedness::Signed>();
  auto testDiv5_6_s_u = testAllDiv<5, signedness::Signed, 6, signedness::Unsigned>();
  BOOST_REQUIRE(testDiv5_6_u_u);
  BOOST_REQUIRE(testDiv5_6_u_s);
  BOOST_REQUIRE(testDiv5_6_s_s);
  BOOST_REQUIRE(testDiv5_6_s_u);

  auto testDiv8_8_u_u = testAllDiv<8, signedness::Unsigned, 8, signedness::Unsigned>();
  auto testDiv8_8_u_s = testAllDiv<8, signedness::Unsigned, 8, signedness::Signed>();
  auto testDiv8_8_s_s = testAllDiv<8, signedness::Signed, 8, signedness::Signed>();
  auto testDiv8_8_s_u = testAllDiv<8, signedness::Signed, 8, signedness::Unsigned>();
  BOOST_REQUIRE(testDiv8_8_u_u);
  BOOST_REQUIRE(testDiv8_8_u_s);
  BOOST_REQUIRE(testDiv8_8_s_s);
  BOOST_REQUIRE(testDiv8_8_s_u);
}
