#ifndef INCLUDE_BUILDER
#define INCLUDE_BUILDER

#include <memory>

#include <graph/region.hpp>

namespace iris {

class IRBuilder {
public:
  IRBuilder() = default;

  virtual ~IRBuilder() {
    delete m_currRegion;
    delete m_currBasicBlock;
  }

  void reset() {
    delete m_currRegion;
    delete m_currBasicBlock;

    m_currRegion = nullptr;
    m_currBasicBlock = nullptr;
  }

  bool isRegionBuilding() const {
    return (m_currRegion != nullptr);
  }

  bool isBasicBlockBuilding() const {
    return (m_currBasicBlock != nullptr);
  }

  void startNewRegion(std::string_view name) {
    if (m_currRegion != nullptr) {
      throw IrisException("Current region is still building!");
    }

    m_currRegion = new Region(name);
  }

  const Region& getCurRegion() const {
    if (m_currRegion == nullptr) {
      throw IrisException("No region is in process currently");
    }
    return *m_currRegion;
  }

  Region& getCurRegion() {
    if (m_currRegion == nullptr) {
      throw IrisException("No region is in process currently");
    }
    return *m_currRegion;
  }

  void dropRegion() {
    if (m_currRegion == nullptr) {
      throw IrisException("No region is building!");
    }
    delete m_currRegion;
  }

  std::unique_ptr<Region> obtainRegion() {
    if (m_currRegion == nullptr && m_currBasicBlock != nullptr) {
      return nullptr;
    }

    m_currRegion->finalize();
    auto* tmp = m_currRegion;
    m_currRegion = nullptr;
    return std::unique_ptr<Region>(tmp);
  }

  bb_id_t obtainIdForBasicBlock() {
    if (m_currRegion == nullptr) {
      throw IrisException("No region is building!");
    }
    return m_currRegion->obtainIDForBasicBlock();
  }

  void startNewBasicBlock() {
    if (m_currRegion == nullptr) {
      throw IrisException("No region is building!");
    }

    if (m_currBasicBlock != nullptr) {
      throw IrisException("Current block is still building!");
    }

    auto id = m_currRegion->obtainIDForBasicBlock();
    m_currBasicBlock = new BasicBlock(id);
  }

  void startNewBasicBlock(bb_id_t id) {
    if (m_currRegion == nullptr) {
      throw IrisException("No region is building!");
    }

    if (m_currBasicBlock != nullptr) {
      throw IrisException("Current block is still building!");
    }

    if (m_currRegion->isBasicBlockPresent(id)) {
      // Attempt to create a basic block with an ID
      // that is already used in the region
      throw IrisException("ID is already used in the current region!");
    }

    m_currBasicBlock = new BasicBlock(id);
  }

  template <typename OpTy, typename... Args>
  Operation* createAndAddOp(Args... args) {
    if (m_currRegion == nullptr || m_currBasicBlock == nullptr) {
      // No basic block in building process
      return nullptr;
    }

    auto op = std::make_unique<OpTy>(std::forward<Args>(args)...);
    auto* opPtr = op.get();

    op->setID(m_currRegion->obtainIDForOperation());
    m_currBasicBlock->addOp(std::move(op));
    return opPtr;
  }

  int64_t getCurBasicBlockID() {
    if (m_currBasicBlock == nullptr) {
      return -1;
    }

    return static_cast<int64_t>(m_currBasicBlock->getID());
  }

  const BasicBlock& getCurBasicBlock() const {
    if (m_currBasicBlock == nullptr) {
      throw IrisException("No basic block in building!");
    }
    return *m_currBasicBlock;
  }

  BasicBlock& getCurBasicBlock() {
    if (m_currBasicBlock == nullptr) {
      throw IrisException("No basic block in building!");
    }
    return *m_currBasicBlock;
  }

  void dropBasicBlock() {
    if (m_currBasicBlock == nullptr) {
      throw IrisException("No basic block is building!");
    }
    delete m_currBasicBlock;
  }

  BasicBlock& finalizeBasicBlock() {
    if (m_currBasicBlock == nullptr) {
      throw IrisException("No basic block in building!");
    }

    auto& bb = *m_currBasicBlock;
    m_currRegion->addBasicBlock(
      std::unique_ptr<BasicBlock>(std::exchange(m_currBasicBlock, nullptr)));
    return bb;
  }

private:
  Region* m_currRegion = nullptr;
  BasicBlock* m_currBasicBlock = nullptr;
};

} // namespace iris

#endif // INCLUDE_BUILDER
