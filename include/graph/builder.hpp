#ifndef INCLUDE_BUILDER
#define INCLUDE_BUILDER

#include <memory>

#include <exception.hpp>

#include <graph/basic_block.hpp>
#include <graph/region.hpp>

namespace iris {

namespace detail {
class IDProvider {
private:
  uint32_t m_curID = 0LLU;

public:
  uint32_t obtainID() {
    return m_curID++;
  }

  uint32_t getLastID() const {
    return m_curID;
  }

  void reset() {
    m_curID = 0LLU;
  }
};
} // namespace detail

class Builder final {
private:
  Region* m_currRegion = nullptr;
  BasicBlock* m_currBasicBlock = nullptr;

  detail::IDProvider m_basicBlockIDProvider;
  detail::IDProvider m_opIDProvider;

public:
  Builder() = default;

  ~Builder() {
    delete m_currRegion;
    delete m_currBasicBlock;
  }

  void reset() {
    delete m_currRegion;
    delete m_currBasicBlock;

    m_currRegion = nullptr;
    m_currBasicBlock = nullptr;

    m_basicBlockIDProvider.reset();
    m_opIDProvider.reset();
  }

  bool isRegionBuilding() const {
    return (m_currRegion != nullptr);
  }

  bool isBasicBlockBuilding() const {
    return (m_currBasicBlock != nullptr);
  }

  bool startNewRegion() {
    if (m_currRegion != nullptr) {
      // Current region is still building
      return false;
    }

    m_currRegion = new Region;
    return true;
  }

  std::unique_ptr<Region> obtainRegion() {
    if (m_currRegion == nullptr) {
      return nullptr;
    }

    m_currRegion->finalize();
    auto* tmp = m_currRegion;
    m_currRegion = nullptr;
    return std::unique_ptr<Region>(tmp);
  }

  bool startNewBasicBlock() {
    if (m_currBasicBlock != nullptr || m_currRegion == nullptr) {
      // Current basic block is still building,
      // or no region is building
      return false;
    }

    m_currBasicBlock = new BasicBlock;
    m_currBasicBlock->setID(m_basicBlockIDProvider.obtainID());
    return true;
  }

  template <typename OpTy, typename... Args>
  std::unique_ptr<Operation> createOp(Args... args) {
    return std::make_unique<OpTy>(std::forward<Args>(args)...);
  }

  bool addOp(std::unique_ptr<Operation>&& op) {
    if (m_currBasicBlock == nullptr) {
      // No basic block in building process
      return false;
    }

    m_currBasicBlock->addOp(std::move(op));
    return true;
  }

  template <typename OpTy, typename... Args>
  Operation* createAndAddOp(Args... args) {
    if (m_currBasicBlock == nullptr) {
      // No basic block in building process
      return nullptr;
    }

    auto op = std::make_unique<OpTy>(std::forward<Args>(args)...);
    auto* opPtr = op.get();
    m_currBasicBlock->addOp(std::move(op));
    return opPtr;
  }

  int64_t getCurBasicBlockID() {
    if (m_currBasicBlock == nullptr) {
      return -1;
    }

    return static_cast<int64_t>(m_basicBlockIDProvider.getLastID());
  }

  bool finalizeBasicBlock() {
    if (m_currBasicBlock == nullptr) {
      return false;
    }

    m_currRegion->addBasicBlock(std::unique_ptr<BasicBlock>(m_currBasicBlock));
    m_currBasicBlock = nullptr;
    return true;
  }
};

} // namespace iris

#endif // INCLUDE_BUILDER
