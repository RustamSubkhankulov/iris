#ifndef INCLUDE_GRAPH_DOMS_HPP
#define INCLUDE_GRAPH_DOMS_HPP

#include <unordered_map>
#include <unordered_set>

#include <graph/basic_block.hpp>

namespace iris {

class Region;

namespace doms {

class DomInfo {
public:
  void analyze(const Region& region);

  static std::vector<const BasicBlock*> getDFS(const Region& region);
  static std::vector<const BasicBlock*> getRPO(const Region& region);

  const BasicBlock* getIDom(const BasicBlock* basicBlock) const;
  const BasicBlock* getIDomByID(bb_id_t id, const Region& region) const;

  std::vector<const BasicBlock*>
  getDominatedBlocks(const BasicBlock* basicBlock) const;

  std::vector<const BasicBlock*>
  getDominatedBlocksByID(bb_id_t id, const Region& region) const;

  std::vector<const BasicBlock*>
  getDominatorsChain(const BasicBlock* basicBlock) const;

  std::vector<const BasicBlock*>
  getDominatorsChainByID(bb_id_t id, const Region& region) const;

  void expire() noexcept {
    m_isExpired = true;
  }
  bool isExpired() const noexcept {
    return m_isExpired;
  }

private:
  void static runDFSFrom(const BasicBlock* basicBlock,
                         std::unordered_set<const BasicBlock*>& visited,
                         std::vector<const BasicBlock*>& order);

  void static runRPOFrom(const BasicBlock* basicBlock,
                         std::unordered_set<const BasicBlock*>& visited,
                         std::vector<const BasicBlock*>& order);

  const BasicBlock* getLCAImmDominator(
    const BasicBlock* b1, const BasicBlock* b2,
    const std::unordered_map<const BasicBlock*, const BasicBlock*>& idom,
    const std::vector<const BasicBlock*>& order) const;

  void buildDominatedLists();

private:
  // BB -> its IDOM
  std::unordered_map<const BasicBlock*, const BasicBlock*> m_idom;

  // BB -> BBs that are dominated by it
  std::unordered_map<const BasicBlock*, std::vector<const BasicBlock*>>
    m_dominated;

  // By default dominators info is expired
  // (it must be prepared before querying)
  bool m_isExpired = true;
};

} // namespace doms
} // namespace iris

#endif // INCLUDE_GRAPH_DOMS_HPP
