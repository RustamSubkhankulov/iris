#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <stack>

#include <graph/loops.hpp>
#include <graph/region.hpp>

namespace iris {
namespace loops {

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

  // Basic block -> loop
  std::unordered_map<const BasicBlock*, Loop*> blockToLoop;

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

    // Add header to block2loop map
    blockToLoop.insert(std::make_pair(loop->getHeader(), loop.get()));

    for (auto* latch : headerToLatches[loop->getHeader()]) {
      // Add latch to loop
      loop->addLatch(latch);

      // Add latch to block2loop map
      blockToLoop.insert(std::make_pair(latch, loop.get()));
    }
  }

  auto postOrder = region.getPO();

  // Populate loops for headers in reverse RPO
  for (auto* bb : postOrder) {

    auto res = headerToLoop.find(bb);
    if (res == headerToLoop.end()) {
      // Not a header
      continue;
    }

    auto* loop = res->second;
    if (loop->isReducible()) {
      // For reducible loops for all back edges run loop search
      for (auto* latch : loop->getLatches()) {
        loopSearch(latch, loop, blockToLoop);
      }
    }
  }

  for (auto& loop : m_loops) {
    // Collect exiting and exit blocks
    if (loop->isReducible()) {
      loop->collectExitEdges();
    }

    // Collect top-level loops
    if (loop->m_parent == nullptr) {
      m_rootLoop.addNestedLoop(loop.get());
    }
  }

  // Add remaining block to root loop
  collectRootLoopBasicBlocks(postOrder, blockToLoop);

  // Set loops depth recursively
  setLoopDepth();

  m_isExpired = false;
}

void LoopInfo::collectRootLoopBasicBlocks(
  const std::vector<const BasicBlock*>& postOrder,
  const std::unordered_map<const BasicBlock*, Loop*>& blockToLoop) {
  for (auto* bb : postOrder) {
    if (!blockToLoop.contains((bb))) {
      m_rootLoop.populate(bb);
    }
  }
}

void LoopInfo::setLoopDepth() {
  std::function<void(unsigned, Loop*)> setDepth = [&](unsigned depth,
                                                      Loop* loop) {
    loop->setDepth(depth);
    for (auto* nested : loop->getNestedLoops()) {
      setDepth(depth + 1U, nested);
    }
  };
  setDepth(0, &m_rootLoop);
}

std::unordered_set<const BasicBlock*> Loop::getContainedBlockRecursive() {
  std::unordered_set<const BasicBlock*> result;

  result.insert(m_header);
  result.insert(m_blocks.begin(), m_blocks.end());
  result.insert(m_latches.begin(), m_latches.end());

  for (auto* loop : m_nestedLoops) {
    auto nestedRes = loop->getContainedBlockRecursive();
    result.insert(nestedRes.begin(), nestedRes.end());
  }

  return result;
}

void Loop::collectExitEdges() {
  auto containedBlocks = getContainedBlockRecursive();

  for (auto* bb : containedBlocks) {
    for (auto* succ : {bb->getSucc(true), bb->getSucc(false)}) {
      if (succ == nullptr) {
        continue;
      }

      if (!containedBlocks.contains(succ)) {
        addExitEdge(bb, succ);
      }
    } // for succ
  } // for bb
}

void LoopInfo::loopSearch(
  const BasicBlock* latch, Loop* loop,
  std::unordered_map<const BasicBlock*, Loop*>& blockToLoop) {
  std::unordered_set<const BasicBlock*> green;

  green.insert(loop->getHeader());

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
        auto* nestedLoop = res->second;
        if (nestedLoop != loop && nestedLoop->getParent() == nullptr) {
          // if block is in the other loop (inner),
          // link outer loop and inner loop
          loop->addNestedLoop(nestedLoop);
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
  if (m_isExpired) {
    os << "[LoopInfo expired]" << std::endl;
    return;
  }

  os << "========== Loop Tree ==========" << std::endl;
  m_rootLoop.dump(os, 0);
  os << "================================" << std::endl;
}

void Loop::dump(std::ostream& os, unsigned indent) const {
  std::string pad(indent, ' ');
  if (isRoot()) {
    os << pad << "[Root Loop]" << std::endl;
  } else {
    os << pad << "Loop Header: " << m_header->getID() << std::endl;
  }

  os << pad << "  Depth: " << m_depth << " | Reducible: " << std::boolalpha
     << m_isReducible << std::endl;

  // Latches
  if (!m_latches.empty()) {
    os << pad << "  Latches: ";
    for (auto* latch : m_latches) {
      os << latch->getID() << " ";
    }
    os << std::endl;
  }

  // Blocks
  if (m_isReducible) {
    os << pad << "  Blocks (" << m_blocks.size() << "): ";
    for (auto* bb : m_blocks) {
      os << bb->getID() << " ";
    }
    os << std::endl;

    // Exit edges
    if (!m_exits.empty()) {
      os << pad << "  Exits:" << std::endl;
      for (const auto& e : m_exits) {
        os << pad << "    " << e.src()->getID() << " -> " << e.dst()->getID()
           << std::endl;
      }
    }

    // Nested loops
    if (!m_nestedLoops.empty()) {
      os << pad << "  Nested Loops:" << std::endl;
      for (auto* nested : m_nestedLoops) {
        nested->dump(os, indent + 2);
      }
    }
  } // if (m_isReducible)
}

} // namespace loops
} // namespace iris
