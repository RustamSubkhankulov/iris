#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(BASIC_BLOCK, DEFAULT) {
  BasicBlock bb;

  EXPECT_FALSE(bb.hasParentRegion());
  EXPECT_EQ(bb.getPredsNum(), 0);

  EXPECT_FALSE(bb.hasSucc(true));
  EXPECT_FALSE(bb.hasSucc(false));
}

TEST(BASIC_BLOCK, CONNECTION_P2P) {
  BasicBlock bb1(1);
  BasicBlock bb2(2);

  bb1.setSucc(bb2);

  EXPECT_EQ(bb2.getPredsNum(), 1);
  EXPECT_EQ(bb2.getPreds().front(), 1);

  EXPECT_TRUE(bb1.hasSucc(true));
  EXPECT_FALSE(bb1.hasSucc(false));

  EXPECT_EQ(bb1.getSuccID(true), 2);
  EXPECT_EQ(bb1.getSuccID(false), -1);
}

TEST(BASIC_BLOCK, CONNECTION_TRUE_AND_FALSE) {
  BasicBlock bb1(1);
  BasicBlock bbT(2);
  BasicBlock bbF(3);

  bb1.setSucc(bbT, true);
  bb1.setSucc(bbF, false);

  EXPECT_EQ(bbT.getPredsNum(), 1);
  EXPECT_EQ(bbT.getPreds().front(), 1);

  EXPECT_EQ(bbF.getPredsNum(), 1);
  EXPECT_EQ(bbF.getPreds().front(), 1);

  EXPECT_TRUE(bb1.hasSucc(true));
  EXPECT_TRUE(bb1.hasSucc(false));

  EXPECT_EQ(bb1.getSuccID(true), 2);
  EXPECT_EQ(bb1.getSuccID(false), 3);
}

TEST(BASIC_BLOCK, EXFAIL_ORPHAN) {
  BasicBlock bb;
  std::string msg;
  bool vres = bb.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("has no parent region"));
}

TEST(BASIC_BLOCK, EXFAIL_START_WITH_PREDECESSORS) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);
  region.addBasicBlock(std::make_unique<BasicBlock>(2));
  auto* bb2 = region.getBasicBlockByID(2);

  bb1->setSucc(*bb2);

  std::string msg;
  bool vres = bb2->verify(msg, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("is starting bb, but has predecessor"));
}

TEST(BASIC_BLOCK, EXFAIL_FINAL_WITH_SUCCESSORS) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);
  region.addBasicBlock(std::make_unique<BasicBlock>(2));
  auto* bb2 = region.getBasicBlockByID(2);

  bb1->setSucc(*bb2);

  std::string msg;
  bool vres = bb1->verify(msg, false, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("is final bb, but has successors"));
}

TEST(BASIC_BLOCK, EXFAIL_TRUE_SUCCESSOR_NOT_IN_REGION) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);

  BasicBlock bb2(2);
  bb1->setSucc(bb2);

  std::string msg;
  bool vres = bb1->verify(msg, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("true successor is not in the region"));
}

TEST(BASIC_BLOCK, EXFAIL_FALSE_SUCCESSOR_NOT_IN_REGION) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);

  BasicBlock bb2(2);
  bb1->setSucc(bb2, false);

  std::string msg;
  bool vres = bb1->verify(msg, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("false successor is not in the region"));
}

TEST(BASIC_BLOCK, EXFAIL_FALSE_BUT_NO_TRUE_SUCCESSORS) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);
  region.addBasicBlock(std::make_unique<BasicBlock>(2));
  auto* bb2 = region.getBasicBlockByID(2);

  bb1->setSucc(*bb2, false);

  std::string msg;
  bool vres = bb1->verify(msg, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains(
    "has false successor specified, but true successor is missing"));
}

TEST(BASIC_BLOCK, EXFAIL_NOT_FINAL_HAS_NO_SUCCESSORS) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);

  std::string msg;
  bool vres = bb1->verify(msg, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("is not final, but has no successors"));
}

TEST(BASIC_BLOCK, EXFAIL_EMPTY_BB) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);
  region.addBasicBlock(std::make_unique<BasicBlock>(2));
  auto* bb2 = region.getBasicBlockByID(2);

  bb1->setSucc(*bb2);

  std::string msg;
  bool vres = bb1->verify(msg, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("is empty"));
}