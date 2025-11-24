#ifndef INCLUDE_OPT_LIB_ARITH_CONSTFOLD_HPP
#define INCLUDE_OPT_LIB_ARITH_CONSTFOLD_HPP

#include <opt/generic/pattern.hpp>

namespace iris::opt::arith {

class ArithConstFoldPass final : public PatternPass {
public:
  ArithConstFoldPass();
};

} // namespace iris::opt::arith

#endif // INCLUDE_OPT_LIB_ARITH_CONSTFOLD_HPP
