#ifndef INCLUDE_GRAPH_REGION_HPP
#define INCLUDE_GRAPH_REGION_HPP

#include <memory>
#include <vector>

#include <exception.hpp>
#include <graph/basic_block.hpp>

namespace iris {

class Region {
private:
  std::vector<std::unique_ptr<BasicBlock>> m_BasicBlocks;
  BasicBlock* m_startBB = nullptr;
  BasicBlock* m_endBB = nullptr;

public:
  void addBasicBlock(std::unique_ptr<BasicBlock>&& basicBlock) {
    m_BasicBlocks.push_back(std::move(basicBlock));
  }

  void addStartBasicBlock(std::unique_ptr<BasicBlock>&& basicBlock) {
    if (m_startBB != nullptr) {
      throw IrisException("Start basic block has been already added!");
    }
    m_startBB = basicBlock.get();
    m_BasicBlocks.push_back(std::move(basicBlock));
  }

  void addEndBasicBlock(std::unique_ptr<BasicBlock>&& basicBlock) {
    if (m_endBB != nullptr) {
      throw IrisException("Start basic block has been already added!");
    }
    m_endBB = basicBlock.get();
    m_BasicBlocks.push_back(std::move(basicBlock));
  }

  const BasicBlock& getStartBasicBlock() const {
    if (m_startBB != nullptr) {
      throw IrisException("No start basic block specified!");
    }
    return *m_startBB;
  }

  const BasicBlock& getEndBasicBlock() const {
    if (m_endBB != nullptr) {
      throw IrisException("No end basic block specified!");
    }
    return *m_endBB;
  }
};

} // namespace iris

#endif // INCLUDE_GRAPH_REGION_HPP
