#include <boost/test/unit_test.hpp>

#include "apintext.hpp"

using namespace std;

using namespace apintext;

BOOST_AUTO_TEST_CASE(StaticBitwiseOp) {
  constexpr Value<4, signedness::Unsigned> left { 0b0110 };
  constexpr Value<4, signedness::Unsigned> right { 0b1100 };

  using ExactWidthValue4 =
      Value<4, signedness::Unsigned, Forbid, Forbid, Forbid>;
  constexpr ExactWidthValue4 andVal = left & right;
  constexpr ExactWidthValue4 orVal = left | right;
  constexpr ExactWidthValue4 xorVal = left ^ right;

  constexpr auto andRes = getAs<uint32_t>(andVal);
  constexpr auto orRes = getAs<uint32_t>(orVal);
  constexpr auto xorRes = getAs<uint32_t>(xorVal);

  BOOST_REQUIRE_MESSAGE(andRes == 0b0100, "Error on static and");
  BOOST_REQUIRE_MESSAGE(orRes == 0b1110, "Error on static or");
  BOOST_REQUIRE_MESSAGE(xorRes == 0b1010, "Error on static xor");
}

bool testAnd(Value<4, signedness::Unsigned> left,
             Value<4, signedness::Unsigned> right, uint32_t res) {
  return res == getAs<uint32_t>(left & right);
}

bool testOr(Value<4, signedness::Unsigned> left,
            Value<4, signedness::Unsigned> right, uint32_t res) {
  return res == getAs<uint32_t>(left | right);
}

bool testXor(Value<4, signedness::Unsigned> left,
             Value<4, signedness::Unsigned> right, uint32_t res) {
  return res == getAs<uint32_t>(left ^ right);
}

BOOST_AUTO_TEST_CASE(DynamicBitwiseOp) {
  BOOST_REQUIRE_MESSAGE(testAnd({ 0b0110 }, { 0b1100 }, 0b0100),
                        "Error on static and");
  BOOST_REQUIRE_MESSAGE(testOr({ 0b0110 }, { 0b1100 }, 0b1110),
                        "Error on static or");
  BOOST_REQUIRE_MESSAGE(testXor({ 0b0110 }, { 0b1100 }, 0b1010),
                        "Error on static xor");
}

BOOST_AUTO_TEST_CASE(StaticBitInvert) {
  constexpr Value<4, signedness::Unsigned> in { 0b0101 };
  constexpr auto res = getAs<uint32_t>(~in);
  static_assert(res == 0b1010, "Error on sign inversion");
}

BOOST_AUTO_TEST_CASE(DynamicBitInvert) {
  Value<4, signedness::Unsigned> in { 0b0101 };
  auto res = getAs<uint32_t>(~in);
  BOOST_REQUIRE_EQUAL(res, 0b1010);
}

BOOST_AUTO_TEST_CASE(StaticReduction) {
  constexpr Value<3, signedness::Unsigned> zero { 0 }, mixed { 0b101 },
      fullOne { 0b111 };
  static_assert(getAs<int>(orReduce(zero)) == 0);
  static_assert(getAs<int>(orReduce(mixed)) == 1);
  static_assert(getAs<int>(orReduce(fullOne)) == 1);

  static_assert(getAs<int>(norReduce(zero)) == 1);
  static_assert(getAs<int>(norReduce(mixed)) == 0);
  static_assert(getAs<int>(norReduce(fullOne)) == 0);

  static_assert(getAs<int>(andReduce(zero)) == 0);
  static_assert(getAs<int>(andReduce(mixed)) == 0);
  static_assert(getAs<int>(andReduce(fullOne)) == 1);

  constexpr Value<5, signedness::Unsigned> a { 0 }, b { 1 }, c { 0b10000 },
      d { 0b01000 }, e { 0b11000 }, f { 0b11100 }, g { 0b00111 }, h { 0b10111 },
      i { 0b11111 };

    static_assert(getAs<int>(xorReduce(a)) == 0);
    static_assert(getAs<int>(xorReduce(b)) == 1);
    static_assert(getAs<int>(xorReduce(c)) == 1);
    static_assert(getAs<int>(xorReduce(d)) == 1);
    static_assert(getAs<int>(xorReduce(e)) == 0);
    static_assert(getAs<int>(xorReduce(f)) == 1);
    static_assert(getAs<int>(xorReduce(g)) == 1);
    static_assert(getAs<int>(xorReduce(h)) == 0);
    static_assert(getAs<int>(xorReduce(i)) == 1);
};

BOOST_AUTO_TEST_CASE(StaticConcatenate) {
  constexpr Value<16, signedness::Unsigned> h { 0xDEAD }, l { 0xBEEF };
  constexpr auto res = getAs<uint32_t>(concatenate(h, l));
  static_assert(res == 0xDEADBEEF);

  constexpr Value<8, signedness::Unsigned> a { 0xDE }, b { 0xAD }, c { 0xBE },
      d { 0xEF };
  constexpr auto res2 = getAs<uint32_t>(concatenate(a, b, c, d));
  static_assert(res2 == 0xDEADBEEF);
}
