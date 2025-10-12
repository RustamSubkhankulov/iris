#ifndef INCLUDE_GRAPH_REGION_HPP
#define INCLUDE_GRAPH_REGION_HPP

#include <list>
#include <memory>
#include <ostream>
#include <string_view>

#include <exception.hpp>
#include <graph/basic_block.hpp>

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

  bb_id_t obtainIDForBasicBlock() {
    return m_bbIDProvider.obtainID();
  }

  op_id_t obtainIDForOperation() {
    return m_opIDProvider.obtainID();
  }

  void addBasicBlock(std::unique_ptr<BasicBlock> basicBlock) {
    basicBlock->setParentRegion(this);
    m_BasicBlocks.push_back(std::move(basicBlock));
  }

  void addStartBasicBlock(std::unique_ptr<BasicBlock> basicBlock);
  void addFinalBasicBlock(std::unique_ptr<BasicBlock> basicBlock);

  bool setStartBasicBlock(bb_id_t id);
  bool setStartBasicBlock(BasicBlock* basicBlock);

  bool setFinalBasicBlock(bb_id_t id);
  bool setFinalBasicBlock(BasicBlock* basicBlock);

  const BasicBlock& getStartBasicBlock() const;

  const BasicBlock& getFinalBasicBlock() const;

  const std::list<std::unique_ptr<BasicBlock>>& getBasicBlocks() const {
    return m_BasicBlocks;
  }

  BasicBlock* getBasicBlockByID(bb_id_t id);
  const BasicBlock* getBasicBlockByID(bb_id_t id) const;

  bool isBasicBlockPresent(bb_id_t id) const {
    return (getBasicBlockByID(id) != nullptr);
  }

  bool isBasicBlockPresent(const BasicBlock* basicBlock) const;

  bool removeBasicBlock(BasicBlock* basicBlock);
  bool removeBasicBlock(bb_id_t id);

  bool replaceBasicBlockWith(bb_id_t id,
                             std::unique_ptr<BasicBlock> newBasicBlock);
  bool replaceBasicBlockWith(BasicBlock* oldBasicBlock,
                             std::unique_ptr<BasicBlock> newBasicBlock);

  void dump(std::ostream& os);
  bool verify(std::string& msg) const;

private:
  std::string m_name;
  std::list<std::unique_ptr<BasicBlock>> m_BasicBlocks;
  BasicBlock* m_startBB = nullptr;
  BasicBlock* m_finalBB = nullptr;

  detail::IDProvider<bb_id_t> m_bbIDProvider;
  detail::IDProvider<op_id_t> m_opIDProvider;
};

} // namespace iris

#endif // INCLUDE_GRAPH_REGION_HPP
