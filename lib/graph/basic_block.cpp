#include <set>

#include <graph/basic_block.hpp>
#include <graph/region.hpp>

#include <ops/dialects/ctrlflow/ops.hpp>
#include <ops/dialects/opcodes.hpp>

namespace iris {

void BasicBlock::addOp(std::unique_ptr<Operation> op) {
  op->setParentBasicBlock(this);
  if (op->isa(GlobalOpcodes::PHI)) {
    m_PhiOps.append_back(op.release());
  } else {
    m_RegOps.append_back(op.release());
  }
}

void BasicBlock::dump(std::ostream& os, const std::string& bbIdent) {
  os << bbIdent << "^bb" << m_ID << " ";

  if (m_preds.size() == 0) {
    os << "<start> ";
  } else {
    os << "<from";
    for (bb_id_t predID : m_preds) {
      os << " " << "bb" << predID;
    }
    os << "> ";
  }

  if (m_succTrueID == -1 && m_succFalseID == -1) {
    os << "<final> ";
  } else if (m_succTrueID != -1 && m_succFalseID != -1) {
    os << "<to T:bb" << m_succTrueID << " / F:bb" << m_succFalseID << "> ";
  } else {
    os << "<to bb" << m_succTrueID << "> ";
  }

  os << ": \n";

  std::string opIdent = bbIdent + "    ";
  for (auto phiOpIt = m_PhiOps.begin(); phiOpIt != m_PhiOps.end(); ++phiOpIt) {
    os << opIdent << static_cast<Operation&>(*phiOpIt) << std::endl;
  }
  for (auto regOpIt = m_RegOps.begin(); regOpIt != m_RegOps.end(); ++regOpIt) {
    os << opIdent << static_cast<Operation&>(*regOpIt) << std::endl;
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

  if (isFinal && (m_succTrueID != -1 || m_succFalseID != -1)) {
    msg = bbName + " is final bb, but has successors!";
    return false;
  }

  // TODO move to region's verifier maybe?
  for (bb_id_t predID : m_preds) {
    if (!m_ParentRegion->isBasicBlockPresent(predID)) {
      msg =
        bbName + "'s pred " + std::to_string(predID) + " is not in the region!";
      return false;
    }
  }

  // TODO move to region's verifier maybe?
  if (m_succTrueID != -1 &&
      !m_ParentRegion->isBasicBlockPresent(m_succTrueID)) {
    msg = bbName + "'s true successor is not in the region!";
    return false;
  }

  // TODO move to region's verifier maybe?
  if (m_succFalseID != -1 &&
      !m_ParentRegion->isBasicBlockPresent(m_succFalseID)) {
    msg = bbName + "'s false successor is not in the region!";
    return false;
  }

  if (m_succFalseID != -1 && m_succTrueID == -1) {
    msg =
      bbName + " has false successor specified, but true successor is missing!";
    return false;
  }

  if (!isFinal && m_succTrueID == -1) {
    msg = bbName + " is not final, but has no successors!";
    return false;
  }

  if (m_RegOps.size() == 0) {
    msg = bbName + " is empty!";
    return false;
  }

  const Operation& lastOp = static_cast<const Operation&>(m_RegOps.cback());

  if (isFinal && !lastOp.isa(GlobalOpcodes::RETURN)) {
    msg = bbName +
          " is final, but its last operation is not an \'ctrlflow.return\'";
    return false;
  }

  bool hasTwoSuccs = (m_succFalseID != -1);
  bool lastOpIsCondJump = lastOp.isa(GlobalOpcodes::JUMPC);

  if (hasTwoSuccs && m_succTrueID == m_succFalseID) {
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
  auto opIt = m_RegOps.begin();
  for (std::size_t opIdx = 0; opIdx < m_RegOps.size() - 1; ++opIdx) {
    const Operation& op = static_cast<const Operation&>(*opIt);
    if (op.isTerminator()) {
      msg =
        bbName + " - terminator operation is not the last one in the block!";
      return false;
    }

    if (!op.verify(msg)) {
      return false;
    }
    ++opIt;
  }

  return true;
}

} // namespace iris
