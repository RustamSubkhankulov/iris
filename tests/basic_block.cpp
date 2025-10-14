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

TEST(BASIC_BLOCK, PARENT_REGION) {
  BasicBlock bb;
  Region region("foo");

  bb.setParentRegion(&region);

  EXPECT_TRUE(bb.hasParentRegion());
  EXPECT_EQ(bb.getParentRegion(), &region);

  bb.clearParentRegion();

  EXPECT_FALSE(bb.hasParentRegion());
  EXPECT_EQ(bb.getParentRegion(), nullptr);
}

TEST(BASIC_BLOCK, CONNECTION_TRUE) {
  BasicBlock bb1(1);
  BasicBlock bb2(2);

  bb1.linkSucc(&bb2);

  EXPECT_EQ(bb2.getPredsNum(), 1);
  EXPECT_EQ(bb2.getPreds().front(), &bb1);

  EXPECT_TRUE(bb1.hasSucc(true));
  EXPECT_FALSE(bb1.hasSucc(false));

  EXPECT_EQ(bb1.getSuccID(true), 2);
  EXPECT_EQ(bb1.getSuccID(false), -1);
}

TEST(BASIC_BLOCK, UNLINK) {
  BasicBlock bbPred(1);
  BasicBlock bb(2);
  BasicBlock bbSuccT(3);
  BasicBlock bbSuccF(4);

  bbPred.linkSucc(&bb);
  bb.linkSucc(&bbSuccT, true);
  bb.linkSucc(&bbSuccF, false);

  EXPECT_TRUE(bbPred.hasSucc());
  EXPECT_EQ(bbPred.getSuccID(), 2);

  EXPECT_TRUE(bb.hasSucc(true));
  EXPECT_EQ(bb.getSuccID(true), 3);

  EXPECT_TRUE(bb.hasSucc(false));
  EXPECT_EQ(bb.getSuccID(false), 4);

  EXPECT_EQ(bb.getPredsNum(), 1);
  EXPECT_EQ(bb.getPreds().front(), &bbPred);

  EXPECT_EQ(bbSuccT.getPredsNum(), 1);
  EXPECT_EQ(bbSuccT.getPreds().front(), &bb);

  EXPECT_EQ(bbSuccF.getPredsNum(), 1);
  EXPECT_EQ(bbSuccF.getPreds().front(), &bb);

  bb.unlink();

  EXPECT_FALSE(bbPred.hasSucc());
  EXPECT_EQ(bbPred.getSuccID(), -1);

  EXPECT_FALSE(bb.hasSucc(true));
  EXPECT_EQ(bb.getSuccID(true), -1);

  EXPECT_FALSE(bb.hasSucc(false));
  EXPECT_EQ(bb.getSuccID(false), -1);

  EXPECT_EQ(bb.getPredsNum(), 0);
  EXPECT_EQ(bbSuccT.getPredsNum(), 0);
  EXPECT_EQ(bbSuccF.getPredsNum(), 0);
}

TEST(BASIC_BLOCK, DELETION) {
  auto bb0 = new BasicBlock(0);

  BasicBlock bbP1(1);
  BasicBlock bbP2(2);

  BasicBlock scT(3);
  BasicBlock scF(4);

  bbP1.linkSucc(bb0);
  bbP2.linkSucc(bb0);

  bb0->linkSucc(&scT, true);
  bb0->linkSucc(&scF, false);

  EXPECT_EQ(scT.getPredsNum(), 1);
  EXPECT_EQ(scF.getPredsNum(), 1);
  EXPECT_TRUE(bbP1.hasSucc());
  EXPECT_TRUE(bbP2.hasSucc());

  bb0->unlink();
  delete bb0;

  EXPECT_EQ(scT.getPredsNum(), 0);
  EXPECT_EQ(scF.getPredsNum(), 0);
  EXPECT_FALSE(bbP1.hasSucc());
  EXPECT_FALSE(bbP2.hasSucc());
}

TEST(BASIC_BLOCK, CONNECTION_TRUE_AND_FALSE) {
  BasicBlock bb1(1);
  BasicBlock bbT(2);
  BasicBlock bbF(3);

  bb1.linkSucc(&bbT, true);
  bb1.linkSucc(&bbF, false);

  EXPECT_EQ(bbT.getPredsNum(), 1);
  EXPECT_EQ(bbT.getPreds().front(), &bb1);

  EXPECT_EQ(bbF.getPredsNum(), 1);
  EXPECT_EQ(bbF.getPreds().front(), &bb1);

  EXPECT_TRUE(bb1.hasSucc(true));
  EXPECT_TRUE(bb1.hasSucc(false));

  EXPECT_EQ(bb1.getSuccID(true), 2);
  EXPECT_EQ(bb1.getSuccID(false), 3);
}

// TODO: to region tests
TEST(BASIC_BLOCK, EXFAIL_ORPHAN) {
  BasicBlock bb;
  std::string msg;
  bool vres = bb.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("has no parent region"));
}

// TODO: to region tests
TEST(BASIC_BLOCK, EXFAIL_START_WITH_PREDECESSORS) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);
  region.addBasicBlock(std::make_unique<BasicBlock>(2));
  auto* bb2 = region.getBasicBlockByID(2);

  bb1->linkSucc(bb2);

  std::string msg;
  bool vres = bb2->verify(msg, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("is starting bb, but has predecessor"));
}

// TODO: to region tests
TEST(BASIC_BLOCK, EXFAIL_FINAL_WITH_SUCCESSORS) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);
  region.addBasicBlock(std::make_unique<BasicBlock>(2));
  auto* bb2 = region.getBasicBlockByID(2);

  bb1->linkSucc(bb2);

  std::string msg;
  bool vres = bb1->verify(msg, false, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("is final bb, but has successors"));
}

// TODO: to region tests
TEST(BASIC_BLOCK, EXFAIL_TRUE_SUCCESSOR_NOT_IN_REGION) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);

  BasicBlock bb2(2);
  bb1->linkSucc(&bb2);

  std::string msg;
  bool vres = bb1->verify(msg, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("true successor is not in the region"));
}

// TODO: to region tests
TEST(BASIC_BLOCK, EXFAIL_FALSE_SUCCESSOR_NOT_IN_REGION) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);

  BasicBlock bb2(2);
  bb1->linkSucc(&bb2, false);

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

  bb1->linkSucc(bb2, false);

  std::string msg;
  bool vres = bb1->verify(msg, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains(
    "has false successor specified, but true successor is missing"));
}

// TODO: to region tests
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

  bb1->linkSucc(bb2);

  std::string msg;
  bool vres = bb1->verify(msg, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("is empty"));
}

// TODO: to region tests
TEST(BASIC_BLOCK, EXFAIL_FINAL_HAS_NO_RETURN) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);

  bb1->insertOpBack(std::make_unique<ctrlflow::JumpOp>());

  std::string msg;
  bool vres = bb1->verify(msg, true, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("last operation is not an \'ctrlflow.return\'"));
}

// TODO: to region tests
TEST(BASIC_BLOCK, EXFAIL_TWO_SUCC_IDENTICAL) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);
  region.addBasicBlock(std::make_unique<BasicBlock>(2));
  auto* bb2 = region.getBasicBlockByID(2);

  bb1->linkSucc(bb2, true);
  bb1->linkSucc(bb2, false);
  bb1->insertOpBack(std::make_unique<ctrlflow::JumpOp>());

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

  bb1->linkSucc(bb2, true);
  bb1->linkSucc(bb3, false);
  bb1->insertOpBack(std::make_unique<ctrlflow::JumpOp>());

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

  bb1->linkSucc(bb2);

  auto val = std::make_unique<arith::ConstantOp>(makeConstAttribute(true));
  auto jmp = std::make_unique<ctrlflow::JumpcOp>(val.get());

  bb1->insertOpBack(std::move(val));
  bb1->insertOpBack(std::move(jmp));

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
  auto ret = std::make_unique<ctrlflow::ReturnOp>();

  bb1->insertOpBack(std::move(vl1));
  bb1->insertOpBack(std::move(jmp));
  bb1->insertOpBack(std::move(ret));

  std::string msg;
  bool vres = bb1->verify(msg, true, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(
    msg.contains("terminator operation is not the last one in the block"));
}

TEST(BASIC_BLOCK, EXFAIL_PHI_OP_NOT_IN_ITS_LIST) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);

  bb1->insertOpBack(std::make_unique<ctrlflow::PhiOp>(nullptr, nullptr));
  bb1->insertOpBack(std::make_unique<ctrlflow::ReturnOp>());

  std::string msg;
  bool vres = bb1->verify(msg, true, true);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("phi operation is not in the phi ops list"));
}
