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
public:
  explicit BasicBlock(bb_id_t id = 0U)
    : m_ID(id) {}

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

  bool linkSucc(BasicBlock& succ, bool which = true) {
    auto& succID = (which == true) ? m_succTrueID : m_succFalseID;
    if (succID != -1) {
      // Successor is already set.
      return false;
    }
    succID = succ.m_ID;
    succ.m_preds.push_back(m_ID);
    return true;
  }

  bool hasSucc(bool which = true) const {
    auto succID = (which == true) ? m_succTrueID : m_succFalseID;
    return (succID != -1);
  }

  bb_id_t getSuccID(bool which = true) const {
    return static_cast<bb_id_t>((which == true) ? m_succTrueID : m_succFalseID);
  }

  //--- Operation ---

  void addOp(std::unique_ptr<Operation> op);

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
  std::list<bb_id_t> m_preds;
  int64_t m_succTrueID = -1;
  int64_t m_succFalseID = -1;

  detail::List m_PhiOps;
  detail::List m_RegOps;

  Region* m_ParentRegion = nullptr;

  // Identifier of the basic block
  bb_id_t m_ID;

  bool verifyOps(std::string& msg, const std::string& bbName);
};

} // namespace iris

#endif // INCLUDE_GRAPH_BASIC_BLOCK_HPP