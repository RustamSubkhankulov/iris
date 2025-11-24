#ifndef INCLUDE_OPT_LIB_ARITH_PEEPHOLE_HPP
#define INCLUDE_OPT_LIB_ARITH_PEEPHOLE_HPP

#include <opt/generic/pattern.hpp>

namespace iris::opt::arith {

class ArithPeepHolePass final : public PatternPass {
public:
  ArithPeepHolePass();
};

} // namespace iris::opt::arith

#endif // INCLUDE_OPT_LIB_ARITH_PEEPHOLE_HPP
