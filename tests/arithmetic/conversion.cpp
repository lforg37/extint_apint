#include <boost/test/unit_test.hpp>

#include "apintext.hpp"

using namespace std;

namespace utf = boost::unit_test;

using namespace apintext;

BOOST_AUTO_TEST_CASE(Extension) {
  constexpr Value<1, false> us_one { ap_repr<1, false> { 1 } };
  constexpr Value<1, true> s_m_one { ap_repr<1, true> { -1 } };
  constexpr int us_z_ext = getAs<int, ZeroExtension>(us_one);
  static_assert(us_z_ext == 1, "Error with unsigned zero extension");
  constexpr int s_z_ext = getAs<int, ZeroExtension>(s_m_one);
  static_assert(s_z_ext == 1, "Error with signed zero extension");
  constexpr int us_s_ext = getAs<int, SignExtension>(us_one);
  static_assert(us_z_ext == 1, "Error with unsigned sign extension");
  constexpr int s_s_ext = getAs<int, SignExtension>(s_m_one);
  static_assert(s_s_ext == -1, "Error with signed sign extension");
}
