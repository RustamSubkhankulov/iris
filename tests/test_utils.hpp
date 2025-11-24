#ifndef TEST_UTILS_HPP
#define TEST_UTILS_HPP

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include <iris.hpp>

namespace iris::test {

inline void verifyRegion(Region& region) {
  std::string msg;
  const bool ok = region.verify(msg);
  ASSERT_TRUE(ok) << msg;
  ASSERT_TRUE(msg.empty());
}

template <typename PassTy>
bool runSinglePass(Region& region) {
  opt::PassManager pm;
  pm.addPass(std::make_unique<PassTy>());
  return pm.run(region);
}

} // namespace iris::test

#endif // TEST_UTILS_HPP
