#include <graph/region.hpp>

namespace iris {

void Region::runDFSFrom(BasicBlock* basicBlock,
                        std::unordered_set<BasicBlock*>& visited,
                        std::vector<BasicBlock*>& order) const {
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

std::vector<BasicBlock*> Region::getDFS() const {
  if (m_startBB == nullptr) {
    throw IrisException("Cannot run DFS with no start basic block specified!");
  }

  std::unordered_set<BasicBlock*> visited;
  std::vector<BasicBlock*> order;

  runDFSFrom(m_startBB, visited, order);
  return order;
}

void Region::runRPOFrom(BasicBlock* basicBlock,
                        std::unordered_set<BasicBlock*>& visited,
                        std::vector<BasicBlock*>& order) const {
  if (visited.count(basicBlock)) {
    // This BB is already marked visited
    return;
  }

  visited.insert(basicBlock);

  if (auto* succT = basicBlock->getSucc(true)) {
    runRPOFrom(succT, visited, order);
  }

  if (auto* succF = basicBlock->getSucc(false)) {
    runRPOFrom(succF, visited, order);
  }

  order.push_back(basicBlock);
}

std::vector<BasicBlock*> Region::getRPO() const {
  if (m_startBB == nullptr) {
    throw IrisException("Cannot run RPO with no start basic block specified!");
  }

  std::unordered_set<BasicBlock*> visited;
  std::vector<BasicBlock*> order;

  runRPOFrom(m_startBB, visited, order);
  std::reverse(order.begin(), order.end());

  return order;
}

void Region::collectDomInfo() {
  if (m_startBB == nullptr) {
    throw IrisException("Cannot run DFS with no start basic block specified!");
  }

  if (m_domInfo.isExpired == false) {
    return;
  }

  m_domInfo.idom.clear();
  m_domInfo.dominated.clear();

  auto rpo = getRPO();
  m_domInfo.idom[m_startBB] = m_startBB;

  bool changed = false;
  do {
    // For every basic block in RPO order ...
    for (auto* bbPtr : rpo) {
      // ... besides the starting one
      if (bbPtr == m_startBB) {
        continue;
      }

      BasicBlock* iDomCandidate = nullptr;

      // Find predecessor, which has idom defined
      for (auto* predPtr : bbPtr->getPreds()) {
        if (m_domInfo.idom.count(predPtr)) {
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
        if (predPtr == iDomCandidate || m_domInfo.idom.count(predPtr) == 0) {
          continue;
        }

        iDomCandidate =
          getLCAImmDominator(predPtr, iDomCandidate, m_domInfo.idom, rpo);
      }

      auto& idom = m_domInfo.idom[bbPtr];

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
  m_domInfo.isExpired = false;
}

BasicBlock* Region::getIDom(const BasicBlock* basicBlock) const {
  if (m_domInfo.isExpired == true) {
    throw IrisException("Dom info is expired!");
  }

  auto iter = m_domInfo.idom.find(const_cast<BasicBlock*>(basicBlock));
  if (iter == m_domInfo.idom.end()) {
    return nullptr;
  }

  return iter->second;
}

BasicBlock* Region::getIDomByID(bb_id_t id) const {
  return getIDom(getBasicBlockByID(id));
}

std::vector<BasicBlock*>
Region::getDominatedBlocks(const BasicBlock* basicBlock) const {
  if (m_domInfo.isExpired == true) {
    throw IrisException("Dom info is expired!");
  }

  auto iter = m_domInfo.dominated.find(const_cast<BasicBlock*>(basicBlock));
  if (iter == m_domInfo.dominated.end()) {
    return {};
  }

  return iter->second;
}

std::vector<BasicBlock*> Region::getDominatedBlocksByID(bb_id_t id) const {
  return getDominatedBlocks(getBasicBlockByID(id));
}

std::vector<BasicBlock*>
Region::getDominatorsChain(const BasicBlock* basicBlock) const {
  if (m_domInfo.isExpired == true) {
    throw IrisException("Dom info is expired!");
  }

  std::vector<BasicBlock*> domChain;
  auto curBasicBlock = const_cast<BasicBlock*>(basicBlock);

  while (curBasicBlock) {
    domChain.push_back(curBasicBlock);
    curBasicBlock = getIDom(curBasicBlock);
  }

  return domChain;
}

std::vector<BasicBlock*> Region::getDominatorsChainByID(bb_id_t id) const {
  return getDominatorsChain(getBasicBlockByID(id));
}

BasicBlock* Region::getLCAImmDominator(
  BasicBlock* b1, BasicBlock* b2,
  const std::unordered_map<BasicBlock*, BasicBlock*>& idom,
  const std::vector<BasicBlock*>& rpo) const {

  // Invert RPO
  std::unordered_map<BasicBlock*, std::size_t> index;
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

void Region::buildDominatedLists() {
  for (auto& [basicBlock, idom] : m_domInfo.idom) {
    if (basicBlock == idom) {
      continue;
    }
    m_domInfo.dominated[idom].push_back(basicBlock);
  }
}

} // namespace iris