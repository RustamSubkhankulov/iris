#ifndef INCLUDE_OPT_GENERIC_PASS_HPP
#define INCLUDE_OPT_GENERIC_PASS_HPP

#include <memory>
#include <vector>

#include <exception.hpp>

#include <graph/region.hpp>

namespace iris {
namespace opt {

class Pass {
public:
  virtual ~Pass() = default;

  /// @return true if IR was changed
  virtual bool run(Region& region) = 0;
};

class PassManager {
public:
  void addPass(std::unique_ptr<Pass> pass) {
    if (!pass) {
      throw IrisException("RegionPassManager::addPass() got empty pass!");
    }
    m_passes.push_back(std::move(pass));
  }

  bool run(Region& region) {
    bool changed = false;
    for (auto& pass : m_passes) {
      changed |= pass->run(region);
    }
    return changed;
  }

private:
  std::vector<std::unique_ptr<Pass>> m_passes;
};

} // namespace opt
} // namespace iris

#endif // INCLUDE_OPT_GENERIC_PASS_HPP
