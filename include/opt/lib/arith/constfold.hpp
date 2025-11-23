#ifndef INCLUDE_OPT_LIB_ARITH_CONSTFOLD_HPP
#define INCLUDE_OPT_LIB_ARITH_CONSTFOLD_HPP

#include <opt/generic/pattern.hpp>

namespace iris {
namespace opt {
namespace arith {

class ArithConstFoldPass final : public PatternPass {
public:
  ArithConstFoldPass();
};

} // namespace arith
} // namespace opt
} // namespace iris

#endif // INCLUDE_OPT_LIB_ARITH_CONSTFOLD_HPP
