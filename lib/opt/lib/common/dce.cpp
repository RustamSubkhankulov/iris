#include <opt/lib/common/dce.hpp>

namespace iris {
namespace opt {
namespace common {

namespace {

class DCEPattern final : public Pattern {
public:
  bool matchAndRewrite(Operation& op, PatternRewriter& rewriter) const override {
    // We only remove side-effect-free, non-terminator operations that
    // produce a result and have no users.
    if (!op.hasResult() || op.hasUsers() || op.isTerminator() ||
        op.hasSideEffects()) {
      return false;
    }

    // We also cannot remove ParamOp, since function
    // signatures are considered fixed for now
    if (op.isa(GlobalOpcodes::PARAM)) {
      return false;
    }

    rewriter.eraseOp(op);
    return true;
  }
};

} // anonymous

DCEPass::DCEPass() {
  m_patterns.push_back(std::make_unique<DCEPattern>());
}

} // namespace common
} // namespace opt
} // namespace iris
