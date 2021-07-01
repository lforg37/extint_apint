#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SimpleArithmetic

#include <cstdint>
#include <type_traits>

#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;

#include "apintext.hpp"

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

template <uint32_t a, uint32_t b, uint32_t res> constexpr void check_prod_8() {
  constexpr Value<8, false> x { a };
  constexpr Value<8, false> y { b };
  constexpr unsigned _ExtInt(8) expected_res { res };
  constexpr Value<8, false> prod = a * b;
  static_assert(prod.compute() == expected_res, "Unexpected product result");
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
