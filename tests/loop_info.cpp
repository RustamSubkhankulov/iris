#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(LOOP_INFO, DEFAULT) {
  loops::LoopInfo loopInfo;

  EXPECT_TRUE(loopInfo.isExpired());
}