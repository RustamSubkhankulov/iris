#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(BASIC_BLOCK, DEFAULT) {
  BasicBlock bb;

  EXPECT_FALSE(bb.hasParentRegion());
  EXPECT_EQ(bb.getPredsNum(), 0u);

  EXPECT_FALSE(bb.hasSucc(true));
  EXPECT_FALSE(bb.hasSucc(false));

  EXPECT_EQ(bb.getOps().size(), 0u);
  EXPECT_EQ(bb.getPhiOps().size(), 0u);
}

TEST(BASIC_BLOCK, ID_MANIPULATING) {
  BasicBlock bb;

  EXPECT_EQ(bb.getID(), 0);

  auto newID = static_cast<bb_id_t>(1);
  bb.setID(newID);

  EXPECT_EQ(bb.getID(), newID);
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

  ASSERT_EQ(bb2.getPredsNum(), 1u);
  ASSERT_FALSE(bb2.getPreds().empty());
  EXPECT_EQ(bb2.getPreds().front(), &bb1);

  EXPECT_TRUE(bb1.hasSucc(true));
  EXPECT_FALSE(bb1.hasSucc(false));

  EXPECT_EQ(bb1.getSuccID(true), 2);
  EXPECT_EQ(bb1.getSuccID(false), -1);
}

TEST(BASIC_BLOCK, REPLACE) {
  BasicBlock bbPred(1);
  BasicBlock bb(2);
  BasicBlock bbSuccT(3);
  BasicBlock bbSuccF(4);

  bbPred.linkSucc(&bb);
  bb.linkSucc(&bbSuccT, true);
  bb.linkSucc(&bbSuccF, false);

  BasicBlock newBB(5);
  bb.replaceWith(newBB);

  EXPECT_TRUE(bbPred.hasSucc());
  EXPECT_EQ(bbPred.getSucc(), &newBB);

  EXPECT_EQ(bb.getPredsNum(), 0u);

  ASSERT_EQ(newBB.getPredsNum(), 1u);
  ASSERT_FALSE(newBB.getPreds().empty());
  EXPECT_EQ(newBB.getPreds().front(), &bbPred);

  EXPECT_FALSE(bb.hasSucc(true));
  EXPECT_FALSE(bb.hasSucc(false));

  EXPECT_EQ(bb.getSucc(true), nullptr);
  EXPECT_EQ(bb.getSucc(false), nullptr);

  EXPECT_TRUE(newBB.hasSucc(true));
  EXPECT_TRUE(newBB.hasSucc(false));

  EXPECT_EQ(newBB.getSucc(true), &bbSuccT);
  EXPECT_EQ(newBB.getSucc(false), &bbSuccF);

  ASSERT_EQ(bbSuccT.getPredsNum(), 1u);
  ASSERT_FALSE(bbSuccT.getPreds().empty());
  ASSERT_EQ(bbSuccF.getPredsNum(), 1);
  ASSERT_FALSE(bbSuccF.getPreds().empty());

  EXPECT_EQ(bbSuccT.getPreds().front(), &newBB);
  EXPECT_EQ(bbSuccF.getPreds().front(), &newBB);
}

TEST(BASIC_BLOCK, UNLINK) {
  BasicBlock bbPred(1);
  BasicBlock bb(2);
  BasicBlock bbSuccT(3);
  BasicBlock bbSuccF(4);

  bbPred.linkSucc(&bb);
  bb.linkSucc(&bbSuccT, true);
  bb.linkSucc(&bbSuccF, false);

  bb.unlink();

  EXPECT_FALSE(bbPred.hasSucc());
  EXPECT_EQ(bbPred.getSuccID(), -1);

  EXPECT_FALSE(bb.hasSucc(true));
  EXPECT_EQ(bb.getSuccID(true), -1);

  EXPECT_FALSE(bb.hasSucc(false));
  EXPECT_EQ(bb.getSuccID(false), -1);

  EXPECT_EQ(bb.getPredsNum(), 0u);
  EXPECT_EQ(bbSuccT.getPredsNum(), 0u);
  EXPECT_EQ(bbSuccF.getPredsNum(), 0u);
}

TEST(BASIC_BLOCK, DELETION) {
  auto* bb0 = new BasicBlock(0);
  ASSERT_NE(bb0, nullptr);

  BasicBlock bbP1(1);
  BasicBlock bbP2(2);

  BasicBlock scT(3);
  BasicBlock scF(4);

  bbP1.linkSucc(bb0);
  bbP2.linkSucc(bb0);

  bb0->linkSucc(&scT, true);
  bb0->linkSucc(&scF, false);

  ASSERT_EQ(scT.getPredsNum(), 1u);
  ASSERT_EQ(scF.getPredsNum(), 1u);
  ASSERT_TRUE(bbP1.hasSucc());
  ASSERT_TRUE(bbP2.hasSucc());

  bb0->unlink();
  delete bb0;

  EXPECT_EQ(scT.getPredsNum(), 0u);
  EXPECT_EQ(scF.getPredsNum(), 0u);
  EXPECT_FALSE(bbP1.hasSucc());
  EXPECT_FALSE(bbP2.hasSucc());
}

TEST(BASIC_BLOCK, CONNECTION_TRUE_AND_FALSE) {
  BasicBlock bb1(1);
  BasicBlock bbT(2);
  BasicBlock bbF(3);

  bb1.linkSucc(&bbT, true);
  bb1.linkSucc(&bbF, false);

  ASSERT_EQ(bbT.getPredsNum(), 1u);
  ASSERT_FALSE(bbT.getPreds().empty());
  EXPECT_EQ(bbT.getPreds().front(), &bb1);

  ASSERT_EQ(bbF.getPredsNum(), 1u);
  ASSERT_FALSE(bbF.getPreds().empty());
  EXPECT_EQ(bbF.getPreds().front(), &bb1);

  EXPECT_TRUE(bb1.hasSucc(true));
  EXPECT_TRUE(bb1.hasSucc(false));

  EXPECT_EQ(bb1.getSuccID(true), 2);
  EXPECT_EQ(bb1.getSuccID(false), 3);
}

TEST(BASIC_BLOCK, PHI_OPS_INSERT_BACK) {
  BasicBlock bb;

  auto phi1 = std::make_unique<ctrlflow::PhiOp>(nullptr, nullptr);
  auto* phi1Ptr = phi1.get();

  bb.insertPhiOpBack(std::move(phi1));

  ASSERT_EQ(bb.getPhiOps().size(), 1u);
  EXPECT_EQ(&bb.getPhiOps().back(), phi1Ptr);
  EXPECT_EQ(bb.getPhiOps().back().getParentBasicBlock(), &bb);

  auto phi2 = std::make_unique<ctrlflow::PhiOp>(nullptr, nullptr);
  auto* phi2Ptr = phi2.get();

  bb.insertPhiOpBack(std::move(phi2));

  ASSERT_EQ(bb.getPhiOps().size(), 2u);
  EXPECT_EQ(&bb.getPhiOps().front(), phi1Ptr);
  EXPECT_EQ(&bb.getPhiOps().back(), phi2Ptr);
  EXPECT_EQ(bb.getPhiOps().back().getParentBasicBlock(), &bb);
}

TEST(BASIC_BLOCK, PHI_OPS_ERASE_FRONT) {
  BasicBlock bb;

  auto phi1 = std::make_unique<ctrlflow::PhiOp>(nullptr, nullptr);
  auto* phi1Ptr = phi1.get();

  auto phi2 = std::make_unique<ctrlflow::PhiOp>(nullptr, nullptr);
  auto* phi2Ptr = phi2.get();

  auto phi3 = std::make_unique<ctrlflow::PhiOp>(nullptr, nullptr);
  auto* phi3Ptr = phi3.get();

  bb.insertPhiOpBack(std::move(phi1));
  bb.insertPhiOpBack(std::move(phi2));
  bb.insertPhiOpBack(std::move(phi3));

  ASSERT_EQ(bb.getPhiOps().size(), 3u);
  EXPECT_EQ(bb.getPhiOps().begin().get(), phi1Ptr);
  EXPECT_EQ(std::next(bb.getPhiOps().begin(), 1).get(), phi2Ptr);
  EXPECT_EQ(std::next(bb.getPhiOps().begin(), 2).get(), phi3Ptr);

  bb.erasePhiOp(bb.getPhiOps().begin());

  ASSERT_EQ(bb.getPhiOps().size(), 2u);
  EXPECT_EQ(bb.getPhiOps().begin().get(), phi2Ptr);
  EXPECT_EQ(std::next(bb.getPhiOps().begin(), 1).get(), phi3Ptr);
}

TEST(BASIC_BLOCK, PHI_OPS_ERASE_MID) {
  BasicBlock bb;

  auto phi1 = std::make_unique<ctrlflow::PhiOp>(nullptr, nullptr);
  auto* phi1Ptr = phi1.get();

  auto phi2 = std::make_unique<ctrlflow::PhiOp>(nullptr, nullptr);
  auto* phi2Ptr = phi2.get();

  auto phi3 = std::make_unique<ctrlflow::PhiOp>(nullptr, nullptr);
  auto* phi3Ptr = phi3.get();

  bb.insertPhiOpBack(std::move(phi1));
  bb.insertPhiOpBack(std::move(phi2));
  bb.insertPhiOpBack(std::move(phi3));

  ASSERT_EQ(bb.getPhiOps().size(), 3u);
  EXPECT_EQ(bb.getPhiOps().begin().get(), phi1Ptr);
  EXPECT_EQ(std::next(bb.getPhiOps().begin(), 1).get(), phi2Ptr);
  EXPECT_EQ(std::next(bb.getPhiOps().begin(), 2).get(), phi3Ptr);

  bb.erasePhiOp(std::next(bb.getPhiOps().begin()));

  ASSERT_EQ(bb.getPhiOps().size(), 2u);
  EXPECT_EQ(bb.getPhiOps().begin().get(), phi1Ptr);
  EXPECT_EQ(std::next(bb.getPhiOps().begin(), 1).get(), phi3Ptr);
}

TEST(BASIC_BLOCK, PHI_OPS_ERASE_BACK) {
  BasicBlock bb;

  auto phi1 = std::make_unique<ctrlflow::PhiOp>(nullptr, nullptr);
  auto* phi1Ptr = phi1.get();

  auto phi2 = std::make_unique<ctrlflow::PhiOp>(nullptr, nullptr);
  auto* phi2Ptr = phi2.get();

  auto phi3 = std::make_unique<ctrlflow::PhiOp>(nullptr, nullptr);
  auto* phi3Ptr = phi3.get();

  bb.insertPhiOpBack(std::move(phi1));
  bb.insertPhiOpBack(std::move(phi2));
  bb.insertPhiOpBack(std::move(phi3));

  ASSERT_EQ(bb.getPhiOps().size(), 3u);
  EXPECT_EQ(bb.getPhiOps().begin().get(), phi1Ptr);
  EXPECT_EQ(std::next(bb.getPhiOps().begin(), 1).get(), phi2Ptr);
  EXPECT_EQ(std::next(bb.getPhiOps().begin(), 2).get(), phi3Ptr);

  bb.erasePhiOp(std::next(bb.getPhiOps().begin(), 2));

  ASSERT_EQ(bb.getPhiOps().size(), 2u);
  EXPECT_EQ(bb.getPhiOps().begin().get(), phi1Ptr);
  EXPECT_EQ(std::next(bb.getPhiOps().begin(), 1).get(), phi2Ptr);
}

TEST(BASIC_BLOCK, PHI_OP_ERASE_LAST) {
  BasicBlock bb;

  auto phi = std::make_unique<ctrlflow::PhiOp>(nullptr, nullptr);
  auto* phiPtr = phi.get();

  bb.insertPhiOpBack(std::move(phi));

  ASSERT_EQ(bb.getPhiOps().size(), 1u);
  EXPECT_EQ(bb.getPhiOps().begin().get(), phiPtr);

  bb.erasePhiOp(bb.getPhiOps().begin());

  EXPECT_EQ(bb.getPhiOps().size(), 0u);
}

TEST(BASIC_BLOCK, PHI_OPS_REPLACE) {
  BasicBlock bb;

  builtin::ParamOp param1(DataType::F32);
  builtin::ParamOp param2(DataType::F32);

  auto phi1 = std::make_unique<ctrlflow::PhiOp>(nullptr, nullptr);
  auto phi2 = std::make_unique<ctrlflow::PhiOp>(&param1, &param2);
  auto phi3 = std::make_unique<ctrlflow::PhiOp>(nullptr, nullptr);

  auto* phi1Ptr = phi1.get();
  auto* phi2Ptr = phi2.get();
  auto* phi3Ptr = phi3.get();

  builtin::CopyOp copy(phi2Ptr);

  bb.insertPhiOpBack(std::move(phi1));
  bb.insertPhiOpBack(std::move(phi2));
  bb.insertPhiOpBack(std::move(phi3));

  builtin::ParamOp newParam1(DataType::F32);
  builtin::ParamOp newParam2(DataType::F32);

  auto newPhi = std::make_unique<ctrlflow::PhiOp>(&newParam1, &newParam2);
  auto* newPhiPtr = newPhi.get();

  bb.replacePhiOpWith(std::next(bb.getPhiOps().begin()), std::move(newPhi));

  EXPECT_FALSE(param1.hasUsers());
  EXPECT_EQ(param1.getUsersNum(), 0u);

  EXPECT_FALSE(param2.hasUsers());
  EXPECT_EQ(param2.getUsersNum(), 0u);

  EXPECT_FALSE(copy.getInputAt(0).isEmpty());
  EXPECT_EQ(copy.getInputAt(0).getDefiningOp(), newPhiPtr);

  ASSERT_EQ(bb.getPhiOps().size(), 3u);
  EXPECT_EQ(bb.getPhiOps().begin().get(), phi1Ptr);
  EXPECT_EQ(std::next(bb.getPhiOps().begin()).get(), newPhiPtr);
  EXPECT_EQ(std::next(bb.getPhiOps().begin(), 2).get(), phi3Ptr);
}

TEST(BASIC_BLOCK, OPS_INSERT_BACK) {
  BasicBlock bb;

  auto op1 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op1ptr = op1.get();

  bb.insertOpBack(std::move(op1));

  ASSERT_EQ(bb.getOps().size(), 1u);
  EXPECT_EQ(&bb.getOps().back(), op1ptr);
  EXPECT_EQ(bb.getOps().back().getParentBasicBlock(), &bb);

  auto op2 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op2ptr = op2.get();

  bb.insertOpBack(std::move(op2));

  ASSERT_EQ(bb.getOps().size(), 2u);
  EXPECT_EQ(&bb.getOps().front(), op1ptr);
  EXPECT_EQ(&bb.getOps().back(), op2ptr);
  EXPECT_EQ(bb.getOps().back().getParentBasicBlock(), &bb);
}

TEST(BASIC_BLOCK, OPS_INSERT_FRONT) {
  BasicBlock bb;

  auto op1 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op1ptr = op1.get();

  bb.insertOpFront(std::move(op1));

  ASSERT_EQ(bb.getOps().size(), 1u);
  EXPECT_EQ(&bb.getOps().front(), op1ptr);
  EXPECT_EQ(bb.getOps().front().getParentBasicBlock(), &bb);

  auto op2 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op2ptr = op2.get();

  bb.insertOpFront(std::move(op2));

  ASSERT_EQ(bb.getOps().size(), 2u);
  EXPECT_EQ(&bb.getOps().back(), op1ptr);
  EXPECT_EQ(&bb.getOps().front(), op2ptr);
  EXPECT_EQ(bb.getOps().front().getParentBasicBlock(), &bb);
}

TEST(BASIC_BLOCK, OPS_INSERT_AFTER) {
  BasicBlock bb;

  auto op1 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op1ptr = op1.get();

  auto op2 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op2ptr = op2.get();

  bb.insertOpBack(std::move(op1));
  bb.insertOpBack(std::move(op2));

  ASSERT_EQ(bb.getOps().size(), 2u);
  EXPECT_EQ(&bb.getOps().front(), op1ptr);
  EXPECT_EQ(&bb.getOps().back(), op2ptr);

  auto op3 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op3ptr = op3.get();

  bb.insertOpAfter(bb.getOps().begin(), std::move(op3));

  ASSERT_EQ(bb.getOps().size(), 3u);
  EXPECT_EQ(std::next(bb.getOps().begin(), 0).get(), op1ptr);
  EXPECT_EQ(std::next(bb.getOps().begin(), 1).get(), op3ptr);
  EXPECT_EQ(std::next(bb.getOps().begin(), 2).get(), op2ptr);

  EXPECT_EQ(&bb.getOps().front(), op1ptr);
  EXPECT_EQ(&bb.getOps().back(), op2ptr);
}

TEST(BASIC_BLOCK, OPS_INSERT_AFTER_LAST) {
  BasicBlock bb;

  auto op1 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op1ptr = op1.get();

  auto op2 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op2ptr = op2.get();

  bb.insertOpBack(std::move(op1));
  bb.insertOpBack(std::move(op2));

  ASSERT_EQ(bb.getOps().size(), 2u);
  EXPECT_EQ(&bb.getOps().front(), op1ptr);
  EXPECT_EQ(&bb.getOps().back(), op2ptr);

  auto op3 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op3ptr = op3.get();

  bb.insertOpAfter(std::next(bb.getOps().begin()), std::move(op3));

  ASSERT_EQ(bb.getOps().size(), 3u);
  EXPECT_EQ(std::next(bb.getOps().begin(), 0).get(), op1ptr);
  EXPECT_EQ(std::next(bb.getOps().begin(), 1).get(), op2ptr);
  EXPECT_EQ(std::next(bb.getOps().begin(), 2).get(), op3ptr);

  EXPECT_EQ(&bb.getOps().front(), op1ptr);
  EXPECT_EQ(&bb.getOps().back(), op3ptr);
}

TEST(BASIC_BLOCK, OPS_INSERT_BEFORE) {
  BasicBlock bb;

  auto op1 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op1ptr = op1.get();

  auto op2 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op2ptr = op2.get();

  bb.insertOpBack(std::move(op1));
  bb.insertOpBack(std::move(op2));

  ASSERT_EQ(bb.getOps().size(), 2u);
  EXPECT_EQ(&bb.getOps().front(), op1ptr);
  EXPECT_EQ(&bb.getOps().back(), op2ptr);

  auto op3 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op3ptr = op3.get();

  bb.insertOpBefore(std::next(bb.getOps().begin()), std::move(op3));

  ASSERT_EQ(bb.getOps().size(), 3u);
  EXPECT_EQ(std::next(bb.getOps().begin(), 0).get(), op1ptr);
  EXPECT_EQ(std::next(bb.getOps().begin(), 1).get(), op3ptr);
  EXPECT_EQ(std::next(bb.getOps().begin(), 2).get(), op2ptr);

  EXPECT_EQ(&bb.getOps().front(), op1ptr);
  EXPECT_EQ(&bb.getOps().back(), op2ptr);
}

TEST(BASIC_BLOCK, OPS_INSERT_BEFORE_FIRST) {
  BasicBlock bb;

  auto op1 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op1ptr = op1.get();

  auto op2 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op2ptr = op2.get();

  bb.insertOpBack(std::move(op1));
  bb.insertOpBack(std::move(op2));

  ASSERT_EQ(bb.getOps().size(), 2u);
  EXPECT_EQ(&bb.getOps().front(), op1ptr);
  EXPECT_EQ(&bb.getOps().back(), op2ptr);

  auto op3 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op3ptr = op3.get();

  bb.insertOpBefore(bb.getOps().begin(), std::move(op3));

  ASSERT_EQ(bb.getOps().size(), 3u);
  EXPECT_EQ(std::next(bb.getOps().begin(), 0).get(), op3ptr);
  EXPECT_EQ(std::next(bb.getOps().begin(), 1).get(), op1ptr);
  EXPECT_EQ(std::next(bb.getOps().begin(), 2).get(), op2ptr);

  EXPECT_EQ(&bb.getOps().front(), op3ptr);
  EXPECT_EQ(&bb.getOps().back(), op2ptr);
}

TEST(BASIC_BLOCK, OPS_ERASE_FRONT) {
  BasicBlock bb;

  auto op1 = std::make_unique<builtin::ParamOp>(DataType::F32);

  auto op2 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op2ptr = op2.get();

  auto op3 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op3ptr = op3.get();

  bb.insertOpBack(std::move(op1));
  bb.insertOpBack(std::move(op2));
  bb.insertOpBack(std::move(op3));

  bb.eraseOp(bb.getOps().begin());

  ASSERT_EQ(bb.getOps().size(), 2u);

  EXPECT_EQ(std::next(bb.getOps().begin(), 0).get(), op2ptr);
  EXPECT_EQ(std::next(bb.getOps().begin(), 1).get(), op3ptr);

  EXPECT_EQ(&bb.getOps().front(), op2ptr);
  EXPECT_EQ(&bb.getOps().back(), op3ptr);
}

TEST(BASIC_BLOCK, OPS_ERASE_MID) {
  BasicBlock bb;

  auto op1 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op1ptr = op1.get();

  auto op2 = std::make_unique<builtin::ParamOp>(DataType::F32);

  auto op3 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op3ptr = op3.get();

  bb.insertOpBack(std::move(op1));
  bb.insertOpBack(std::move(op2));
  bb.insertOpBack(std::move(op3));

  bb.eraseOp(std::next(bb.getOps().begin()));

  ASSERT_EQ(bb.getOps().size(), 2u);

  EXPECT_EQ(std::next(bb.getOps().begin(), 0).get(), op1ptr);
  EXPECT_EQ(std::next(bb.getOps().begin(), 1).get(), op3ptr);

  EXPECT_EQ(&bb.getOps().front(), op1ptr);
  EXPECT_EQ(&bb.getOps().back(), op3ptr);
}

TEST(BASIC_BLOCK, OPS_ERASE_BACK) {
  BasicBlock bb;

  auto op1 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op1ptr = op1.get();

  auto op2 = std::make_unique<builtin::ParamOp>(DataType::F32);
  auto* op2ptr = op2.get();

  auto op3 = std::make_unique<builtin::ParamOp>(DataType::F32);

  bb.insertOpBack(std::move(op1));
  bb.insertOpBack(std::move(op2));
  bb.insertOpBack(std::move(op3));

  bb.eraseOp(std::next(bb.getOps().begin(), 2));

  ASSERT_EQ(bb.getOps().size(), 2u);

  EXPECT_EQ(std::next(bb.getOps().begin(), 0).get(), op1ptr);
  EXPECT_EQ(std::next(bb.getOps().begin(), 1).get(), op2ptr);

  EXPECT_EQ(&bb.getOps().front(), op1ptr);
  EXPECT_EQ(&bb.getOps().back(), op2ptr);
}

TEST(BASIC_BLOCK, OPS_REPLACE) {
  BasicBlock bb;

  builtin::ParamOp param1(DataType::F32);
  builtin::ParamOp param2(DataType::F32);

  auto op1 = std::make_unique<arith::AddOp>(nullptr, nullptr);
  auto add = std::make_unique<arith::AddOp>(&param1, &param2);
  auto op3 = std::make_unique<arith::AddOp>(nullptr, nullptr);

  auto* op1ptr = op1.get();
  auto* addptr = add.get();
  auto* op3ptr = op3.get();

  builtin::CopyOp copy(addptr);

  bb.insertOpBack(std::move(op1));
  bb.insertOpBack(std::move(add));
  bb.insertOpBack(std::move(op3));

  builtin::ParamOp newParam1(DataType::F32);
  builtin::ParamOp newParam2(DataType::F32);

  auto newAdd = std::make_unique<arith::AddOp>(&newParam1, &newParam2);
  auto* newAddPtr = newAdd.get();

  bb.replaceOpWith(std::next(bb.getOps().begin()), std::move(newAdd));

  EXPECT_FALSE(param1.hasUsers());
  EXPECT_EQ(param1.getUsersNum(), 0u);

  EXPECT_FALSE(param2.hasUsers());
  EXPECT_EQ(param2.getUsersNum(), 0u);

  EXPECT_FALSE(copy.getInputAt(0).isEmpty());
  EXPECT_EQ(copy.getInputAt(0).getDefiningOp(), newAddPtr);

  ASSERT_EQ(bb.getOps().size(), 3u);
  EXPECT_EQ(std::next(bb.getOps().begin(), 0).get(), op1ptr);
  EXPECT_EQ(std::next(bb.getOps().begin(), 1).get(), newAddPtr);
  EXPECT_EQ(std::next(bb.getOps().begin(), 2).get(), op3ptr);
}

TEST(BASIC_BLOCK, EXFAIL_ORPHAN) {
  BasicBlock bb;
  std::string msg;
  bool vres = bb.verify(msg);

  ASSERT_FALSE(vres);
  ASSERT_FALSE(msg.empty());
  EXPECT_TRUE(msg.contains("has no parent region"));
}

TEST(BASIC_BLOCK, EXFAIL_START_WITH_PREDECESSORS) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);
  region.addBasicBlock(std::make_unique<BasicBlock>(2));
  auto* bb2 = region.getBasicBlockByID(2);

  ASSERT_NE(bb1, nullptr);
  ASSERT_NE(bb2, nullptr);

  bb1->linkSucc(bb2);

  std::string msg;
  bool vres = bb2->verify(msg, true);

  ASSERT_FALSE(vres);
  ASSERT_FALSE(msg.empty());
  EXPECT_TRUE(msg.contains("is starting bb, but has predecessor"));
}

TEST(BASIC_BLOCK, EXFAIL_FINAL_WITH_SUCCESSORS) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);
  region.addBasicBlock(std::make_unique<BasicBlock>(2));
  auto* bb2 = region.getBasicBlockByID(2);

  ASSERT_NE(bb1, nullptr);
  ASSERT_NE(bb2, nullptr);

  bb1->linkSucc(bb2);

  std::string msg;
  bool vres = bb1->verify(msg, false, true);

  ASSERT_FALSE(vres);
  ASSERT_FALSE(msg.empty());
  EXPECT_TRUE(msg.contains("is final bb, but has successors"));
}

TEST(BASIC_BLOCK, EXFAIL_TRUE_SUCCESSOR_NOT_IN_REGION) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);

  ASSERT_NE(bb1, nullptr);

  BasicBlock bb2(2);
  bb1->linkSucc(&bb2);

  std::string msg;
  bool vres = bb1->verify(msg, true);

  ASSERT_FALSE(vres);
  ASSERT_FALSE(msg.empty());
  EXPECT_TRUE(msg.contains("true successor is not in the region"));
}

TEST(BASIC_BLOCK, EXFAIL_FALSE_SUCCESSOR_NOT_IN_REGION) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);

  ASSERT_NE(bb1, nullptr);

  BasicBlock bb2(2);
  bb1->linkSucc(&bb2, false);

  std::string msg;
  bool vres = bb1->verify(msg, true);

  ASSERT_FALSE(vres);
  ASSERT_FALSE(msg.empty());
  EXPECT_TRUE(msg.contains("false successor is not in the region"));
}

TEST(BASIC_BLOCK, EXFAIL_FALSE_BUT_NO_TRUE_SUCCESSORS) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);
  region.addBasicBlock(std::make_unique<BasicBlock>(2));
  auto* bb2 = region.getBasicBlockByID(2);

  ASSERT_NE(bb1, nullptr);
  ASSERT_NE(bb2, nullptr);

  bb1->linkSucc(bb2, false);

  std::string msg;
  bool vres = bb1->verify(msg, true);

  ASSERT_FALSE(vres);
  ASSERT_FALSE(msg.empty());
  EXPECT_TRUE(msg.contains(
    "has false successor specified, but true successor is missing"));
}

TEST(BASIC_BLOCK, EXFAIL_NOT_FINAL_HAS_NO_SUCCESSORS) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);

  ASSERT_NE(bb1, nullptr);

  std::string msg;
  bool vres = bb1->verify(msg, true);

  ASSERT_FALSE(vres);
  ASSERT_FALSE(msg.empty());
  EXPECT_TRUE(msg.contains("is not final, but has no successors"));
}

TEST(BASIC_BLOCK, EXFAIL_EMPTY_BB) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);
  region.addBasicBlock(std::make_unique<BasicBlock>(2));
  auto* bb2 = region.getBasicBlockByID(2);

  ASSERT_NE(bb1, nullptr);
  ASSERT_NE(bb2, nullptr);

  bb1->linkSucc(bb2);

  std::string msg;
  bool vres = bb1->verify(msg, true);

  ASSERT_FALSE(vres);
  ASSERT_FALSE(msg.empty());
  EXPECT_TRUE(msg.contains("is empty"));
}

TEST(BASIC_BLOCK, EXFAIL_FINAL_HAS_NO_RETURN) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);

  ASSERT_NE(bb1, nullptr);

  bb1->insertOpBack(std::make_unique<ctrlflow::JumpOp>());

  std::string msg;
  bool vres = bb1->verify(msg, true, true);

  ASSERT_FALSE(vres);
  ASSERT_FALSE(msg.empty());
  EXPECT_TRUE(msg.contains("last operation is not an 'ctrlflow.return'"));
}

TEST(BASIC_BLOCK, EXFAIL_TWO_SUCC_IDENTICAL) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);
  region.addBasicBlock(std::make_unique<BasicBlock>(2));
  auto* bb2 = region.getBasicBlockByID(2);

  ASSERT_NE(bb1, nullptr);
  ASSERT_NE(bb2, nullptr);

  bb1->linkSucc(bb2, true);
  bb1->linkSucc(bb2, false);
  bb1->insertOpBack(std::make_unique<ctrlflow::JumpOp>());

  std::string msg;
  bool vres = bb1->verify(msg, true);

  ASSERT_FALSE(vres);
  ASSERT_FALSE(msg.empty());
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

  ASSERT_NE(bb1, nullptr);
  ASSERT_NE(bb2, nullptr);
  ASSERT_NE(bb3, nullptr);

  bb1->linkSucc(bb2, true);
  bb1->linkSucc(bb3, false);
  bb1->insertOpBack(std::make_unique<ctrlflow::JumpOp>());

  std::string msg;
  bool vres = bb1->verify(msg, true);

  ASSERT_FALSE(vres);
  ASSERT_FALSE(msg.empty());
  EXPECT_TRUE(msg.contains(
    "has two successors, but conditional jump at the end is missing"));
}

TEST(BASIC_BLOCK, EXFAIL_ONE_SUCC_WITH_JUMPC) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);
  region.addBasicBlock(std::make_unique<BasicBlock>(2));
  auto* bb2 = region.getBasicBlockByID(2);

  ASSERT_NE(bb1, nullptr);
  ASSERT_NE(bb2, nullptr);

  bb1->linkSucc(bb2);

  auto val = std::make_unique<arith::ConstantOp>(makeConstAttribute(true));
  auto jmp = std::make_unique<ctrlflow::JumpcOp>(val.get());

  bb1->insertOpBack(std::move(val));
  bb1->insertOpBack(std::move(jmp));

  std::string msg;
  bool vres = bb1->verify(msg, true);

  ASSERT_FALSE(vres);
  ASSERT_FALSE(msg.empty());
  EXPECT_TRUE(
    msg.contains("has single successor, but has conditional jump at the end"));
}

TEST(BASIC_BLOCK, EXFAIL_TERMINATOR_INSIDE) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);

  ASSERT_NE(bb1, nullptr);

  auto vl1 = std::make_unique<arith::ConstantOp>(makeConstAttribute(true));
  auto jmp = std::make_unique<ctrlflow::JumpcOp>(vl1.get());
  auto ret = std::make_unique<ctrlflow::ReturnOp>();

  bb1->insertOpBack(std::move(vl1));
  bb1->insertOpBack(std::move(jmp));
  bb1->insertOpBack(std::move(ret));

  std::string msg;
  bool vres = bb1->verify(msg, true, true);

  ASSERT_FALSE(vres);
  ASSERT_FALSE(msg.empty());
  EXPECT_TRUE(
    msg.contains("terminator operation is not the last one in the block"));
}

TEST(BASIC_BLOCK, EXFAIL_PHI_OP_NOT_IN_ITS_LIST) {
  Region region("foo");

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  auto* bb1 = region.getBasicBlockByID(1);

  ASSERT_NE(bb1, nullptr);

  bb1->insertOpBack(std::make_unique<ctrlflow::PhiOp>(nullptr, nullptr));
  bb1->insertOpBack(std::make_unique<ctrlflow::ReturnOp>());

  std::string msg;
  bool vres = bb1->verify(msg, true, true);

  ASSERT_FALSE(vres);
  ASSERT_FALSE(msg.empty());
  EXPECT_TRUE(msg.contains("phi operation is not in the phi ops list"));
}
