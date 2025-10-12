#ifndef INCLUDE_GRAPH_BASIC_BLOCK_HPP
#define INCLUDE_GRAPH_BASIC_BLOCK_HPP

#include <cstdint>
#include <memory>
#include <ostream>

#include <ops/generic/operation.hpp>

#include <ops/dialects/ctrlflow/ops.hpp>
#include <ops/dialects/opcodes.hpp>

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

  ~BasicBlock() = default;

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

  const std::list<BasicBlock*>& getPreds() const {
    return m_preds;
  }

  std::size_t getPredsNum() const {
    return m_preds.size();
  }

  //--- BB's successor ---

  void unlink() {
    clearSucc(true);
    clearSucc(false);
    removeFromPredsAsSucc();
  }

  bool linkSucc(BasicBlock* bb, bool which = true) {
    auto& succ = (which == true) ? m_succTrue : m_succFalse;
    if (succ != nullptr || bb == nullptr) {
      // Successor is already set, or given bb pointer is invalid
      return false;
    }
    succ = bb;
    succ->m_preds.push_back(this);
    return true;
  }

  void clearSucc(bool which = true) {
    auto& succ = (which == true) ? m_succTrue : m_succFalse;
    if (succ != nullptr) {
      succ->m_preds.remove(this);
    }
    succ = nullptr;
  }

  bool hasSucc(bool which = true) const {
    auto succ = (which == true) ? m_succTrue : m_succFalse;
    return (succ != nullptr);
  }

  const BasicBlock* getSucc(bool which = true) const {
    return (which == true) ? m_succTrue : m_succFalse;
  }

  int64_t getSuccID(bool which = true) const {
    auto succ = (which == true) ? m_succTrue : m_succFalse;
    return (succ != nullptr) ? static_cast<int64_t>(succ->m_ID) : -1;
  }

  //--- Operation ---

  using OpList = detail::List;
  using op_iterator = OpList::iterator;
  using const_op_iterator = OpList::const_iterator;

  const OpList& getPhiOps() const& {
    return m_PhiOps;
  }

  const OpList& getOps() const& {
    return m_RegOps;
  }

  void insertPhiOpBack(std::unique_ptr<ctrlflow::PhiOp> op);
  void erasePhiOp(op_iterator pos);
  void erasePhiOp(const_op_iterator pos);

  void insertOpFront(std::unique_ptr<Operation> op);
  void insertOpBack(std::unique_ptr<Operation> op);

  void insertOpAfter(op_iterator pos, std::unique_ptr<Operation> op);
  void insertOpAfter(const_op_iterator pos, std::unique_ptr<Operation> op);

  void insertOpBefore(op_iterator pos, std::unique_ptr<Operation> op);
  void insertOpBefore(const_op_iterator pos, std::unique_ptr<Operation> op);

  void eraseOp(op_iterator pos);
  void eraseOp(const_op_iterator pos);

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
  std::list<BasicBlock*> m_preds;
  BasicBlock* m_succTrue = nullptr;
  BasicBlock* m_succFalse = nullptr;

  OpList m_PhiOps;
  OpList m_RegOps;

  Region* m_ParentRegion = nullptr;

  // Identifier of the basic block
  bb_id_t m_ID;

  // Verify operations, which bb contains
  bool verifyOps(std::string& msg, const std::string& bbName);

  void removeFromPredsAsSucc();
};

} // namespace iris

#endif // INCLUDE_GRAPH_BASIC_BLOCK_HPP