#ifndef INCLUDE_OPT_LIB_DCE_HPP
#define INCLUDE_OPT_LIB_DCE_HPP

#include <opt/generic/pattern.hpp>

namespace iris {
namespace opt {
namespace common {

class DCEPattern final : public Pattern {
public:
  bool matchAndRewrite(Operation& op, PatternRewriter& rewriter) const override;
};

class DCEPass final : public PatternPass {
public:
  DCEPass();
};

} // namespace common
} // namespace opt
} // namespace iris

#endif // INCLUDE_OPT_LIB_DCE_HPP
