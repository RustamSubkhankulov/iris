#include <graph/basic_block.hpp>

namespace iris {

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
  return true;
}

} // namespace iris
