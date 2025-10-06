#include "attributes.hpp"
#include "ops/dialects/arith/ops.hpp"
#include "ops/dialects/ctrlflow/ops.hpp"
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

  bb1.linkSucc(bb2);

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

  bb1.linkSucc(bbT, true);
  bb1.linkSucc(bbF, false);

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

  bb1->linkSucc(*bb2);

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

  bb1->linkSucc(*bb2);

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
  bb1->linkSucc(bb2);

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
  bb1->linkSucc(bb2, false);

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

  bb1->linkSucc(*bb2, false);

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

  bb1->linkSucc(*bb2);

  std::string msg;
  bool vres = bb1->verify(msg, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("is empty"));
}

TEST(BASIC_BLOCK, EXFAIL_TWO_SUCC_IDENTIVAL) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);
  region.addBasicBlock(std::make_unique<BasicBlock>(2));
  auto* bb2 = region.getBasicBlockByID(2);

  bb1->linkSucc(*bb2, true);
  bb1->linkSucc(*bb2, false);
  bb1->addOp(std::make_unique<ctrlflow::JumpOp>());

  std::string msg;
  bool vres = bb1->verify(msg, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("has two identical successors"));
}

TEST(BASIC_BLOCK, EXFAIL_TWO_SUCC_NO_JUMPC) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);
  region.addBasicBlock(std::make_unique<BasicBlock>(2));
  auto* bb2 = region.getBasicBlockByID(2);
  region.addBasicBlock(std::make_unique<BasicBlock>(3));
  auto* bb3 = region.getBasicBlockByID(3);

  bb1->linkSucc(*bb2, true);
  bb1->linkSucc(*bb3, false);
  bb1->addOp(std::make_unique<ctrlflow::JumpOp>());

  std::string msg;
  bool vres = bb1->verify(msg, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains(
    "has two successors, but conditional jump at the end is missing"));
}

TEST(BASIC_BLOCK, EXFAIL_ONE_SUCC_WITH_JUMPC) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);
  region.addBasicBlock(std::make_unique<BasicBlock>(2));
  auto* bb2 = region.getBasicBlockByID(2);

  bb1->linkSucc(*bb2);

  auto val = std::make_unique<arith::ConstantOp>(makeConstAttribute(true));
  auto jmp = std::make_unique<ctrlflow::JumpcOp>(val.get());

  bb1->addOp(std::move(val));
  bb1->addOp(std::move(jmp));

  std::string msg;
  bool vres = bb1->verify(msg, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(
    msg.contains("has single successor, but has conditional jump at the end"));
}

TEST(BASIC_BLOCK, EXFAIL_TERMINATOR_INSIDE) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);

  auto vl1 = std::make_unique<arith::ConstantOp>(makeConstAttribute(true));
  auto jmp = std::make_unique<ctrlflow::JumpcOp>(vl1.get());
  auto vl2 = std::make_unique<arith::ConstantOp>(makeConstAttribute(true));

  bb1->addOp(std::move(vl1));
  bb1->addOp(std::move(jmp));
  bb1->addOp(std::move(vl2));

  std::string msg;
  bool vres = bb1->verify(msg, true, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(
    msg.contains("terminator operation is not the last one in the block"));
}
