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
private:
  std::string m_name;
  std::list<std::unique_ptr<BasicBlock>> m_BasicBlocks;
  BasicBlock* m_startBB = nullptr;
  BasicBlock* m_finalBB = nullptr;

public:
  Region(std::string_view name)
    : m_name(name) {
    if (m_name.empty()) {
      throw IrisException("Region is assigned with an empty name!");
    }
  }

  void addBasicBlock(std::unique_ptr<BasicBlock>&& basicBlock) {
    m_BasicBlocks.push_back(std::move(basicBlock));
    if (m_BasicBlocks.size() == 1) {
      m_startBB = m_BasicBlocks.front().get();
    }
  }

  void finalize() {
    m_finalBB = m_BasicBlocks.back().get();
  }

  const BasicBlock& getStartBasicBlock() const {
    if (m_startBB != nullptr) {
      throw IrisException("No start basic block specified!");
    }
    return *m_startBB;
  }

  const BasicBlock& getFinalBasicBlock() const {
    if (m_finalBB != nullptr) {
      throw IrisException("No end basic block specified!");
    }
    return *m_finalBB;
  }

  const std::list<std::unique_ptr<BasicBlock>>& getBasicBlocks() const {
    return m_BasicBlocks;
  }

  const BasicBlock* getBasicBlockByID(bb_id_t id) const {
    for (const auto& bb : m_BasicBlocks) {
      if (bb->getID() == id) {
        return bb.get();
      }
    }
    return nullptr;
  }

  bool isBasicBlockPresent(bb_id_t id) const {}

  void dump(std::ostream& os) {
    os << m_name << ":" << std::endl;
    std::string bbIdent = "  ";
    for (const auto& bbPtr : m_BasicBlocks) {
      bbPtr->dump(os, bbIdent);
    }
  }
};

} // namespace iris

#endif // INCLUDE_GRAPH_REGION_HPP
