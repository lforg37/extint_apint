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

template <uint32_t targetWidth, bool targetSignedness>
typename std::conditional<targetSignedness, int32_t, uint32_t>::type
getValueForFormat(uint32_t val) {
  constexpr uint32_t mask = (1 << targetWidth) - 1;
  uint32_t masked = val & mask;
  if constexpr (!targetSignedness) {
    return masked;
  } else {
    constexpr uint32_t s_mask = 1 << (targetWidth - 1);
    int32_t pos_bits = masked & (mask >> 1);
    return (masked & s_mask) ? pos_bits - s_mask : pos_bits;
  }
}

template <uint32_t a, uint32_t b, uint32_t res> constexpr void check_sum_8() {
  constexpr Value<8, false> x { a };
  constexpr Value<8, false> y { b };
  constexpr unsigned _ExtInt(8) expected_res { res };
  constexpr Value<8, false> sum = a + b;
  static_assert(sum.compute() == expected_res, "Unexpected sum result");
}

BOOST_AUTO_TEST_CASE(StaticSum) {
  check_sum_8<0, 0, 0>();
  check_sum_8<(1 << 8) - 2, 1, (1 << 8) - 1>();
  check_sum_8<(1 << 8) - 2, 3, 1>();
}

template <uint32_t wA, uint32_t wB, bool sA, bool sB>
bool sum_extensive_check() {
  constexpr uint32_t aUpperBound = 1 << wA;
  constexpr uint32_t bUpperBound = 1 << wB;
  using arithprop = ArithmeticProp<wA, wB, sA, sB>;
  using sumtype = ap_repr<arithprop::sumWidth, arithprop::sumSigned>;
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
        cerr << "Error in " << wA << " (" << sA << ") + " << wB << " (" << sB
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
  res = sum_extensive_check<1, 1, false, false>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<1, 1, true, false>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<1, 1, true, true>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<1, 4, false, false>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<1, 4, true, false>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<1, 4, true, true>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<1, 4, false, true>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<5, 5, false, false>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<5, 5, true, false>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<5, 5, true, true>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<5, 5, false, true>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<5, 8, false, false>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<5, 8, true, false>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<5, 8, true, true>();
  BOOST_REQUIRE(res);
  res = sum_extensive_check<5, 8, false, true>();
  BOOST_REQUIRE(res);
}

template <uint32_t a, uint32_t b, uint32_t res> constexpr void check_sub_8() {
  constexpr Value<8, false> x { a };
  constexpr Value<8, false> y { b };
  constexpr unsigned _ExtInt(8) expected_res { res };
  constexpr Value<8, false> sum = a - b;
  static_assert(sum.compute() == expected_res, "Unexpected sum result");
}

BOOST_AUTO_TEST_CASE(StaticSub) {
  check_sub_8<0, 0, 0>();
  check_sub_8<0, 1, ((1 << 8) - 1)>();
  check_sub_8<157, 13, 144>();
  check_sub_8<42, 7, 35>();
}

template <uint32_t wA, uint32_t wB, bool sA, bool sB>
bool sub_extensive_check() {
  constexpr uint32_t aUpperBound = 1 << wA;
  constexpr uint32_t bUpperBound = 1 << wB;
  using arithprop = ArithmeticProp<wA, wB, sA, sB>;
  using sumtype = ap_repr<arithprop::sumWidth, arithprop::sumSigned>;
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
  res = sub_extensive_check<1, 1, false, false>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<1, 1, true, false>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<1, 1, true, true>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<1, 4, false, false>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<1, 4, true, false>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<1, 4, true, true>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<1, 4, false, true>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<5, 5, false, false>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<5, 5, true, false>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<5, 5, true, true>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<5, 5, false, true>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<5, 8, false, false>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<5, 8, true, false>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<5, 8, true, true>();
  BOOST_REQUIRE(res);
  res = sub_extensive_check<5, 8, false, true>();
  BOOST_REQUIRE(res);
}

template <uint32_t a, uint32_t b, uint32_t res> constexpr void check_prod_8() {
  constexpr Value<8, false> x { a };
  constexpr Value<8, false> y { b };
  constexpr unsigned _ExtInt(8) expected_res { res };
  constexpr Value<8, false> prod = a * b;
  static_assert(prod.compute() == expected_res, "Unexpected product result");
}

BOOST_AUTO_TEST_CASE(StaticProducts) {
  // Product in range
  check_prod_8<14, 2, 28>();
  // Product overflow
  check_prod_8<225, 2, 194>();
  // Operand overflow
  check_prod_8<257, 259, 3>();

  using ap = ArithmeticProp<1, 4, true, false>;
  static_assert(ap::prodWidth == 5, "Error in product width computation");
  static_assert(ap::prodSigned == true, "Error in product sign computation");
}

template <uint32_t wA, uint32_t wB, bool sA, bool sB>
bool prod_extensive_check() {
  constexpr uint32_t aUpperBound = 1 << wA;
  constexpr uint32_t bUpperBound = 1 << wB;
  using arithprop = ArithmeticProp<wA, wB, sA, sB>;
  using prodtype = ap_repr<arithprop::prodWidth, arithprop::prodSigned>;
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
  res = prod_extensive_check<1, 1, false, false>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<1, 1, true, false>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<1, 1, true, true>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<1, 4, false, false>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<1, 4, true, false>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<1, 4, true, true>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<1, 4, false, true>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<5, 5, false, false>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<5, 5, true, false>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<5, 5, true, true>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<5, 5, false, true>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<5, 8, false, false>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<5, 8, true, false>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<5, 8, true, true>();
  BOOST_REQUIRE(res);
  res = prod_extensive_check<5, 8, false, true>();
  BOOST_REQUIRE(res);
}

BOOST_AUTO_TEST_CASE(StaticGetBit) {
  constexpr Value<4, false> in{char{6 + 64}};
  constexpr auto b0 = getBit<0>(in);
  constexpr auto b1 = getBit<1>(in);
  constexpr auto b2 = getBit<2>(in);
  constexpr auto b3 = getBit<3>(in);

  static_assert(getAs<int>(b0) == 0, "Error on bit 0");
  static_assert(getAs<int>(b1) == 1, "Error on bit 1");
  static_assert(getAs<int>(b2) == 1, "Error on bit 2");
  static_assert(getAs<int>(b3) == 0, "Error on bit 0");
}
