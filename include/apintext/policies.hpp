#ifndef APINTEXT_POLICIES_HPP
#define APINTEXT_POLICIES_HPP

#include "expression.hpp"

namespace apintext {

struct Truncation {
  template <uint32_t targetWidth, ExprType ET>
  static constexpr auto truncate(ET const& source) {
    static_assert(ET::width > targetWidth,
                  "Trying to truncate expression to a bigger target width.");
    return SliceExpr<targetWidth - 1, 0, ET> { source };
  }
};

struct ReinterpretSign {
  template <ExprType ET> static constexpr auto handleSign(ET const& source) {
    return ReinterpretSignExpr<!ET::signedness, ET>(source);
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

  template <ExprType ET> static constexpr auto handleSign(ET const& source) {
    static_assert(ET::width < 0, "Trying to perform forbidden sign conversion");
  }

  template <uint32_t targetWidth, ExprType ET>
  static constexpr auto extend(ET const& ) {
     static_assert(ET::width < 0, "Trying to perform forbiden width extension");
  }
};

} // namespace apintext

#endif
