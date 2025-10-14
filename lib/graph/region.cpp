#include <graph/region.hpp>

namespace iris {

void Region::addStartBasicBlock(std::unique_ptr<BasicBlock> basicBlock) {
  if (m_startBB != nullptr) {
    throw IrisException("Start bb is already specified!");
  }
  m_startBB = basicBlock.get();
  addBasicBlock(std::move(basicBlock));
}

void Region::addFinalBasicBlock(std::unique_ptr<BasicBlock> basicBlock) {
  if (m_finalBB != nullptr) {
    throw IrisException("Final bb is already specified!");
  }
  m_finalBB = basicBlock.get();
  addBasicBlock(std::move(basicBlock));
}

const BasicBlock& Region::getStartBasicBlock() const {
  if (m_startBB != nullptr) {
    throw IrisException("No start basic block is specified!");
  }
  return *m_startBB;
}

const BasicBlock& Region::getFinalBasicBlock() const {
  if (m_finalBB != nullptr) {
    throw IrisException("No final basic block is specified!");
  }
  return *m_finalBB;
}

BasicBlock* Region::getBasicBlockByID(bb_id_t id) {
  for (const auto& bb : m_BasicBlocks) {
    if (bb->getID() == id) {
      return bb.get();
    }
  }
  return nullptr;
}

const BasicBlock* Region::getBasicBlockByID(bb_id_t id) const {
  for (const auto& bb : m_BasicBlocks) {
    if (bb->getID() == id) {
      return bb.get();
    }
  }
  return nullptr;
}

bool Region::isBasicBlockPresent(const BasicBlock* basicBlock) const {
  for (const auto& bb : m_BasicBlocks) {
    if (bb.get() == basicBlock) {
      return true;
    }
  }
  return false;
}

bool Region::setStartBasicBlock(bb_id_t id) {
  if (auto* ptr = getBasicBlockByID(id)) {
    m_startBB = ptr;
    return true;
  }
  return false;
}

bool Region::setStartBasicBlock(BasicBlock* basicBlock) {
  if (isBasicBlockPresent(basicBlock)) {
    m_startBB = basicBlock;
    return true;
  }
  return false;
}

bool Region::setFinalBasicBlock(bb_id_t id) {
  if (auto* ptr = getBasicBlockByID(id)) {
    m_finalBB = ptr;
    return true;
  }
  return false;
}

bool Region::setFinalBasicBlock(BasicBlock* basicBlock) {
  if (isBasicBlockPresent(basicBlock)) {
    m_finalBB = basicBlock;
    return true;
  }
  return false;
}

bool Region::removeBasicBlock(BasicBlock* basicBlock) {
  assert(basicBlock != nullptr);
  return removeBasicBlock(basicBlock->getID());
}

bool Region::removeBasicBlock(bb_id_t id) {
  for (auto bbIter = m_BasicBlocks.begin(); bbIter != m_BasicBlocks.end();
       ++bbIter) {
    auto& bbPtr = *bbIter;
    auto& bb = *bbPtr;

    if (bb.getID() == id) {
      bb.unlink();
      m_BasicBlocks.erase(bbIter);
      expireDomInfo();
      return true;
    }
  }
  return false;
}

bool Region::replaceBasicBlockWith(
  bb_id_t id, std::unique_ptr<BasicBlock> newBasicBlockPtr) {
  for (auto bbIter = m_BasicBlocks.begin(); bbIter != m_BasicBlocks.end();
       ++bbIter) {
    auto& oldBasicBlockPtr = *bbIter;
    auto& oldBasicBlock = *oldBasicBlockPtr;

    if (oldBasicBlock.getID() == id) {

      auto& newBasicBlock = *newBasicBlockPtr;
      if (auto* succ = oldBasicBlock.getSucc(true)) {
        newBasicBlock.linkSucc(succ, true);
      }
      if (auto* succ = oldBasicBlock.getSucc(false)) {
        newBasicBlock.linkSucc(succ, false);
      }

      oldBasicBlock.unlink();

      if (oldBasicBlockPtr.get() == m_startBB) {
        m_startBB = newBasicBlockPtr.get();
      }
      if (oldBasicBlockPtr.get() == m_finalBB) {
        m_finalBB = newBasicBlockPtr.get();
      }

      m_BasicBlocks.erase(bbIter);
      m_BasicBlocks.push_back(std::move(newBasicBlockPtr));

      expireDomInfo();
      return true;
    }
  }
  return false;
}

bool Region::replaceBasicBlockWith(BasicBlock* oldBasicBlock,
                                   std::unique_ptr<BasicBlock> newBasicBlock) {
  assert(oldBasicBlock != nullptr);
  return replaceBasicBlockWith(oldBasicBlock->getID(),
                               std::move(newBasicBlock));
}

void Region::dump(std::ostream& os) {
  os << m_name << ":" << std::endl;
  std::string bbIdent = "  ";
  for (const auto& bbPtr : m_BasicBlocks) {
    bbPtr->dump(os, bbIdent);
  }
}

bool Region::verify(std::string& msg) const {
  for (const auto& bb : m_BasicBlocks) {
    auto bbPtr = bb.get();

    bool isStart = (bbPtr == m_startBB);
    bool isFinal = (bbPtr == m_finalBB);

    if (!bb->verify(msg, isStart, isFinal)) {
      return false;
    }
  }
  return true;
}

} // namespace iris