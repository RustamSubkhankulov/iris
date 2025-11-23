#ifndef INCLUDE_OPT_GENERIC_PATTERN_HPP
#define INCLUDE_OPT_GENERIC_PATTERN_HPP

#include <graph/region.hpp>
#include <ops/generic/operation.hpp>
#include <opt/generic/pass.hpp>

namespace iris {
namespace opt {

class PatternRewriter;

class Pattern {
public:
  virtual ~Pattern() = default;

  /// @return true if IR was changed
  virtual bool matchAndRewrite(Operation& op,
                               PatternRewriter& rewriter) const = 0;
};

class PatternRewriter {
public:
  explicit PatternRewriter(Region& region)
    : m_region(region) {}

  const Region& getRegion() const {
    return m_region;
  }

  Region& getRegion() {
    return m_region;
  }

  /// @brief Erase operation from its parent block
  void eraseOp(Operation& op);

  /// @brief Replace operation with new one in-place, preserving ID and users
  void replaceOpWith(Operation& op, std::unique_ptr<Operation> newOp);

  /// @brief Convenience utility for replacing with newly constructed operation
  template <typename OpTy, typename... Args>
  OpTy* replaceOpWithNew(Operation& op, Args&&... args) {
    auto newOp = std::make_unique<OpTy>(std::forward<Args>(args)...);
    auto* ptr = newOp.get();
    replaceOpWith(op, std::move(newOp));
    return ptr;
  }

private:
  Region& m_region;
};

class PatternPass : public Pass {
public:
  template <typename PatternTy, typename... Args>
  void addPattern(Args&&... args) {
    m_patterns.push_back(
      std::make_unique<PatternTy>(std::forward<Args>(args)...));
  }

  bool run(Region& region) override;

protected:
  std::vector<std::unique_ptr<Pattern>> m_patterns;
};

} // namespace opt
} // namespace iris

#endif // INCLUDE_OPT_GENERIC_PATTERN_HPP
