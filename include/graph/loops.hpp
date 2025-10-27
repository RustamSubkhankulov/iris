#ifndef INCLUDE_GRAPH_LOOPS_HPP
#define INCLUDE_GRAPH_LOOPS_HPP

#include <unordered_map>

#include <graph/basic_block.hpp>

namespace iris {

class Region;

namespace loops {

class Loop {
public:
  explicit Loop(const BasicBlock* header)
    : m_header(header) {}

  const BasicBlock* getHeader() const noexcept {
    return m_header;
  }
  const BasicBlock* getLatch() const noexcept {
    return m_latch;
  }
  const BasicBlock* getPreheader() const noexcept {
    return m_preheader;
  }

  const std::vector<const BasicBlock*>& getBlocks() const noexcept {
    return m_blocks;
  }
  const std::vector<Edge>& getExits() const noexcept {
    return m_exits;
  }

  const std::vector<std::unique_ptr<Loop>>& getNestedLoops() const noexcept {
    return m_nestedLoops;
  }

  Loop* getParent() const noexcept {
    return m_parent;
  }

  void setParent(Loop* parent) {
    m_parent = parent;
    if (m_parent) {
      m_depth = m_parent->m_depth + 1U;
    }
  }

  void populate(BasicBlock* bb);
  void addNestedLoop(std::unique_ptr<Loop> nestedLoop);

  bool contains(const BasicBlock* bb) const;

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

private:
  // Header bb pointer, equals nullptr for root loop
  const BasicBlock* m_header = nullptr;

  // Present for reducible loops
  const BasicBlock* m_preheader = nullptr;

  // One canonical latch (last processed tail)
  const BasicBlock* m_latch = nullptr;

  // Loop's basic blocks
  std::vector<const BasicBlock*> m_blocks;

  // Exiting edges
  std::vector<Edge> m_exits;

  // Nested loops
  std::vector<std::unique_ptr<Loop>> m_nestedLoops;

  // Parend loop, equals nullptr for root loop
  Loop* m_parent = nullptr;

  // Reducibility flag
  bool m_isReducible = true;

  // Loop's depth, equals 0 for root loop
  unsigned m_depth = 0;
};

class LoopInfo {
public:
  void analyze(const Region& region);

  // Root loop
  const Loop* getRootLoop() const noexcept {
    return m_rootLoop.get();
  }

  // Loops imm following the root loop in the hierarchy
  const std::vector<std::unique_ptr<Loop>>& getTopLevelLoops() const noexcept {
    return m_rootLoop->getNestedLoops();
  }

  void expire() noexcept {
    m_isExpired = true;
  }
  bool isExpired() const noexcept {
    return m_isExpired;
  }

  void dump(std::ostream& os);

private:
  // By default loop info is expired
  // (it must be prepared before querying)
  bool m_isExpired = true;

  // Root loop
  std::unique_ptr<Loop> m_rootLoop;
};

} // namespace loops
} // namespace iris

#endif // INCLUDE_GRAPH_LOOPS_HPP