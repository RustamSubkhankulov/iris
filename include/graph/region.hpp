#ifndef INCLUDE_GRAPH_REGION_HPP
#define INCLUDE_GRAPH_REGION_HPP

#include <cassert>
#include <list>
#include <memory>
#include <ostream>
#include <string_view>

#include <exception.hpp>
#include <graph/basic_block.hpp>
#include <graph/doms.hpp>

namespace iris {

class Region final {
public:
  explicit Region(std::string_view name)
    : m_name(name) {
    if (m_name.empty()) {
      throw IrisException("Region is assigned with an empty name!");
    }
  }

  std::string_view getName() const {
    return m_name;
  }

  //--- ID providers ---

  bb_id_t obtainIDForBasicBlock() {
    return m_bbIDProvider.obtainID();
  }

  op_id_t obtainIDForOperation() {
    return m_opIDProvider.obtainID();
  }

  //--- Adding basic block ---

  void addBasicBlock(std::unique_ptr<BasicBlock> basicBlock) {
    assert(!!basicBlock);
    basicBlock->setParentRegion(this);
    m_BasicBlocks.push_back(std::move(basicBlock));
    expireDomInfo();
  }

  void addStartBasicBlock(std::unique_ptr<BasicBlock> basicBlock);
  void addFinalBasicBlock(std::unique_ptr<BasicBlock> basicBlock);

  //--- Start & final basic blocks ---

  bool setStartBasicBlockByID(bb_id_t id);
  bool setStartBasicBlock(BasicBlock* basicBlock);

  bool setFinalBasicBlockByID(bb_id_t id);
  bool setFinalBasicBlock(BasicBlock* basicBlock);

  bool hasStartBasicBlock() const {
    return (m_startBB != nullptr);
  }

  bool hasFinalBasicBlock() const {
    return (m_finalBB != nullptr);
  }

  const BasicBlock* getStartBasicBlock() const {
    return m_startBB;
  }

  const BasicBlock* getFinalBasicBlock() const {
    return m_finalBB;
  }

  BasicBlock* getStartBasicBlock() {
    return m_startBB;
  }

  BasicBlock* getFinalBasicBlock() {
    return m_finalBB;
  }

  //--- RO Accessing basic blocks ---

  const std::list<std::unique_ptr<BasicBlock>>& getBasicBlocks() const {
    return m_BasicBlocks;
  }

  //--- Basic block query ---

  BasicBlock* getBasicBlockByID(bb_id_t id);
  const BasicBlock* getBasicBlockByID(bb_id_t id) const;

  bool isBasicBlockPresentByID(bb_id_t id) const {
    return (getBasicBlockByID(id) != nullptr);
  }

  bool isBasicBlockPresent(const BasicBlock* basicBlock) const;

  //--- Removing basic block ---

  bool removeBasicBlock(BasicBlock* basicBlock);
  bool removeBasicBlockByID(bb_id_t id);

  //--- Replacing basic block ---

  bool replaceBasicBlockWithByID(bb_id_t id,
                                 std::unique_ptr<BasicBlock> newBasicBlock);
  bool replaceBasicBlockWith(BasicBlock* oldBasicBlock,
                             std::unique_ptr<BasicBlock> newBasicBlock);

  //--- Dominators information ---

  bool isDomInfoExpired() const noexcept {
    return m_domInfo.isExpired();
  }

  void collectDomInfo() {
    m_domInfo.analyze(*this);
  }

  const doms::DomInfo& getDomInfo() const {
    return m_domInfo;
  }

  std::vector<const BasicBlock*> getDFS() const {
    return doms::DomInfo::getDFS(*this);
  }

  std::vector<const BasicBlock*> getRPO() const {
    return doms::DomInfo::getRPO(*this);
  }

  //--- Misc ---

  void dump(std::ostream& os);
  bool verify(std::string& msg) const;

private:
  void expireDomInfo() noexcept {
    m_domInfo.expire();
  }

private:
  std::string m_name;
  std::list<std::unique_ptr<BasicBlock>> m_BasicBlocks;
  BasicBlock* m_startBB = nullptr;
  BasicBlock* m_finalBB = nullptr;

  detail::IDProvider<bb_id_t> m_bbIDProvider;
  detail::IDProvider<op_id_t> m_opIDProvider;

  doms::DomInfo m_domInfo;
};

} // namespace iris

#endif // INCLUDE_GRAPH_REGION_HPP
