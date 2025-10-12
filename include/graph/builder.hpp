#ifndef INCLUDE_BUILDER
#define INCLUDE_BUILDER

#include <memory>

#include <graph/region.hpp>

namespace iris {

class IRBuilder {
public:
  IRBuilder() = default;

  virtual ~IRBuilder() = default;

  void reset() {
    m_currRegion.reset();
    m_currBasicBlock.reset();
  }

  bool isRegionBuilding() const {
    return static_cast<bool>(m_currRegion);
  }

  bool isBasicBlockBuilding() const {
    return static_cast<bool>(m_currBasicBlock);
  }

  void startNewRegion(std::string_view name) {
    if (isRegionBuilding()) {
      throw IrisException("Current region is still building!");
    }

    m_currRegion.reset(new Region(name));
  }

  const Region& getCurRegion() const {
    if (!isRegionBuilding()) {
      throw IrisException("No region is in process currently");
    }
    return *m_currRegion;
  }

  Region& getCurRegion() {
    if (!isRegionBuilding()) {
      throw IrisException("No region is in process currently");
    }
    return *m_currRegion;
  }

  void dropRegion() {
    if (!isRegionBuilding()) {
      throw IrisException("No region is building!");
    }
    m_currRegion.reset();
  }

  std::unique_ptr<Region> obtainRegion() {
    if (!isRegionBuilding()) {
      throw IrisException("No region is building!");
    }

    if (isBasicBlockBuilding()) {
      throw IrisException("Current region is still building!");
    }

    return std::exchange(m_currRegion, std::unique_ptr<Region>());
  }

  bb_id_t obtainIdForBasicBlock() {
    if (!isRegionBuilding()) {
      throw IrisException("No region is building!");
    }
    return m_currRegion->obtainIDForBasicBlock();
  }

  void startNewBasicBlock() {
    if (!isRegionBuilding()) {
      throw IrisException("No region is building!");
    }

    if (isBasicBlockBuilding()) {
      throw IrisException("Current block is still building!");
    }

    auto id = m_currRegion->obtainIDForBasicBlock();
    m_currBasicBlock.reset(new BasicBlock(id));
  }

  void startNewBasicBlock(bb_id_t id) {
    if (!isRegionBuilding()) {
      throw IrisException("No region is building!");
    }

    if (isBasicBlockBuilding()) {
      throw IrisException("Current block is still building!");
    }

    if (m_currRegion->isBasicBlockPresent(id)) {
      // Attempt to create a basic block with an ID
      // that is already used in the region
      throw IrisException("ID is already used in the current region!");
    }

    m_currBasicBlock.reset(new BasicBlock(id));
  }

  template <typename OpTy, typename... Args>
  Operation* createAndAddOp(Args... args) {
    if (!isRegionBuilding() || !isBasicBlockBuilding()) {
      // No region or basic block in building process
      return nullptr;
    }

    auto op = std::make_unique<OpTy>(std::forward<Args>(args)...);
    auto* opPtr = op.get();

    op->setID(m_currRegion->obtainIDForOperation());
    m_currBasicBlock->insertOpBack(std::move(op));
    return opPtr;
  }

  int64_t getCurBasicBlockID() const {
    if (!isBasicBlockBuilding()) {
      return -1;
    }

    return static_cast<int64_t>(m_currBasicBlock->getID());
  }

  const BasicBlock& getCurBasicBlock() const {
    if (!isBasicBlockBuilding()) {
      throw IrisException("No basic block in building!");
    }
    return *m_currBasicBlock;
  }

  BasicBlock& getCurBasicBlock() {
    if (!isBasicBlockBuilding()) {
      throw IrisException("No basic block in building!");
    }
    return *m_currBasicBlock;
  }

  void dropBasicBlock() {
    if (!isBasicBlockBuilding()) {
      throw IrisException("No basic block is building!");
    }
    m_currBasicBlock.reset();
  }

  BasicBlock& finalizeBasicBlock() {
    if (!isBasicBlockBuilding()) {
      throw IrisException("No basic block in building!");
    }

    auto& bb = *m_currBasicBlock;
    m_currRegion->addBasicBlock(std::move(m_currBasicBlock));
    return bb;
  }

private:
  std::unique_ptr<Region> m_currRegion;
  std::unique_ptr<BasicBlock> m_currBasicBlock;
};

} // namespace iris

#endif // INCLUDE_BUILDER
