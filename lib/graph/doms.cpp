#include <graph/doms.hpp>
#include <graph/region.hpp>

namespace iris {
namespace doms {

void DomInfo::runDFSFrom(const BasicBlock* basicBlock,
                         std::unordered_set<const BasicBlock*>& visited,
                         std::vector<const BasicBlock*>& order) {
  if (visited.count(basicBlock)) {
    // This BB is already marked visited
    return;
  }

  visited.insert(basicBlock);
  order.push_back(basicBlock);

  if (auto* succT = basicBlock->getSucc(true)) {
    runDFSFrom(succT, visited, order);
  }

  if (auto* succF = basicBlock->getSucc(false)) {
    runDFSFrom(succF, visited, order);
  }
}

std::vector<const BasicBlock*> DomInfo::getDFS(const Region& region) {
  auto startBB = region.getStartBasicBlock();
  if (startBB == nullptr) {
    throw IrisException("Cannot run DFS with no start basic block specified!");
  }

  std::unordered_set<const BasicBlock*> visited;
  std::vector<const BasicBlock*> order;

  runDFSFrom(startBB, visited, order);
  return order;
}

void DomInfo::runPOFrom(const BasicBlock* basicBlock,
                        std::unordered_set<const BasicBlock*>& visited,
                        std::vector<const BasicBlock*>& order) {
  if (visited.count(basicBlock)) {
    // This BB is already marked visited
    return;
  }

  visited.insert(basicBlock);

  if (auto* succT = basicBlock->getSucc(true)) {
    runPOFrom(succT, visited, order);
  }

  if (auto* succF = basicBlock->getSucc(false)) {
    runPOFrom(succF, visited, order);
  }

  order.push_back(basicBlock);
}

std::vector<const BasicBlock*> DomInfo::getPO(const Region& region) {
  auto startBB = region.getStartBasicBlock();
  if (startBB == nullptr) {
    throw IrisException("Cannot run RPO with no start basic block specified!");
  }

  std::unordered_set<const BasicBlock*> visited;
  std::vector<const BasicBlock*> order;

  runPOFrom(startBB, visited, order);
  return order;
}

std::vector<const BasicBlock*> DomInfo::getRPO(const Region& region) {
  auto order = getPO(region);
  std::reverse(order.begin(), order.end());

  return order;
}

void DomInfo::analyze(const Region& region) {
  if (m_isExpired == false) {
    return;
  }

  auto startBB = region.getStartBasicBlock();
  if (startBB == nullptr) {
    throw IrisException(
      "Cannot collect dom info with no start basic block specified!");
  }

  m_idom.clear();
  m_dominated.clear();

  auto rpo = getRPO(region);
  m_idom[startBB] = startBB;

  bool changed = false;
  do {
    // For every basic block in RPO order ...
    for (auto* bbPtr : rpo) {
      // ... besides the starting one
      if (bbPtr == startBB) {
        continue;
      }

      const BasicBlock* iDomCandidate = nullptr;

      // Find predecessor, which has idom defined
      for (auto* predPtr : bbPtr->getPreds()) {
        if (m_idom.count(predPtr)) {
          iDomCandidate = predPtr;
          break;
        }
      }

      // If no such predecessor, skip
      if (iDomCandidate == nullptr) {
        continue;
      }

      for (auto* predPtr : bbPtr->getPreds()) {
        // For every other predecessor, which has its idom defined
        if (predPtr == iDomCandidate || m_idom.count(predPtr) == 0) {
          continue;
        }

        iDomCandidate = getLCAImmDominator(predPtr, iDomCandidate, m_idom, rpo);
      }

      auto& idom = m_idom[bbPtr];

      if (idom != iDomCandidate) {
        // IDom changed, continue iterating
        idom = iDomCandidate;
        changed = true;
      } else {
        changed = false;
      }
    } // for (auto* bbPtr : rpo)
  } while (changed == true);

  buildDominatedLists();
  m_isExpired = false;
}

const BasicBlock* DomInfo::getIDom(const BasicBlock* basicBlock) const {
  if (m_isExpired == true) {
    throw IrisException("Dom info is expired!");
  }

  auto iter = m_idom.find(const_cast<BasicBlock*>(basicBlock));
  if (iter == m_idom.end()) {
    return nullptr;
  }

  return iter->second;
}

const BasicBlock* DomInfo::getIDomByID(bb_id_t id, const Region& region) const {
  return getIDom(region.getBasicBlockByID(id));
}

std::vector<const BasicBlock*>
DomInfo::getDominatedBlocks(const BasicBlock* basicBlock) const {
  if (m_isExpired == true) {
    throw IrisException("Dom info is expired!");
  }

  auto iter = m_dominated.find(const_cast<BasicBlock*>(basicBlock));
  if (iter == m_dominated.end()) {
    return {};
  }

  return iter->second;
}

std::vector<const BasicBlock*>
DomInfo::getDominatedBlocksByID(bb_id_t id, const Region& region) const {
  return getDominatedBlocks(region.getBasicBlockByID(id));
}

std::vector<const BasicBlock*>
DomInfo::getDominatorsChain(const BasicBlock* basicBlock) const {
  if (m_isExpired == true) {
    throw IrisException("Dom info is expired!");
  }

  std::vector<const BasicBlock*> domChain;
  auto curBasicBlock = basicBlock;

  while (curBasicBlock) {
    domChain.push_back(curBasicBlock);

    auto idom = getIDom(curBasicBlock);
    if (idom == curBasicBlock) {
      break;
    }
    curBasicBlock = idom;
  }

  return domChain;
}

std::vector<const BasicBlock*>
DomInfo::getDominatorsChainByID(bb_id_t id, const Region& region) const {
  return getDominatorsChain(region.getBasicBlockByID(id));
}

const BasicBlock* DomInfo::getLCAImmDominator(
  const BasicBlock* b1, const BasicBlock* b2,
  const std::unordered_map<const BasicBlock*, const BasicBlock*>& idom,
  const std::vector<const BasicBlock*>& rpo) const {

  // Invert RPO
  std::unordered_map<const BasicBlock*, std::size_t> index;
  for (std::size_t idx = 0; idx < rpo.size(); ++idx) {
    index[rpo[idx]] = idx;
  }

  while (b1 != b2) {
    while (index.at(b1) > index.at(b2)) {
      b1 = idom.at(b1);
    }
    while (index.at(b2) > index.at(b1)) {
      b2 = idom.at(b2);
    }
  }

  return b1;
}

void DomInfo::buildDominatedLists() {
  for (auto& [basicBlock, idom] : m_idom) {
    if (basicBlock == idom) {
      continue;
    }
    m_dominated[idom].push_back(basicBlock);
  }
}

void DomInfo::dumpDomTreeRec(
  std::ostream& os, const BasicBlock* bb,
  const std::unordered_map<const BasicBlock*, std::vector<const BasicBlock*>>&
    dominated,
  unsigned indent) {
  std::string pad(indent, ' ');
  os << pad << "BB#" << bb->getID() << std::endl;

  auto it = dominated.find(bb);
  if (it == dominated.end()) {
    return;
  }

  for (auto* child : it->second) {
    dumpDomTreeRec(os, child, dominated, indent + 2);
  }
}

void DomInfo::dump(std::ostream& os, unsigned indent) const {
  std::string pad(indent, ' ');
  os << pad << "[Dominator Tree]" << std::endl;

  const BasicBlock* root = nullptr;
  for (const auto& [bb, idom] : m_idom) {
    if (bb == idom) {
      root = bb;
      break;
    }
  }

  if (root == nullptr) {
    os << pad << "  (No root dominator found)" << std::endl;
    return;
  }

  dumpDomTreeRec(os, root, m_dominated, indent + 2);
}

void DomInfo::dump(std::ostream& os) const {
  if (m_isExpired) {
    os << "[DomInfo expired]" << std::endl;
    return;
  }

  os << "========== Dom Tree ===========" << std::endl;
  dump(os, 0U);
  os << "================================" << std::endl;
}

} // namespace doms
} // namespace iris
