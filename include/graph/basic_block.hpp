#ifndef INCLUDE_GRAPH_BASIC_BLOCK_HPP
#define INCLUDE_GRAPH_BASIC_BLOCK_HPP

#include <cstdint>
#include <memory>
#include <ostream>

#include <ops/dialects/opcodes.hpp>
#include <ops/generic/operation.hpp>

namespace iris {

class Region;

class BasicBlock final {
private:
  std::list<bb_id_t> m_preds;
  int64_t m_succTrueID = -1;
  int64_t m_succFalseID = -1;

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

  void setParentRegion(Region* region) {
    m_ParentRegion = region;
  }

  //--- BB's predecessors ---

  const std::list<bb_id_t>& getPreds() const {
    return m_preds;
  }

  std::size_t getPredsNum() const {
    return m_preds.size();
  }

  //--- BB's successor ---

  void setSucc(BasicBlock& succ, bool which = true) {
    if (which == true) {
      m_succTrueID = succ.m_ID;
    } else {
      m_succFalseID = succ.m_ID;
    }
    succ.m_preds.push_back(m_ID);
  }

  bool hasSucc(bool which = true) const {
    auto succID = (which == true) ? m_succTrueID : m_succFalseID;
    return (succID != -1);
  }

  bb_id_t getSuccID(bool which = true) const {
    return static_cast<bb_id_t>((which == true) ? m_succTrueID : m_succFalseID);
  }

  void clearSucc(bool which) {
    if (which) {
      m_succTrueID = -1;
    } else {
      m_succFalseID = -1;
    }
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

  bb_id_t getID() const {
    return m_ID;
  }

  // Text dump to the given ouput stream
  void dump(std::ostream& os, const std::string& bbIdent);

  bool verify(std::string& msg, bool isStart = false, bool isFinal = false);

private:
  void addPred(bb_id_t predID) {
    m_preds.push_back(predID);
  }

  void addPred(const BasicBlock& pred) {
    m_preds.push_back(pred.m_ID);
  }

  template <typename IterT>
  void removePred(IterT iter) {
    m_preds.erase(iter);
  }

  void removePred(std::size_t pos) {
    m_preds.erase(std::next(m_preds.begin(), pos));
  }

  void removePred(bb_id_t predID) {
    m_preds.remove(predID);
  }
};

} // namespace iris

#endif // INCLUDE_GRAPH_BASIC_BLOCK_HPP