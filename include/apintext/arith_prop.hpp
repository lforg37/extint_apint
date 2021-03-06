#ifndef ARITH_PROP_HPP
#define ARITH_PROP_HPP

#include <cstdint>

namespace apintext {
template <uint32_t width1, uint32_t width2, bool signedness1, bool signedness2>
class ArithmeticProp {
 private:
  static constexpr uint32_t _generalProdWidth = width1 + width2;
  static constexpr uint32_t _max = (width1 > width2) ? width1 : width2;
  static constexpr uint32_t _min = (width1 > width2) ? width2 : width1;
  static constexpr bool _sameSignedness = (signedness1 == signedness2);
  static constexpr bool _oneIsOne = ((width1 == 1) || (width2 == 1));
  static constexpr bool _bothAreOne = ((width1 == 1) && (width2 == 1));
  static constexpr uint32_t _caseOneWidth =
      (_sameSignedness || _bothAreOne) ? _max : _max + 1;
  static constexpr bool _oneSigned = signedness1 or signedness2;

 public:
  static constexpr bool prodSigned =
      (_oneSigned and (!_bothAreOne || !_sameSignedness));
  static constexpr uint32_t prodWidth =
      (_oneIsOne) ? _caseOneWidth : _generalProdWidth;
  static constexpr bool sumSigned = _oneSigned;
  static constexpr uint32_t sumWidth = _max + 1;
  static constexpr uint32_t divWidth = (signedness2) ?  width1 + 1 : width1;
  static constexpr bool divSigned = _oneSigned;
  static constexpr uint32_t modWidth = (signedness1) ? _min + 1 : _min;
  static constexpr bool modSigned = signedness1;
};
} // namespace apextint

#endif
