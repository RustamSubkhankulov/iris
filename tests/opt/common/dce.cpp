#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

#include <test_utils.hpp>
using iris::test::runSinglePass;
using iris::test::verifyRegion;

TEST(DCE, SIMPLE_DEAD_CHAIN) {
  IRBuilder builder;
  builder.startNewRegion("dce_simple_dead_chain");
  builder.startNewBasicBlock();

  auto* a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);

  auto* c1 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{10}));
  auto* c2 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{20}));

  /* v3 = */ builder.createAndAddOp<arith::AddOp>(c1, c2);

  builder.createAndAddOp<ctrlflow::ReturnOp>(a0);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::common::DCEPass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GE(ops.size(), 2u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_EQ(ret->getInputsNum(), 1u);

  bool hasParam = false;

  // All dead computations (add + its constants) must be removed.
  for (const auto& op : ops) {
    if (dynamic_cast<const builtin::ParamOp*>(op.get()) != nullptr) {
      hasParam = true;
    }

    EXPECT_EQ(dynamic_cast<const arith::AddOp*>(op.get()), nullptr);
    EXPECT_EQ(dynamic_cast<const arith::ConstantOp*>(op.get()), nullptr);
  }

  EXPECT_TRUE(hasParam);
}

TEST(DCE, BASIC_FROM_EXAMPLE) {
  IRBuilder builder;
  builder.startNewRegion("basic_from_example");
  builder.startNewBasicBlock();

  auto* a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* a1 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* a2 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* a3 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);

  auto* c4 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{10}));
  auto* c5 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{20}));

  auto* v6 = builder.createAndAddOp<arith::AddOp>(a0, a1);
  auto* v7 = builder.createAndAddOp<arith::MulOp>(v6, c4);
  auto* v8 = builder.createAndAddOp<arith::SubOp>(v7, a2);
  auto* v9 = builder.createAndAddOp<builtin::CopyOp>(v8);

  auto* v10 = builder.createAndAddOp<arith::MulOp>(a2, a3);
  auto* v11 = builder.createAndAddOp<arith::AddOp>(v10, c5);
  builder.createAndAddOp<arith::MulOp>(v11, v11);

  auto* c13 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{5}));
  auto* c14 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{7}));
  auto* v15 = builder.createAndAddOp<arith::AddOp>(c13, c14);
  auto* v16 = builder.createAndAddOp<builtin::CopyOp>(v15);
  builder.createAndAddOp<arith::SubOp>(v16, c14);

  builder.createAndAddOp<ctrlflow::ReturnOp>(v9);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::common::DCEPass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_EQ(ret->getInputsNum(), 1u);

  const Input& in = ret->getInput(0);
  auto* def = in.getDefiningOp();
  auto* copyOp = dynamic_cast<const builtin::CopyOp*>(def);
  ASSERT_NE(copyOp, nullptr);

  unsigned paramCount = 0;
  unsigned constCount = 0;
  unsigned addCount = 0;
  unsigned mulCount = 0;
  unsigned subCount = 0;
  unsigned copyCount = 0;
  unsigned returnCount = 0;

  for (const auto& op : ops) {
    if (dynamic_cast<const builtin::ParamOp*>(op.get()) != nullptr) {
      ++paramCount;
    } else if (dynamic_cast<const arith::ConstantOp*>(op.get()) != nullptr) {
      ++constCount;
    } else if (dynamic_cast<const arith::AddOp*>(op.get()) != nullptr) {
      ++addCount;
    } else if (dynamic_cast<const arith::MulOp*>(op.get()) != nullptr) {
      ++mulCount;
    } else if (dynamic_cast<const arith::SubOp*>(op.get()) != nullptr) {
      ++subCount;
    } else if (dynamic_cast<const builtin::CopyOp*>(op.get()) != nullptr) {
      ++copyCount;
    } else if (dynamic_cast<const ctrlflow::ReturnOp*>(op.get()) != nullptr) {
      ++returnCount;
    }
  }

  EXPECT_EQ(paramCount, 4u);
  EXPECT_EQ(constCount, 1u);
  EXPECT_EQ(addCount, 1u);
  EXPECT_EQ(mulCount, 1u);
  EXPECT_EQ(subCount, 1u);
  EXPECT_EQ(copyCount, 1u);
  EXPECT_EQ(returnCount, 1u);

  EXPECT_EQ(ops.size(), 10u);
}

TEST(DCE, DO_KEEP_PARAM) {
  IRBuilder builder;
  builder.startNewRegion("dce_keep_param");
  builder.startNewBasicBlock();

  /* a0 = */ builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* c0 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{42}));

  builder.createAndAddOp<ctrlflow::ReturnOp>(c0);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  EXPECT_FALSE(runSinglePass<opt::common::DCEPass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  bool hasParam = false;
  bool hasConst = false;
  bool hasReturn = false;

  for (const auto& op : ops) {
    if (dynamic_cast<const builtin::ParamOp*>(op.get()) != nullptr) {
      hasParam = true;
    }
    if (dynamic_cast<const arith::ConstantOp*>(op.get()) != nullptr) {
      hasConst = true;
    }
    if (dynamic_cast<const ctrlflow::ReturnOp*>(op.get()) != nullptr) {
      hasReturn = true;
    }
  }

  EXPECT_TRUE(hasParam);
  EXPECT_TRUE(hasConst);
  EXPECT_TRUE(hasReturn);
}

TEST(DCE, DO_KEEP_CALL) {
  IRBuilder builder;
  builder.startNewRegion("dce_keep_call");
  builder.startNewBasicBlock();

  auto* a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* c1 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{10}));

  auto* v2 = builder.createAndAddOp<arith::AddOp>(a0, c1);
  /* v3 = */ builder.createAndAddOp<ctrlflow::CallOp>("foo", DataType::SINT,
                                                      InputList{v2});

  builder.createAndAddOp<ctrlflow::ReturnOp>(a0);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  EXPECT_FALSE(runSinglePass<opt::common::DCEPass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  bool hasCall = false;
  bool hasAdd = false;

  for (const auto& op : ops) {
    if (dynamic_cast<const ctrlflow::CallOp*>(op.get()) != nullptr) {
      hasCall = true;
    }
    if (dynamic_cast<const arith::AddOp*>(op.get()) != nullptr) {
      hasAdd = true;
    }
  }

  EXPECT_TRUE(hasCall);
  EXPECT_TRUE(hasAdd);
}

TEST(DCE, TRANSITIVE_DEAD_CHAIN) {
  IRBuilder builder;
  builder.startNewRegion("dce_transitive_dead_chain");
  builder.startNewBasicBlock();

  auto* a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);

  auto* c1 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{1}));
  auto* c2 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{2}));

  auto* v3 = builder.createAndAddOp<arith::AddOp>(c1, c2);
  auto* v4 = builder.createAndAddOp<arith::MulOp>(v3, c1);
  /* v5 = */ builder.createAndAddOp<builtin::CopyOp>(v4);

  // Only a0 is used in the return, the whole chain above is dead.
  builder.createAndAddOp<ctrlflow::ReturnOp>(a0);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::common::DCEPass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GE(ops.size(), 2u);

  // After DCE only the parameter and the return should remain.
  EXPECT_EQ(ops.size(), 2u);

  unsigned paramCount = 0;
  unsigned returnCount = 0;

  for (const auto& op : ops) {
    if (dynamic_cast<const builtin::ParamOp*>(op.get()) != nullptr) {
      ++paramCount;
      continue;
    }
    if (dynamic_cast<const ctrlflow::ReturnOp*>(op.get()) != nullptr) {
      ++returnCount;
      continue;
    }

    EXPECT_EQ(dynamic_cast<const arith::ConstantOp*>(op.get()), nullptr);
    EXPECT_EQ(dynamic_cast<const arith::AddOp*>(op.get()), nullptr);
    EXPECT_EQ(dynamic_cast<const arith::MulOp*>(op.get()), nullptr);
    EXPECT_EQ(dynamic_cast<const builtin::CopyOp*>(op.get()), nullptr);
  }

  EXPECT_EQ(paramCount, 1u);
  EXPECT_EQ(returnCount, 1u);
}
