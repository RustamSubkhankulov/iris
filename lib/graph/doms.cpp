#include <graph/region.hpp>

namespace iris {

void Region::runDFSearchFrom(BasicBlock* basicBlock,
                             std::unordered_set<BasicBlock*>& visited,
                             std::vector<BasicBlock*>& order) const {
  if (visited.find(basicBlock) != visited.end()) {
    // This BB is already marked visited
    return;
  }

  if (auto* succT = basicBlock->getSucc(true)) {
    runDFSearchFrom(succT, visited, order);
  }

  if (auto* succF = basicBlock->getSucc(false)) {
    runDFSearchFrom(succF, visited, order);
  }

  order.push_back(basicBlock);
}

std::vector<BasicBlock*> Region::getDFS() const {
  if (m_startBB == nullptr) {
    throw IrisException("Cannot run DFS with no start basic block specified!");
  }

  std::unordered_set<BasicBlock*> visited;
  std::vector<BasicBlock*> order;

  runDFSearchFrom(m_startBB, visited, order);
  return order;
}

std::vector<BasicBlock*> Region::getRPO() const {
  auto res = getDFS();
  std::reverse(res.begin(), res.end());
  return res;
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
      for (auto* pred : bbPtr->getPreds()) {
        if (m_domInfo.idom.count(pred)) {
          iDomCandidate = pred;
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