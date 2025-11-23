#include <opt/generic/pattern.hpp>

#include <exception.hpp>

#include <graph/basic_block.hpp>
#include <graph/region.hpp>
#include <ops/generic/operation.hpp>

namespace iris {
namespace opt {

void PatternRewriter::eraseOp(Operation& op) {
  auto* bb = op.getParentBasicBlock();
  if (bb == nullptr) {
    throw IrisException("Cannot erase operation without parent basic block!");
  }

  auto& regOps = bb->getOps();
  for (auto it = regOps.begin(); it != regOps.end(); ++it) {
    if (&*it == &op) {
      bb->eraseOp(it);
      return;
    }
  }

  auto& phiOps = bb->getPhiOps();
  for (auto it = phiOps.begin(); it != phiOps.end(); ++it) {
    if (&*it == &op) {
      bb->erasePhiOp(it);
      return;
    }
  }

  throw IrisException("Operation is not attached to its parent basic block!");
}

void PatternRewriter::replaceOpWith(Operation& op,
                                    std::unique_ptr<Operation> newOp) {
  if (!newOp) {
    throw IrisException("PatternRewriter::replaceOp() got empty newOp!");
  }

  auto* bb = op.getParentBasicBlock();
  if (bb == nullptr) {
    throw IrisException("Cannot replace operation without parent basic block!");
  }

  auto preservedID = op.getID();

  auto& regOps = bb->getOps();
  for (auto it = regOps.begin(); it != regOps.end(); ++it) {
    if (&*it == &op) {
      newOp->setID(preservedID);
      bb->replaceOpWith(it, std::move(newOp));
      return;
    }
  }

  auto& phiOps = bb->getPhiOps();
  for (auto it = phiOps.begin(); it != phiOps.end(); ++it) {
    if (&*it == &op) {
      auto phiOp = std::unique_ptr<ctrlflow::PhiOp>(
        static_cast<ctrlflow::PhiOp*>(newOp.release()));

      phiOp->setID(preservedID);
      bb->replacePhiOpWith(it, std::move(phiOp));
      return;
    }
  }

  throw IrisException("Operation is not attached to its parent basic block!");
}

bool PatternPass::run(Region& region) {
  if (m_patterns.empty()) {
    return false;
  }

  PatternRewriter rewriter(region);
  bool IRchanged = false;

  for (auto& bbPtr : region.getBasicBlocks()) {
    BasicBlock& bb = *bbPtr;

    // TODO: maybe use another policy of applying patterns to ops in region
    auto applyToList = [&](BasicBlock::OpList& opList) {
      bool BBchanged;
      do {
        BBchanged = false;

        for (auto it = opList.begin(); it != opList.end(); ++it) {
          Operation& op = *it;

          for (const auto& pattern : m_patterns) {
            if (pattern->matchAndRewrite(op, rewriter)) {
              IRchanged = BBchanged = true;
              // List was modified, restart from the beginning.
              break;
            }
          }

          if (BBchanged) {
            break;
          }
        }
      } while (BBchanged);
    };

    applyToList(bb.getOps());
    applyToList(bb.getPhiOps());
  }

  return IRchanged;
}

} // namespace opt
} // namespace iris
