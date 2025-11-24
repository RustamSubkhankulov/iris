#ifndef INCLUDE_OPT_LIB_DCE_HPP
#define INCLUDE_OPT_LIB_DCE_HPP

#include <opt/generic/pattern.hpp>

namespace iris::opt::common {

class DCEPass final : public PatternPass {
public:
  DCEPass();
};

} // namespace iris::opt::common

#endif // INCLUDE_OPT_LIB_DCE_HPP
