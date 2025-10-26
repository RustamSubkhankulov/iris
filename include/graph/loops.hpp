#ifndef INCLUDE_GRAPH_LOOPS_HPP
#define INCLUDE_GRAPH_LOOPS_HPP

#include <graph/basic_block.hpp>

namespace iris {

class Region;

namespace loops {

class LoopInfo {
public:
  void analyze(const Region& region);

  void expire() noexcept {
    m_isExpired = true;
  }
  bool isExpired() const noexcept {
    return m_isExpired;
  }

private:
  // By default dominators info is expired
  // (it must be prepared before querying)
  bool m_isExpired = true;
};

} // namespace loops

} // namespace iris

#endif // INCLUDE_GRAPH_LOOPS_HPP