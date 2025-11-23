#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(INTEGRATION, FACTORIAL_IR) {

  IRBuilder builder;
  builder.startNewRegion("factorial");

  // bb0: parameters & constants basic block
  builder.startNewBasicBlock();
  auto a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto c1 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<uint32_t>(1)));
  auto c2 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<uint32_t>(2)));
  auto& bb0 = builder.finalizeCurBasicBlock();

  // bb1: checking whether recursion is needed for computation
  builder.startNewBasicBlock();
  auto v3 =
    builder.createAndAddOp<arith::CompareOp>(a0, c2, arith::CompareOp::Pred::B);
  auto done = builder.obtainIdForBasicBlock();
  /* n4 */ builder.createAndAddOp<ctrlflow::JumpcOp>(v3);
  auto& bb1 = builder.finalizeCurBasicBlock();
  ASSERT_TRUE(bb0.linkSucc(&bb1));

  // bb3:
  builder.startNewBasicBlock();
  auto v5 = builder.createAndAddOp<arith::SubOp>(a0, c1);
  auto v6 = builder.createAndAddOp<ctrlflow::CallOp>(
    builder.getCurRegion().getName(), DataType::UINT, InputList{v5});
  auto v7 = builder.createAndAddOp<arith::MulOp>(a0, v6);
  auto& bb3 = builder.finalizeCurBasicBlock();
  ASSERT_TRUE(bb1.linkSucc(&bb3, false));

  // bb2:
  builder.startNewBasicBlock(done);
  auto v8 = builder.createAndAddOp<ctrlflow::PhiOp>(c1, v7);
  /* n9 */ builder.createAndAddOp<ctrlflow::ReturnOp>(v8);
  auto& bb2 = builder.finalizeCurBasicBlock();
  ASSERT_TRUE(bb3.linkSucc(&bb2));
  ASSERT_TRUE(bb1.linkSucc(&bb2, true));

  auto regionPtr = builder.obtainRegion();
  ASSERT_TRUE(static_cast<bool>(regionPtr));

  ASSERT_TRUE(regionPtr->setStartBasicBlock(&bb0));
  ASSERT_TRUE(regionPtr->setFinalBasicBlock(&bb2));

  std::string msg;
  bool vres;

  vres = regionPtr->verify(msg);

  ASSERT_TRUE(vres);
  ASSERT_TRUE(msg.empty());
}