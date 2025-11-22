#include <cassert>

#include <graph/basic_block.hpp>
#include <graph/region.hpp>

#include <ops/dialects/opcodes.hpp>

namespace iris {

void BasicBlock::replaceWith(BasicBlock& that) {
  that.unlink();

  for (auto pred : m_preds) {
    if (pred->m_succTrue == this) {
      pred->m_succTrue = &that;
    }
    if (pred->m_succFalse == this) {
      pred->m_succFalse = &that;
    }
  }

  that.m_preds = std::move(m_preds);

  if (auto succT = m_succTrue) {
    clearSucc(true);
    that.linkSucc(succT, true);
  }

  if (auto succF = m_succFalse) {
    clearSucc(false);
    that.linkSucc(succF, false);
  }
}

void BasicBlock::insertPhiOpBack(std::unique_ptr<ctrlflow::PhiOp> op) {
  assert(!!op);
  op->setParentBasicBlock(this);
  m_PhiOps.insertBack(std::move(op));
}

void BasicBlock::erasePhiOp(op_iterator pos) {
  m_PhiOps.erase(pos);
}

void BasicBlock::erasePhiOp(const_op_iterator pos) {
  m_PhiOps.erase(pos);
}

void BasicBlock::doReplaceOpWith(Operation* opPtr, Operation* nodePtr) {
  // Set parent basic block
  opPtr->setParentBasicBlock(this);
  // Replace all uses of the previous operation with new one
  opPtr->replaceAllUsesOf(*nodePtr);
}

void BasicBlock::replacePhiOpWith(op_iterator pos,
                                  std::unique_ptr<ctrlflow::PhiOp> op) {
  assert(!!op);
  auto opPtr = op.get();
  auto nodePtr = pos.get();

  doReplaceOpWith(opPtr, nodePtr);

  m_PhiOps.insertAfter(pos, std::move(op));
  m_PhiOps.erase(pos);
}

void BasicBlock::replacePhiOpWith(const_op_iterator pos,
                                  std::unique_ptr<ctrlflow::PhiOp> op) {
  assert(!!op);
  auto opPtr = op.get();
  auto nodePtr = const_cast<Operation*>(pos.get());

  doReplaceOpWith(opPtr, nodePtr);

  m_PhiOps.insertAfter(pos, std::move(op));
  m_PhiOps.erase(pos);
}

void BasicBlock::insertOpFront(std::unique_ptr<Operation> op) {
  assert(!!op);
  op->setParentBasicBlock(this);
  m_RegOps.insertFront(std::move(op));
}

void BasicBlock::insertOpBack(std::unique_ptr<Operation> op) {
  assert(!!op);
  op->setParentBasicBlock(this);
  m_RegOps.insertBack(std::move(op));
}

void BasicBlock::insertOpAfter(op_iterator pos, std::unique_ptr<Operation> op) {
  assert(!!op);
  op->setParentBasicBlock(this);
  m_RegOps.insertAfter(pos, std::move(op));
}

void BasicBlock::insertOpAfter(const_op_iterator pos,
                               std::unique_ptr<Operation> op) {
  assert(!!op);
  op->setParentBasicBlock(this);
  m_RegOps.insertAfter(pos, std::move(op));
}

void BasicBlock::insertOpBefore(op_iterator pos,
                                std::unique_ptr<Operation> op) {
  op->setParentBasicBlock(this);
  m_RegOps.insertBefore(pos, std::move(op));
}

void BasicBlock::insertOpBefore(const_op_iterator pos,
                                std::unique_ptr<Operation> op) {
  assert(!!op);
  op->setParentBasicBlock(this);
  m_RegOps.insertBefore(pos, std::move(op));
}

void BasicBlock::eraseOp(op_iterator pos) {
  m_RegOps.erase(pos);
}

void BasicBlock::eraseOp(const_op_iterator pos) {
  m_RegOps.erase(pos);
}

void BasicBlock::replaceOpWith(op_iterator pos, std::unique_ptr<Operation> op) {
  assert(!!op);
  auto opPtr = op.get();
  auto nodePtr = pos.get();

  doReplaceOpWith(opPtr, nodePtr);

  m_RegOps.insertAfter(pos, std::move(op));
  m_RegOps.erase(pos);
}
void BasicBlock::replaceOpWith(const_op_iterator pos,
                               std::unique_ptr<Operation> op) {
  assert(!!op);
  auto opPtr = op.get();
  auto nodePtr = const_cast<Operation*>(pos.get());

  doReplaceOpWith(opPtr, nodePtr);

  m_RegOps.insertAfter(pos, std::move(op));
  m_RegOps.erase(pos);
}

void BasicBlock::dump(std::ostream& os, const std::string& bbIdent) {
  os << bbIdent << "^bb" << m_ID << " ";

  if (m_preds.size() == 0) {
    os << "<start> ";
  } else {
    os << "<from";
    for (auto pred : m_preds) {
      os << " " << "bb" << pred->m_ID;
    }
    os << "> ";
  }

  if (m_succTrue == nullptr) {
    os << "<final> ";
  } else if (m_succFalse != nullptr) {
    os << "<to T:bb" << m_succTrue->m_ID << " / F:bb" << m_succFalse->m_ID
       << "> ";
  } else {
    os << "<to bb" << m_succTrue->m_ID << "> ";
  }

  os << ": \n";

  std::string opIdent = bbIdent + "    ";
  for (auto phiOpIt = m_PhiOps.begin(); phiOpIt != m_PhiOps.end(); ++phiOpIt) {
    os << opIdent << *phiOpIt << std::endl;
  }
  for (auto regOpIt = m_RegOps.begin(); regOpIt != m_RegOps.end(); ++regOpIt) {
    os << opIdent << *regOpIt << std::endl;
  }
}

bool BasicBlock::verify(std::string& msg, bool isStart, bool isFinal) {
  std::string bbName = "BB" + std::to_string(m_ID);

  if (m_ParentRegion == nullptr) {
    msg = bbName + " has no parent region!";
    return false;
  }

  if (isStart && m_preds.size() != 0) {
    msg = bbName + " is starting bb, but has predecessor!";
    return false;
  }

  if (isFinal && (m_succTrue != nullptr || m_succFalse != nullptr)) {
    msg = bbName + " is final bb, but has successors!";
    return false;
  }

  // TODO move to region's verifier maybe?
  for (auto pred : m_preds) {
    if (!m_ParentRegion->isBasicBlockPresentByID(pred->m_ID)) {
      msg = bbName + "'s pred " + std::to_string(pred->m_ID) +
            " is not in the region!";
      return false;
    }
  }

  // TODO move to region's verifier maybe?
  if (m_succTrue != nullptr &&
      !m_ParentRegion->isBasicBlockPresentByID(m_succTrue->m_ID)) {
    msg = bbName + "'s true successor is not in the region!";
    return false;
  }

  // TODO move to region's verifier maybe?
  if (m_succFalse != nullptr &&
      !m_ParentRegion->isBasicBlockPresentByID(m_succFalse->m_ID)) {
    msg = bbName + "'s false successor is not in the region!";
    return false;
  }

  if (m_succFalse != nullptr && m_succTrue == nullptr) {
    msg =
      bbName + " has false successor specified, but true successor is missing!";
    return false;
  }

  if (!isFinal && m_succTrue == nullptr) {
    msg = bbName + " is not final, but has no successors!";
    return false;
  }

  if (m_RegOps.size() == 0) {
    msg = bbName + " is empty!";
    return false;
  }

  const Operation& lastOp = m_RegOps.cback();

  if (isFinal && !lastOp.isa(GlobalOpcodes::RETURN)) {
    msg = bbName +
          " is final, but its last operation is not an \'ctrlflow.return\'";
    return false;
  }

  bool hasTwoSuccs = (m_succFalse != nullptr);
  bool lastOpIsCondJump = lastOp.isa(GlobalOpcodes::JUMPC);

  if (hasTwoSuccs && m_succTrue == m_succFalse) {
    msg = bbName + " has two identical successors!";
    return false;
  }

  if (hasTwoSuccs && !lastOpIsCondJump) {
    msg = bbName +
          " has two successors, but conditional jump at the end is missing!";
    return false;
  }

  if (!hasTwoSuccs && lastOpIsCondJump) {
    msg =
      bbName + " has single successor, but has conditional jump at the end!";
    return false;
  }

  if (!verifyOps(msg, bbName)) {
    return false;
  }

  return true;
}

bool BasicBlock::verifyOps(std::string& msg, const std::string& bbName) {
  // Phi operations
  for (auto phiOpIt = m_PhiOps.begin(); phiOpIt != m_PhiOps.end(); ++phiOpIt) {
    const Operation& op = *phiOpIt;
    if (!op.verify(msg)) {
      return false;
    }
  }

  // Regular operations
  auto regOpIt = m_RegOps.begin();
  for (std::size_t opIdx = 0U; opIdx < m_RegOps.size(); ++opIdx) {
    const Operation& op = *regOpIt;
    if (opIdx + 1U != m_RegOps.size() && op.isTerminator()) {
      msg =
        bbName + " - terminator operation is not the last one in the block!";
      return false;
    }

    if (op.isa(GlobalOpcodes::PHI)) {
      msg = bbName + " - phi operation is not in the phi ops list!";
      return false;
    }

    if (!op.verify(msg)) {
      return false;
    }

    ++regOpIt;
  }

  return true;
}

void BasicBlock::removeFromPredsAsSucc() {
  for (auto bb : m_preds) {

    if (bb->m_succTrue == this) {
      bb->m_succTrue = nullptr;
    }

    if (bb->m_succFalse == this) {
      bb->m_succFalse = nullptr;
    }
  }
  m_preds.clear();
}

} // namespace iris
