#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "apintext.hpp"

using namespace std;

namespace utf = boost::unit_test;

using namespace apintext;

BOOST_AUTO_TEST_CASE(Extension) {
  constexpr Value<1, signedness::Unsigned> us_one { ap_repr<1, signedness::Unsigned> { 1 } };
  constexpr Value<1, signedness::Signed> s_m_one { ap_repr<1, signedness::Signed> { -1 } };
  constexpr int us_z_ext = getAs<int, ZeroExtension>(us_one);
  static_assert(us_z_ext == 1, "Error with unsigned zero extension");
  constexpr int s_z_ext = getAs<int, ZeroExtension>(s_m_one);
  static_assert(s_z_ext == 1, "Error with signed zero extension");
  constexpr int us_s_ext = getAs<int, SignExtension>(us_one);
  static_assert(us_z_ext == 1, "Error with unsigned sign extension");
  constexpr int s_s_ext = getAs<int, SignExtension>(s_m_one);
  static_assert(s_s_ext == -1, "Error with signed sign extension");
}

BOOST_AUTO_TEST_CASE(toInt) {
    constexpr Value<1, signedness::Signed> m_one {1};
    static_assert(static_cast<int>(m_one) == -1);
    static_assert(static_cast<uint32_t>(m_one) == (uint32_t{0} - 1));
}

BOOST_AUTO_TEST_CASE(testUDL) {
    constexpr auto a = 0b10110101101011_apv;
    static_assert(decltype(a)::width == 14);
    static_assert(getAs<int>(a) == 11627);

    constexpr auto b = 0_apv;
    static_assert(decltype(b)::width == 1);
    static_assert(getAs<int>(b) == 0);

    constexpr auto c = 1_apv;
    static_assert(decltype(c)::width == 1);
    static_assert(getAs<int>(c) == 1);

    constexpr auto d = 17_apv;
    static_assert(decltype(d)::width == 5);
    static_assert(getAs<int>(d) == 17);

    constexpr auto e = 0b00000001101011_apv;
    static_assert(decltype(e)::width == 7);
    static_assert(getAs<int>(e) == 107);

    constexpr auto f = 00123_apv;
    static_assert(decltype(f)::width == 7);
    static_assert(getAs<int>(f) == 83);

    constexpr auto g = 0x666_apv;
    static_assert(decltype(g)::width == 11);
    static_assert(getAs<int>(g) == 1638);

}
