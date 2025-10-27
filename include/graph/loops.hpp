#ifndef INCLUDE_GRAPH_LOOPS_HPP
#define INCLUDE_GRAPH_LOOPS_HPP

#include <unordered_map>
#include <unordered_set>

#include <exception.hpp>
#include <graph/basic_block.hpp>

namespace iris {

class Region;

namespace loops {

class Loop final {
public:
  explicit Loop(const BasicBlock* header)
    : m_header(header) {}

  const BasicBlock* getHeader() const noexcept {
    return m_header;
  }

  const std::unordered_set<const BasicBlock*>& getLatches() const noexcept {
    return m_latches;
  }

  const std::unordered_set<const BasicBlock*>& getBlocks() const noexcept {
    return m_blocks;
  }
  const std::vector<Edge>& getExits() const noexcept {
    return m_exits;
  }

  const std::vector<Loop*>& getNestedLoops() const noexcept {
    return m_nestedLoops;
  }

  Loop* getParent() const noexcept {
    return m_parent;
  }

  bool blocksContain(const BasicBlock* bb) const {
    return m_blocks.contains(bb);
  }

  bool latchesContain(const BasicBlock* bb) const {
    return m_latches.contains(bb);
  }

  unsigned getDepth() const noexcept {
    return m_depth;
  }

  bool isReducible() const noexcept {
    return m_isReducible;
  }

  bool isRoot() const noexcept {
    return (m_header == nullptr);
  }

  void dump(std::ostream& os, unsigned indent = 0) const;

  friend class LoopInfo;

private:
  void populate(const BasicBlock* bb) {
    // Check for uniqueness
    if (!m_blocks.contains(bb)) {
      m_blocks.insert(bb);
    }
  }

  void addNestedLoop(Loop* nestedLoop) {
    if (!nestedLoop) {
      throw IrisException("Empty nested loop!");
    }
    nestedLoop->m_parent = this;
    m_nestedLoops.push_back(nestedLoop);
  }

  void addLatch(const BasicBlock* latch) {
    m_latches.insert(latch);
  }

  void dropNestedLoops() {
    m_nestedLoops.clear();
  }

  void addExitEdge(const BasicBlock* src, const BasicBlock* dst) {
    m_exits.emplace_back(src, dst);
  }

  void setDepth(unsigned depth) {
    m_depth = depth;
  }

  void setReducibility(bool isReducible) {
    m_isReducible = isReducible;
  }

private:
  // Header bb pointer, equals nullptr for root loop
  const BasicBlock* m_header = nullptr;

  // One canonical latch (last processed tail)
  std::unordered_set<const BasicBlock*> m_latches;

  // Loop's basic blocks
  std::unordered_set<const BasicBlock*> m_blocks;

  // Exiting edges
  std::vector<Edge> m_exits;

  // Nested loops
  std::vector<Loop*> m_nestedLoops;

  // Parend loop, equals nullptr for root loop
  Loop* m_parent = nullptr;

  // Reducibility flag
  bool m_isReducible = true;

  // Loop's depth, equals 0 for root loop
  unsigned m_depth = 0;
};

class LoopInfo final {
public:
  void analyze(const Region& region);

  // Root loop
  const Loop& getRootLoop() const noexcept {
    return m_rootLoop;
  }

  // Loops immediate following the root loop in the hierarchy
  const std::vector<Loop*>& getTopLevelLoops() const {
    return m_rootLoop.getNestedLoops();
  }

  void expire() noexcept {
    m_isExpired = true;
  }

  bool isExpired() const noexcept {
    return m_isExpired;
  }

  void dump(std::ostream& os) const;

private:
  void setLoopDepth();

  void collectRootLoopBasicBlocks(const std::vector<const BasicBlock*>& postOrder,
                                  const std::unordered_map<const BasicBlock*, Loop*>& blockToLoop);

  static void collectBackEdges(
    const BasicBlock* bb, std::unordered_set<const BasicBlock*>& gray,
    std::unordered_set<const BasicBlock*>& black, std::vector<Edge>& backEdges,
    std::unordered_map<const BasicBlock*, std::unique_ptr<Loop>>& loops,
    const std::unordered_map<const BasicBlock*, std::vector<const BasicBlock*>>&
      idomChainMap);

  static void
  loopSearch(const BasicBlock* latch, Loop* loop,
             std::unordered_map<const BasicBlock*, Loop*>& blockToLoop);

  static void collectExitEdgesFrom(const BasicBlock* bb, Loop* loop);

private:
  // By default loop info is expired
  // (it must be prepared before querying)
  bool m_isExpired = true;

  // Root loop
  Loop m_rootLoop{nullptr};

  // Other loops
  std::vector<std::unique_ptr<Loop>> m_loops;
};

} // namespace loops
} // namespace iris

#endif // INCLUDE_GRAPH_LOOPS_HPP