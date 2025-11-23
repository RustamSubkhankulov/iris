#ifndef INCLUDE_OPT_LIB_DCE_HPP
#define INCLUDE_OPT_LIB_DCE_HPP

#include <opt/generic/pattern.hpp>

namespace iris {
namespace opt {
namespace common {

class DCEPass final : public PatternPass {
public:
  DCEPass();
};

} // namespace common
} // namespace opt
} // namespace iris

#endif // INCLUDE_OPT_LIB_DCE_HPP
