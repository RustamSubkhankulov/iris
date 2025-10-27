#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <stack>

#include <graph/loops.hpp>
#include <graph/region.hpp>

namespace iris {
namespace loops {

void Loop::dump(std::ostream& os, unsigned indent) const {
  (void)os;
  (void)indent;
}

void LoopInfo::analyze(const Region& region) {
  if (region.isDomInfoExpired()) {
    throw IrisException("LoopInfo::analyze() requires DomInfo unexpired!");
  }

  auto* startBB = region.getStartBasicBlock();
  if (startBB == nullptr) {
    throw IrisException(
      "Cannot collect loop info with no start basic block specified!");
  }

  m_rootLoop.dropNestedLoops();
  m_loops.clear();

  auto& domInfo = region.getDomInfo();

  std::unordered_map<const BasicBlock*, std::vector<const BasicBlock*>>
    idomChainMap;
  for (const auto& bb : region.getBasicBlocks()) {
    idomChainMap.insert(
      std::make_pair(bb.get(), domInfo.getDominatorsChain(bb.get())));
  }

  std::unordered_set<const BasicBlock*> gray;
  std::unordered_set<const BasicBlock*> black;

  std::vector<Edge> backEdges;
  std::unordered_map<const BasicBlock*, std::unique_ptr<Loop>> loops;

  collectBackEdges(startBB, gray, black, backEdges, loops, idomChainMap);

  // Collect detected loops
  std::transform(loops.begin(), loops.end(), std::back_inserter(m_loops),
                 [](auto& kv) { return std::move(kv.second); });

  // Header -> latch map
  std::unordered_map<const BasicBlock*, std::vector<const BasicBlock*>>
    headerToLatches;
  for (const auto& be : backEdges) {
    headerToLatches[be.dst()].push_back(be.src());
  }

  // Header -> loop map
  std::unordered_map<const BasicBlock*, Loop*> headerToLoop;
  for (const auto& loop : m_loops) {
    headerToLoop.insert(std::make_pair(loop->getHeader(), loop.get()));
  }

  // Basic block -> loop
  std::unordered_map<const BasicBlock*, Loop*> blockToLoop;

  auto postOrder = region.getPO();

  // Populate loops for headers in reverse RPO
  for (auto* bb : postOrder) {

    auto res = headerToLoop.find(bb);
    if (res == headerToLoop.end()) {
      // Not a header
      continue;
    }

    auto* loop = res->second;

    if (!loop->isReducible()) {
      // For irreducible loops - append all source for every back edge
      for (auto* latch : headerToLatches[bb]) {
        loop->addLatch(latch);
      }
    } else {
      // For reducible loops for all back edges run loop search
      for (auto* latch : headerToLatches[bb]) {
        loop->addLatch(latch);
        loopSearch(latch, loop, blockToLoop);
      }
    }
  }

  for (auto& loop : m_loops) {
    // Collect exiting and exit blocks
    for (auto* bb : loop->getBlocks()) {
      for (auto* succ : {bb->getSucc(true), bb->getSucc(false)}) {
        if (succ == nullptr) {
          continue;
        }
        if (!loop->contains(succ)) {
          loop->addExitEdge(bb, succ);
        }
      }
    }

    // Collect top-level loops
    if (loop->getParent() == nullptr) {
      m_rootLoop.addNestedLoop(loop.get());
    }
  }

  // Add remaining block to root loop
  for (auto* bb : postOrder) {
    if (!blockToLoop.contains((bb))) {
      m_rootLoop.populate(bb);
      blockToLoop.insert(std::make_pair(bb, &m_rootLoop));
    }
  }

  // Set loops depth recursively
  std::function<void(unsigned, Loop*)> setDepth = [&](unsigned depth,
                                                      Loop* loop) {
    loop->m_depth = depth;
    for (auto* nested : loop->getNestedLoops()) {
      setDepth(depth + 1U, nested);
    }
  };
  setDepth(0, &m_rootLoop);

  m_isExpired = false;
}

void LoopInfo::loopSearch(
  const BasicBlock* latch, Loop* loop,
  std::unordered_map<const BasicBlock*, Loop*>& blockToLoop) {

  std::unordered_set<const BasicBlock*> green;

  green.insert(loop->getHeader());
  green.insert(latch);

  std::stack<const BasicBlock*> st;
  for (auto* pred : latch->getPreds()) {
    st.push(pred);
  }

  while (!st.empty()) {
    auto* cur = st.top();
    st.pop();

    if (!green.contains(cur)) {
      // First time visited
      green.insert(cur);

      auto res = blockToLoop.find(cur);
      if (res == blockToLoop.end()) {
        // If block without loop, added to the current loop
        loop->populate(cur);
        blockToLoop.insert(std::make_pair(cur, loop));
      } else {
        // if block is in the other loop (inner),
        // link outer loop and inner loop
        auto* nestedLoop = res->second;
        if (nestedLoop->getParent() == nullptr) {
          loop->addNestedLoop(res->second);
        }
      }

      for (auto* pred : cur->getPreds()) {
        st.push(pred);
      }
    }
  }
}

void LoopInfo::collectBackEdges(
  const BasicBlock* bb, std::unordered_set<const BasicBlock*>& gray,
  std::unordered_set<const BasicBlock*>& black, std::vector<Edge>& backEdges,
  std::unordered_map<const BasicBlock*, std::unique_ptr<Loop>>& loops,
  const std::unordered_map<const BasicBlock*, std::vector<const BasicBlock*>>&
    idomChainMap) {
  // When a block is visited for the first time,
  // it is marked with gray and black markers
  gray.insert(bb);
  black.insert(bb); // visited

  for (auto* succ : {bb->getSucc(true), bb->getSucc(false)}) {
    if (!succ) {
      continue;
    }

    if (gray.contains(succ)) {
      // While doing DFS, if we encounter a block with gray marker,
      // then edge to this block is a back edge
      backEdges.emplace_back(bb, succ);

      // Create new Loop if it doesn't exist
      if (!loops.contains(succ)) {
        loops.insert(std::make_pair(succ, std::make_unique<Loop>(succ)));
      }

      const auto& idomChain = idomChainMap.at(bb);
      if (std::find(idomChain.begin(), idomChain.end(), succ) ==
          idomChain.end()) {
        // Header does not dominate latch
        loops[succ]->setReducibility(false);
      }
    } else if (!black.contains(succ)) {
      collectBackEdges(succ, gray, black, backEdges, loops, idomChainMap);
    }
  }

  // After visiting all block's successors,
  // it is unmarked with gray marker
  gray.erase(bb);
}

void LoopInfo::dump(std::ostream& os) const {
  os << "Loop Tree:" << std::endl;
  m_rootLoop.dump(os, 2U);
}

} // namespace loops
} // namespace iris
