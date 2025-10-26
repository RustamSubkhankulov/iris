#include <graph/region.hpp>

namespace iris {

void Region::addStartBasicBlock(std::unique_ptr<BasicBlock> basicBlock) {
  assert(!!basicBlock);
  m_startBB = basicBlock.get();
  addBasicBlock(std::move(basicBlock));
}

void Region::addFinalBasicBlock(std::unique_ptr<BasicBlock> basicBlock) {
  assert(!!basicBlock);
  m_finalBB = basicBlock.get();
  addBasicBlock(std::move(basicBlock));
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

bool Region::setStartBasicBlockByID(bb_id_t id) {
  if (auto* ptr = getBasicBlockByID(id)) {
    m_startBB = ptr;
    return true;
  }
  return false;
}

bool Region::setStartBasicBlock(BasicBlock* basicBlock) {
  assert(basicBlock != nullptr);
  if (isBasicBlockPresent(basicBlock)) {
    m_startBB = basicBlock;
    return true;
  }
  return false;
}

bool Region::setFinalBasicBlockByID(bb_id_t id) {
  if (auto* ptr = getBasicBlockByID(id)) {
    m_finalBB = ptr;
    return true;
  }
  return false;
}

bool Region::setFinalBasicBlock(BasicBlock* basicBlock) {
  assert(basicBlock != nullptr);
  if (isBasicBlockPresent(basicBlock)) {
    m_finalBB = basicBlock;
    return true;
  }
  return false;
}

bool Region::removeBasicBlock(BasicBlock* basicBlock) {
  assert(basicBlock != nullptr);
  return removeBasicBlockByID(basicBlock->getID());
}

bool Region::removeBasicBlockByID(bb_id_t id) {
  for (auto bbIter = m_BasicBlocks.begin(); bbIter != m_BasicBlocks.end();
       ++bbIter) {
    auto& bbPtr = *bbIter;
    auto& bb = *bbPtr;

    if (bb.getID() == id) {
      bb.unlink();

      if (bbPtr.get() == m_startBB) {
        m_startBB = nullptr;
      }

      if (bbPtr.get() == m_finalBB) {
        m_finalBB = nullptr;
      }

      m_BasicBlocks.erase(bbIter);
      expireDomAndLoopInfo();
      return true;
    }
  }
  return false;
}

bool Region::replaceBasicBlockWithByID(
  bb_id_t id, std::unique_ptr<BasicBlock> newBasicBlockPtr) {
  assert(!!newBasicBlockPtr);

  for (auto bbIter = m_BasicBlocks.begin(); bbIter != m_BasicBlocks.end();
       ++bbIter) {
    auto& oldBasicBlockPtr = *bbIter;

    if (oldBasicBlockPtr->getID() == id) {

      oldBasicBlockPtr->replaceWith(*newBasicBlockPtr);

      if (oldBasicBlockPtr.get() == m_startBB) {
        m_startBB = newBasicBlockPtr.get();
      }
      if (oldBasicBlockPtr.get() == m_finalBB) {
        m_finalBB = newBasicBlockPtr.get();
      }

      m_BasicBlocks.erase(bbIter);
      m_BasicBlocks.push_back(std::move(newBasicBlockPtr));

      expireDomAndLoopInfo();
      return true;
    }
  }
  return false;
}

bool Region::replaceBasicBlockWith(BasicBlock* oldBasicBlock,
                                   std::unique_ptr<BasicBlock> newBasicBlock) {
  assert(oldBasicBlock != nullptr);
  assert(!!newBasicBlock);

  return replaceBasicBlockWithByID(oldBasicBlock->getID(),
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