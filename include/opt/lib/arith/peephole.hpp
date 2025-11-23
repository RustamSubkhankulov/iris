#ifndef INCLUDE_OPT_LIB_ARITH_PEEPHOLE_HPP
#define INCLUDE_OPT_LIB_ARITH_PEEPHOLE_HPP

#include <opt/generic/pattern.hpp>

namespace iris {
namespace opt {
namespace arith {

class ArithPeepHolePass final : public PatternPass {
public:
  ArithPeepHolePass();
};

} // namespace arith
} // namespace opt
} // namespace iris

#endif // INCLUDE_OPT_LIB_ARITH_PEEPHOLE_HPP
