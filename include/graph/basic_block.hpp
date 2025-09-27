#ifndef INCLUDE_GRAPH_BASIC_BLOCK_HPP
#define INCLUDE_GRAPH_BASIC_BLOCK_HPP

#include <memory>
#include <ostream>

#include <ops/dialects/ctrlflow/ops.hpp>
#include <ops/dialects/opcodes.hpp>
#include <ops/generic/operation.hpp>

namespace iris {

class Region;

class BasicBlock final {
private:
  std::list<BasicBlock*> m_preds;
  BasicBlock* m_succTrue = nullptr;
  BasicBlock* m_succFalse = nullptr;

  List m_PhiOps;
  List m_RegOps;

  Region* m_ParentRegion;

  // Identifier of the basic block
  bb_id_t m_ID = 0U;

public:
  BasicBlock() = default;

  BasicBlock(const BasicBlock&) = delete;
  BasicBlock& operator=(const BasicBlock&) = delete;

  BasicBlock(BasicBlock&&) = delete;
  BasicBlock& operator=(BasicBlock&&) = delete;

  //--- BB's parent region ---

  bool hasParentRegion() const {
    return (m_ParentRegion != nullptr);
  }

  Region* getParentRegion() {
    return m_ParentRegion;
  }

  const Region* getParentRegion() const {
    return m_ParentRegion;
  }

  //--- BB's predecessors ---

  const std::list<BasicBlock*>& getPreds() const {
    return m_preds;
  }

  void addPred(BasicBlock* pred) {
    m_preds.push_back(pred);
  }

  template <typename IterT>
  void removePred(IterT iter) {
    m_preds.erase(iter);
  }

  void removePred(std::size_t pos) {
    m_preds.erase(std::next(m_preds.begin(), pos));
  }

  void removePred(BasicBlock* pred) {
    m_preds.remove(pred);
  }

  //--- BB's successor ---

  void setSucc(BasicBlock* succ, bool which = true) {
    if (which == true) {
      m_succTrue = succ;
    } else {
      m_succFalse = succ;
    }
  }

  bool hasSucc(bool which = true) const {
    auto* succ = (which == true) ? m_succTrue : m_succFalse;
    return (succ != nullptr);
  }

  BasicBlock* getSucc(bool which = true) {
    return (which == true) ? m_succTrue : m_succFalse;
  }

  const BasicBlock* getSucc(bool which = true) const {
    return (which == true) ? m_succTrue : m_succFalse;
  }

  //--- Operation ---

  void addOp(std::unique_ptr<Operation>&& op) {
    op->setParentBasicBlock(this);
    if (op->isa(GlobalOpcodes::PHI)) {
      m_PhiOps.append_back(op.release());
    } else {
      m_RegOps.append_back(op.release());
    }
  }

  //--- Misc ---
  void setID(bb_id_t id) {
    m_ID = id;
  }

  void dump(std::ostream& os, const std::string& bbIdent) {
    os << bbIdent << "^bb" << m_ID << ":" << std::endl;
    std::string opIdent = bbIdent + "    ";
    for (auto phiOpIt = m_PhiOps.begin(); phiOpIt != m_PhiOps.end();
         ++phiOpIt) {
      os << opIdent << static_cast<Operation&>(*phiOpIt) << std::endl;
    }
    for (auto regOpIt = m_RegOps.begin(); regOpIt != m_RegOps.end();
         ++regOpIt) {
      os << opIdent << static_cast<Operation&>(*regOpIt) << std::endl;
    }
  }
};

} // namespace iris

#endif // INCLUDE_GRAPH_BASIC_BLOCK_HPP