#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

#include <test_utils.hpp>
using iris::test::runSinglePass;
using iris::test::verifyRegion;

TEST(CONST_FOLD, ADD_UINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_add_uint");
  builder.startNewBasicBlock();

  auto* c0 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<uint64_t>(40)));
  auto* c1 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<uint64_t>(2)));

  auto* add = builder.createAndAddOp<arith::AddOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(add);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::UINT);
  auto* a = dynamic_cast<const UIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, 42u);
}

TEST(CONST_FOLD, ADD_SINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_add_sint");
  builder.startNewBasicBlock();

  auto* c0 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{-5}));
  auto* c1 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{7}));

  auto* add = builder.createAndAddOp<arith::AddOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(add);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::SINT);
  auto* a = dynamic_cast<const SIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0, 2);
}

TEST(CONST_FOLD, ADD_FLOAT) {
  IRBuilder builder;
  builder.startNewRegion("cf_add_float");
  builder.startNewBasicBlock();

  auto* c0 = builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(1.5));
  auto* c1 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(2.25));

  auto* add = builder.createAndAddOp<arith::AddOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(add);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::FLOAT);
  auto* a = dynamic_cast<const FloatConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_DOUBLE_EQ(a ? a->getData() : 0.0, 3.75);
}

TEST(CONST_FOLD, ADD_NOT_APPLIED) {
  IRBuilder builder;
  builder.startNewRegion("cf_add_not_applied");
  builder.startNewBasicBlock();

  auto* a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* c0 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{10}));

  auto* add = builder.createAndAddOp<arith::AddOp>(a0, c0);
  builder.createAndAddOp<ctrlflow::ReturnOp>(add);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  EXPECT_FALSE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);
}

TEST(CONST_FOLD, ADD_CHAINED_OPS) {
  IRBuilder builder;
  builder.startNewRegion("cf_add_chain");
  builder.startNewBasicBlock();

  auto* c0 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{1}));
  auto* c1 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{2}));
  auto* c2 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{3}));
  auto* c3 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{4}));

  auto* a0 = builder.createAndAddOp<arith::AddOp>(c0, c1); // 3
  auto* a1 = builder.createAndAddOp<arith::AddOp>(a0, c2); // 6
  auto* a2 = builder.createAndAddOp<arith::AddOp>(a1, c3); // 10

  builder.createAndAddOp<ctrlflow::ReturnOp>(a2);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::UINT);
  auto* a = dynamic_cast<const UIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, 10u);

  // No Add operations should remain.
  for (const auto& op : bb->getOps()) {
    EXPECT_EQ(dynamic_cast<const arith::AddOp*>(op.get()), nullptr);
  }
}

TEST(CONST_FOLD, SUB_UINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_sub_uint");
  builder.startNewBasicBlock();

  auto* c0 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<uint64_t>(10)));
  auto* c1 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<uint64_t>(3)));

  auto* sub = builder.createAndAddOp<arith::SubOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(sub);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::UINT);
  auto* a = dynamic_cast<const UIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, 7u);
}

TEST(CONST_FOLD, SUB_SINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_sub_sint");
  builder.startNewBasicBlock();

  auto* c0 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{5}));
  auto* c1 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{-2}));

  auto* sub = builder.createAndAddOp<arith::SubOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(sub);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::SINT);
  auto* a = dynamic_cast<const SIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0, 7);
}

TEST(CONST_FOLD, SUB_FLOAT) {
  IRBuilder builder;
  builder.startNewRegion("cf_sub_float");
  builder.startNewBasicBlock();

  auto* c0 = builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(4.5));
  auto* c1 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(1.25));

  auto* sub = builder.createAndAddOp<arith::SubOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(sub);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::FLOAT);
  auto* a = dynamic_cast<const FloatConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_DOUBLE_EQ(a ? a->getData() : 0.0, 3.25);
}

TEST(CONST_FOLD, SUB_NOT_APPLIED) {
  IRBuilder builder;
  builder.startNewRegion("cf_sub_not_applied");
  builder.startNewBasicBlock();

  auto* a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto* c0 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{5}));

  auto* sub = builder.createAndAddOp<arith::SubOp>(a0, c0);
  builder.createAndAddOp<ctrlflow::ReturnOp>(sub);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  EXPECT_FALSE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);
}

TEST(CONST_FOLD, MUL_UINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_mul_uint");
  builder.startNewBasicBlock();

  auto* c0 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{6}));
  auto* c1 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{7}));

  auto* mul = builder.createAndAddOp<arith::MulOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(mul);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::UINT);
  auto* a = dynamic_cast<const UIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, 42u);
}

TEST(CONST_FOLD, MUL_SINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_mul_sint");
  builder.startNewBasicBlock();

  auto* c0 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{-6}));
  auto* c1 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{-7}));

  auto* mul = builder.createAndAddOp<arith::MulOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(mul);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::SINT);
  auto* a = dynamic_cast<const SIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0, 42);
}

TEST(CONST_FOLD, MUL_FLOAT) {
  IRBuilder builder;
  builder.startNewRegion("cf_mul_float");
  builder.startNewBasicBlock();

  auto* c0 = builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(2.0));
  auto* c1 = builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(3.5));

  auto* mul = builder.createAndAddOp<arith::MulOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(mul);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::FLOAT);
  auto* a = dynamic_cast<const FloatConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_DOUBLE_EQ(a ? a->getData() : 0.0, 7.0);
}

TEST(CONST_FOLD, MUL_NOT_APPLIED) {
  IRBuilder builder;
  builder.startNewRegion("cf_mul_not_applied");
  builder.startNewBasicBlock();

  auto* a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* c0 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{5}));

  auto* mul = builder.createAndAddOp<arith::MulOp>(a0, c0);
  builder.createAndAddOp<ctrlflow::ReturnOp>(mul);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  EXPECT_FALSE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);
}

TEST(CONST_FOLD, DIV_UINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_div_uint");
  builder.startNewBasicBlock();

  auto* c0 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{84}));
  auto* c1 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{2}));

  auto* div = builder.createAndAddOp<arith::DivOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(div);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::UINT);
  auto* a = dynamic_cast<const UIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, 42u);
}

TEST(CONST_FOLD, DIV_SINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_div_sint");
  builder.startNewBasicBlock();

  auto* c0 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{-84}));
  auto* c1 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{-2}));

  auto* div = builder.createAndAddOp<arith::DivOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(div);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::SINT);
  auto* a = dynamic_cast<const SIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0, 42);
}

TEST(CONST_FOLD, DIV_FLOAT) {
  IRBuilder builder;
  builder.startNewRegion("cf_div_float");
  builder.startNewBasicBlock();

  auto* c0 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(84.0));
  auto* c1 = builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(2.0));

  auto* div = builder.createAndAddOp<arith::DivOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(div);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::FLOAT);
  auto* a = dynamic_cast<const FloatConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_DOUBLE_EQ(a ? a->getData() : 0.0, 42.0);
}

TEST(CONST_FOLD, DIV_NOT_APPLIED_DIV_BY_ZERO) {
  IRBuilder builder;
  builder.startNewRegion("cf_div_not_applied");
  builder.startNewBasicBlock();

  auto* c0 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{1}));
  auto* c1 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{0}));

  auto* div = builder.createAndAddOp<arith::DivOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(div);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  EXPECT_FALSE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);
}

TEST(CONST_FOLD, DIV_NOT_APPLIED_NON_CONST) {
  IRBuilder builder;
  builder.startNewRegion("cf_div_not_applied");
  builder.startNewBasicBlock();

  auto* a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* c0 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{10}));

  auto* div = builder.createAndAddOp<arith::DivOp>(a0, c0);
  builder.createAndAddOp<ctrlflow::ReturnOp>(div);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  EXPECT_FALSE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);
}

TEST(CONST_FOLD, AND_UINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_and_uint");
  builder.startNewBasicBlock();

  auto* c0 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(uint64_t{0xF0}));
  auto* c1 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(uint64_t{0x0F}));

  auto* andOp = builder.createAndAddOp<arith::AndOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(andOp);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::UINT);
  auto* a = dynamic_cast<const UIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, 0u);
}

TEST(CONST_FOLD, AND_SINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_and_sint");
  builder.startNewBasicBlock();

  auto* c0 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(int64_t{0xF0}));
  auto* c1 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(int64_t{0x0F}));

  auto* andOp = builder.createAndAddOp<arith::AndOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(andOp);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::SINT);
  auto* a = dynamic_cast<const SIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, 0u);
}

TEST(CONST_FOLD, OR_UINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_or_uint");
  builder.startNewBasicBlock();

  auto* c0 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(uint64_t{0xF0}));
  auto* c1 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(uint64_t{0x0F}));

  auto* orOp = builder.createAndAddOp<arith::OrOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(orOp);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::UINT);
  auto* a = dynamic_cast<const UIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, 0xFFu);
}

TEST(CONST_FOLD, OR_SINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_or_sint");
  builder.startNewBasicBlock();

  auto* c0 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(int64_t{0xF0}));
  auto* c1 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(int64_t{0x0F}));

  auto* orOp = builder.createAndAddOp<arith::OrOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(orOp);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::SINT);
  auto* a = dynamic_cast<const SIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, 0xFFu);
}

TEST(CONST_FOLD, XOR_UINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_xor_uint");
  builder.startNewBasicBlock();

  auto* c0 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(uint64_t{0xF0}));
  auto* c1 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(uint64_t{0x0F}));

  auto* xorOp = builder.createAndAddOp<arith::XorOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(xorOp);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::UINT);
  auto* a = dynamic_cast<const UIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, 0xFFu);
}

TEST(CONST_FOLD, XOR_SINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_xor_sint");
  builder.startNewBasicBlock();

  auto* c0 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(int64_t{0xF0}));
  auto* c1 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(int64_t{0x0F}));

  auto* xorOp = builder.createAndAddOp<arith::XorOp>(c0, c1);
  builder.createAndAddOp<ctrlflow::ReturnOp>(xorOp);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::SINT);
  auto* a = dynamic_cast<const SIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, 0xFFu);
}

TEST(CONST_FOLD, SAL_SINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_sal");
  builder.startNewBasicBlock();

  auto* value =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{1}));
  auto* shift =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{3}));

  auto* sal = builder.createAndAddOp<arith::SalOp>(value, shift);
  builder.createAndAddOp<ctrlflow::ReturnOp>(sal);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::SINT);
  auto* a = dynamic_cast<const SIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0, 8);
}

TEST(CONST_FOLD, SHL_UINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_shl");
  builder.startNewBasicBlock();

  auto* value =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{2}));
  auto* shift =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{4}));

  auto* shl = builder.createAndAddOp<arith::ShlOp>(value, shift);
  builder.createAndAddOp<ctrlflow::ReturnOp>(shl);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::UINT);
  auto* a = dynamic_cast<const UIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, 32u);
}

TEST(CONST_FOLD, SAR_SINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_sar");
  builder.startNewBasicBlock();

  auto* value =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{16}));
  auto* shift =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{2}));

  auto* sar = builder.createAndAddOp<arith::SarOp>(value, shift);
  builder.createAndAddOp<ctrlflow::ReturnOp>(sar);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::SINT);
  auto* a = dynamic_cast<const SIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0, 4);
}

TEST(CONST_FOLD, SHR_UINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_shr");
  builder.startNewBasicBlock();

  auto* value =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{16}));
  auto* shift =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{2}));

  auto* shr = builder.createAndAddOp<arith::ShrOp>(value, shift);
  builder.createAndAddOp<ctrlflow::ReturnOp>(shr);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::UINT);
  auto* a = dynamic_cast<const UIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, 4u);
}

TEST(CONST_FOLD, NOT_UINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_not_uint");
  builder.startNewBasicBlock();

  auto* c0 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{0u}));
  auto* notOp = builder.createAndAddOp<arith::NotOp>(c0);
  builder.createAndAddOp<ctrlflow::ReturnOp>(notOp);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::UINT);
  auto* a = dynamic_cast<const UIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, std::numeric_limits<uint64_t>::max());
}

TEST(CONST_FOLD, NOT_SINT) {
  IRBuilder builder;
  builder.startNewRegion("cf_not_sint");
  builder.startNewBasicBlock();

  auto* c0 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{0}));
  auto* notOp = builder.createAndAddOp<arith::NotOp>(c0);
  builder.createAndAddOp<ctrlflow::ReturnOp>(notOp);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::SINT);
  auto* a = dynamic_cast<const SIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0, -1);
}

TEST(CONST_FOLD, NOT_CHAINED) {
  IRBuilder builder;
  builder.startNewRegion("cf_not_chain");
  builder.startNewBasicBlock();

  auto* c0 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(uint64_t{0x0F}));

  auto* n0 = builder.createAndAddOp<arith::NotOp>(c0);
  auto* n1 = builder.createAndAddOp<arith::NotOp>(n0);
  auto* n2 = builder.createAndAddOp<arith::NotOp>(n1);

  builder.createAndAddOp<ctrlflow::ReturnOp>(n2);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::UINT);

  auto* a = dynamic_cast<const UIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, ~uint64_t{0x0F});
}

TEST(CONST_FOLD, CMP_SINT) {
  using Pred = arith::CompareOp::Pred;

  IRBuilder builder;
  builder.startNewRegion("cf_cmp_sint");
  builder.startNewBasicBlock();

  auto* a =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{1}));
  auto* b =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{2}));

  auto* cmp = builder.createAndAddOp<arith::CompareOp>(a, b, Pred::B); // 1 < 2
  builder.createAndAddOp<ctrlflow::ReturnOp>(cmp);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::BOOL);
  auto* attr = dynamic_cast<const BoolConstAttribute*>(&c->getAttr());
  EXPECT_NE(attr, nullptr);
  EXPECT_TRUE(attr ? attr->getData() : false);
}

TEST(CONST_FOLD, CMP_UINT) {
  using Pred = arith::CompareOp::Pred;

  IRBuilder builder;
  builder.startNewRegion("cf_cmp_uint");
  builder.startNewBasicBlock();

  auto* a =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{1}));
  auto* b =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{2}));

  auto* cmp = builder.createAndAddOp<arith::CompareOp>(a, b, Pred::B); // 1 < 2
  builder.createAndAddOp<ctrlflow::ReturnOp>(cmp);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::BOOL);
  auto* attr = dynamic_cast<const BoolConstAttribute*>(&c->getAttr());
  EXPECT_NE(attr, nullptr);
  EXPECT_TRUE(attr ? attr->getData() : false);
}

TEST(CONST_FOLD, CMP_FLOAT) {
  using Pred = arith::CompareOp::Pred;

  IRBuilder builder;
  builder.startNewRegion("cf_cmp_float");
  builder.startNewBasicBlock();

  auto* a = builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(1.0));
  auto* b = builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(1.0));

  auto* cmp = builder.createAndAddOp<arith::CompareOp>(a, b, Pred::EQ);
  builder.createAndAddOp<ctrlflow::ReturnOp>(cmp);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::BOOL);
  auto* attr = dynamic_cast<const BoolConstAttribute*>(&c->getAttr());
  EXPECT_NE(attr, nullptr);
  EXPECT_TRUE(attr ? attr->getData() : false);
}

TEST(CONST_FOLD, CMP_BOOL) {
  using Pred = arith::CompareOp::Pred;

  IRBuilder builder;
  builder.startNewRegion("cf_cmp_bool");
  builder.startNewBasicBlock();

  auto* a = builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(true));
  auto* b =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(false));

  auto* cmp = builder.createAndAddOp<arith::CompareOp>(a, b, Pred::NEQ);
  builder.createAndAddOp<ctrlflow::ReturnOp>(cmp);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::BOOL);
  auto* attr = dynamic_cast<const BoolConstAttribute*>(&c->getAttr());
  EXPECT_NE(attr, nullptr);
  EXPECT_TRUE(attr ? attr->getData() : false);
}

TEST(CONST_FOLD, CMP_NOT_APPLIED) {
  IRBuilder builder;
  builder.startNewRegion("cf_cmp_not_applied");
  builder.startNewBasicBlock();

  auto* a =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{1}));
  auto* bParam = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);

  auto* cmp = builder.createAndAddOp<arith::CompareOp>(
    a, bParam, arith::CompareOp::Pred::EQ);
  builder.createAndAddOp<ctrlflow::ReturnOp>(cmp);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  EXPECT_FALSE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);
}

TEST(CONST_FOLD, INTEGRATION_BASIC_ARITH) {
  // ((2 + 3) * (10 - 2)) -> 40
  IRBuilder builder;
  builder.startNewRegion("cf_integration_1");
  builder.startNewBasicBlock();

  auto* c2 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{2}));
  auto* c3 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{3}));
  auto* c10 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{10}));
  auto* c2b =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{2}));

  auto* add = builder.createAndAddOp<arith::AddOp>(c2, c3);   // 5
  auto* sub = builder.createAndAddOp<arith::SubOp>(c10, c2b); // 8
  auto* mul = builder.createAndAddOp<arith::MulOp>(add, sub); // 40

  builder.createAndAddOp<ctrlflow::ReturnOp>(mul);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::UINT);
  auto* a = dynamic_cast<const UIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, 40u);
}

TEST(CONST_FOLD, INTEGRATION_BITWISE) {
  // ((0xF0 & 0x0F) ^ 0xFF) << 4 -> 0xFF0
  IRBuilder builder;
  builder.startNewRegion("cf_integration_2");
  builder.startNewBasicBlock();

  auto* cF0 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(uint64_t{0xF0}));
  auto* c0F = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(uint64_t{0x0F}));
  auto* cFF = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(uint64_t{0xFF}));
  auto* c4 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{4}));

  auto* andOp = builder.createAndAddOp<arith::AndOp>(cF0, c0F);   // 0
  auto* xorOp = builder.createAndAddOp<arith::XorOp>(andOp, cFF); // 0xFF
  auto* shlOp = builder.createAndAddOp<arith::ShlOp>(xorOp, c4);  // 0xFF0

  builder.createAndAddOp<ctrlflow::ReturnOp>(shlOp);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::UINT);
  auto* a = dynamic_cast<const UIntegerConstAttribute*>(&c->getAttr());
  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a ? a->getData() : 0u, 0xFF0u);
}

TEST(CONST_FOLD, INTEGRATION_CMP) {
  using Pred = arith::CompareOp::Pred;

  // cmp(a, b) -> bool, then another cmp(bool, true) -> final bool const
  IRBuilder builder;
  builder.startNewRegion("cf_integration_3");
  builder.startNewBasicBlock();

  auto* c10 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{10}));
  auto* c20 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{20}));
  auto* cTrue =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(true));

  auto* cmp0 = builder.createAndAddOp<arith::CompareOp>(
    c10, c20, Pred::B); // 10 < 20 -> true
  auto* cmp1 = builder.createAndAddOp<arith::CompareOp>(
    cmp0, cTrue, Pred::EQ); // true == true

  builder.createAndAddOp<ctrlflow::ReturnOp>(cmp1);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithConstFoldPass>(*region));
  verifyRegion(*region);

  const arith::ConstantOp* c = nullptr;

  auto* bb = region->getStartBasicBlock();
  ASSERT_TRUE(bb != nullptr);

  const auto& ops = bb->getOps();
  ASSERT_GT(ops.size(), 0u);

  const Operation& lastOp = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&lastOp);
  ASSERT_NE(ret, nullptr);
  ASSERT_TRUE(ret->getInputsNum() == 0 || ret->getInputsNum() == 1);

  if (ret->getInputsNum()) {
    const Input& in = ret->getInput(0);
    auto* def = in.getDefiningOp();
    c = dynamic_cast<const arith::ConstantOp*>(def);
    ASSERT_NE(c, nullptr);
  } else {
    c = nullptr;
  }

  ASSERT_EQ(c->getDataType(), DataType::BOOL);
  auto* attr = dynamic_cast<const BoolConstAttribute*>(&c->getAttr());
  EXPECT_NE(attr, nullptr);
  EXPECT_TRUE(attr ? attr->getData() : false);
}
