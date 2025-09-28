#ifndef INCLUDE_BUILDER
#define INCLUDE_BUILDER

#include "ops/types.hpp"
#include <concepts>
#include <memory>

#include <graph/region.hpp>

namespace iris {

namespace detail {

template <std::unsigned_integral IdType>
class IDProvider final {
private:
  IdType m_curID = 0;

public:
  IdType obtainID() {
    return m_curID++;
  }

  IdType getLastID() const {
    return m_curID;
  }

  void reset() {
    m_curID = 0;
  }
};
} // namespace detail

class IRBuilder {
private:
  Region* m_currRegion = nullptr;
  BasicBlock* m_currBasicBlock = nullptr;

  detail::IDProvider<bb_id_t> m_bbIDProvider;
  detail::IDProvider<op_id_t> m_opIDProvider;

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

    m_bbIDProvider.reset();
    m_opIDProvider.reset();
  }

  bool isRegionBuilding() const {
    return (m_currRegion != nullptr);
  }

  bool isBasicBlockBuilding() const {
    return (m_currBasicBlock != nullptr);
  }

  bool startNewRegion(std::string_view name) {
    if (m_currRegion != nullptr) {
      // Current region is still building
      return false;
    }

    m_currRegion = new Region(name);
    return true;
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

  std::unique_ptr<Region> obtainRegion() {
    if (m_currRegion == nullptr) {
      return nullptr;
    }

    m_currRegion->finalize();
    auto* tmp = m_currRegion;
    m_currRegion = nullptr;
    return std::unique_ptr<Region>(tmp);
  }

  bb_id_t obtainIdForBasicBlock() {
    return m_bbIDProvider.obtainID();
  }

  bool startNewBasicBlock() {
    if (m_currBasicBlock != nullptr || m_currRegion == nullptr) {
      // Current basic block is still building,
      // or no region is building
      return false;
    }

    m_currBasicBlock = new BasicBlock;
    m_currBasicBlock->setID(m_bbIDProvider.obtainID());
    return true;
  }

  bool startNewBasicBlock(bb_id_t id) {
    if (m_currBasicBlock != nullptr || m_currRegion == nullptr) {
      // Current basic block is still building,
      // or no region is building
      return false;
    }

    m_currBasicBlock = new BasicBlock;
    m_currBasicBlock->setID(id);
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

    op->setID(m_opIDProvider.obtainID());
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

    op->setID(m_opIDProvider.obtainID());
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
      throw IrisException("No basic block in process currently");
    }
    return *m_currBasicBlock;
  }

  BasicBlock& getCurBasicBlock() {
    if (m_currBasicBlock == nullptr) {
      throw IrisException("No basic block in process currently");
    }
    return *m_currBasicBlock;
  }

  BasicBlock& finalizeBasicBlock() {
    if (m_currBasicBlock == nullptr) {
      throw IrisException("No basic block in process currently");
    }

    auto& bb = *m_currBasicBlock;
    m_currRegion->addBasicBlock(
      std::unique_ptr<BasicBlock>(std::exchange(m_currBasicBlock, nullptr)));
    return bb;
  }
};

} // namespace iris

#endif // INCLUDE_BUILDER
