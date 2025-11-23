#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(DCE, BASIC) {
  IRBuilder builder;
  builder.startNewRegion("foo");

  builder.startNewBasicBlock();

  auto a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::UI32);
  auto a1 = builder.createAndAddOp<builtin::ParamOp>(DataType::UI32);

  auto a2 = builder.createAndAddOp<builtin::ParamOp>(DataType::UI32);
  auto a3 = builder.createAndAddOp<builtin::ParamOp>(DataType::UI32);

  auto v4 = builder.createAndAddOp<arith::MulOp>(a0, a1);
  /* v5 */ builder.createAndAddOp<arith::AddOp>(a2, a3);

  auto n6 = builder.createAndAddOp<ctrlflow::ReturnOp>(v4);
  auto& bb0 = builder.finalizeCurBasicBlock();

  auto regionPtr = builder.obtainRegion();
  ASSERT_TRUE(static_cast<bool>(regionPtr));

  ASSERT_TRUE(regionPtr->setStartBasicBlock(&bb0));
  ASSERT_TRUE(regionPtr->setFinalBasicBlock(&bb0));

  std::string msg;
  bool vres;

  vres = regionPtr->verify(msg);
  ASSERT_TRUE(vres);
  ASSERT_TRUE(msg.empty());

  opt::PassManager pm;
  pm.addPass(std::make_unique<opt::common::DCEPass>());

  ASSERT_TRUE(pm.run(*regionPtr));

  vres = regionPtr->verify(msg);
  ASSERT_TRUE(vres);
  ASSERT_TRUE(msg.empty());

  auto bb = regionPtr->getStartBasicBlock();
  ASSERT_EQ(bb->getOps().size(), 6u);

  auto& ops = bb->getOps();

  EXPECT_EQ(std::next(ops.begin(), 0).get(), a0);
  EXPECT_EQ(std::next(ops.begin(), 1).get(), a1);
  EXPECT_EQ(std::next(ops.begin(), 2).get(), a2);
  EXPECT_EQ(std::next(ops.begin(), 3).get(), a3);
  EXPECT_EQ(std::next(ops.begin(), 4).get(), v4);
  EXPECT_EQ(std::next(ops.begin(), 5).get(), n6);
}
