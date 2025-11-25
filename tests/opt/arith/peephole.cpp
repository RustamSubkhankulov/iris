#include <gtest/gtest.h>

#include <limits>

#include <iris.hpp>
using namespace iris;

#include <test_utils.hpp>
using iris::test::runSinglePass;
using iris::test::verifyRegion;

TEST(ARITH_PEEPHOLES, ADD_ZERO_RHS) {
  IRBuilder builder;
  builder.startNewRegion("ph_add_zero_rhs");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{0}));

  auto* add = builder.createAndAddOp<arith::AddOp>(x, zero);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(add);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();

  // No AddOp should remain
  for (const auto& op : ops) {
    EXPECT_EQ(dynamic_cast<const arith::AddOp*>(op.get()), nullptr);
  }

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(x));
}

TEST(ARITH_PEEPHOLES, ADD_ZERO_LHS) {
  IRBuilder builder;
  builder.startNewRegion("ph_add_zero_lhs");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{0}));

  auto* add = builder.createAndAddOp<arith::AddOp>(zero, x);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(add);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  for (const auto& op : ops) {
    EXPECT_EQ(dynamic_cast<const arith::AddOp*>(op.get()), nullptr);
  }

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(x));
}

TEST(ARITH_PEEPHOLES, ADD_ZERO_NOT_APPLIED) {
  IRBuilder builder;
  builder.startNewRegion("ph_add_zero_not_applied");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto* y = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);

  auto* add = builder.createAndAddOp<arith::AddOp>(x, y);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(add);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  EXPECT_FALSE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(add));
}

TEST(ARITH_PEEPHOLES, ADD_ROTATE_CONSTANTS) {
  IRBuilder builder;
  builder.startNewRegion("ph_add_rotate_constants");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto* c10 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{10}));
  auto* c5 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{5}));

  auto* addInner = builder.createAndAddOp<arith::AddOp>(x, c10); // x + 10
  auto* addOuter = builder.createAndAddOp<arith::AddOp>(addInner,
                                                        c5); // (x + 10) + 5
  builder.createAndAddOp<ctrlflow::ReturnOp>(addOuter);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  ASSERT_FALSE(ops.empty());

  auto* retAfter = dynamic_cast<const ctrlflow::ReturnOp*>(ops.back().get());
  ASSERT_NE(retAfter, nullptr);

  auto* outerAfter =
    dynamic_cast<const arith::AddOp*>(retAfter->getInput(0).getDefiningOp());
  ASSERT_NE(outerAfter, nullptr);
  EXPECT_EQ(outerAfter, addOuter);

  auto* outerLhs = outerAfter->getInputX().getDefiningOp();
  auto* outerRhs = outerAfter->getInputY().getDefiningOp();

  ASSERT_EQ(outerLhs, static_cast<Operation*>(x));
  auto* innerAfter = dynamic_cast<const arith::AddOp*>(outerRhs);
  ASSERT_NE(innerAfter, nullptr);
  EXPECT_EQ(innerAfter, addInner);

  auto* innerLhsConst = dynamic_cast<const arith::ConstantOp*>(
    innerAfter->getInputX().getDefiningOp());
  auto* innerRhsConst = dynamic_cast<const arith::ConstantOp*>(
    innerAfter->getInputY().getDefiningOp());
  ASSERT_NE(innerLhsConst, nullptr);
  ASSERT_NE(innerRhsConst, nullptr);

  EXPECT_NE(innerAfter->getInputX().getDefiningOp(),
            static_cast<Operation*>(x));
  EXPECT_NE(innerAfter->getInputY().getDefiningOp(),
            static_cast<Operation*>(x));

  EXPECT_TRUE((innerLhsConst == c10 && innerRhsConst == c5) ||
              (innerLhsConst == c5 && innerRhsConst == c10));
}

TEST(ARITH_PEEPHOLES, ADD_ROTATE_NOT_APPLIED_MULTIUSE) {
  IRBuilder builder;
  builder.startNewRegion("ph_add_rotate_multiuse");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto* c10 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{10}));
  auto* c5 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{5}));

  auto* addInner = builder.createAndAddOp<arith::AddOp>(x, c10); // x + 10
  auto* addOuter = builder.createAndAddOp<arith::AddOp>(addInner,
                                                        c5); // (x + 10) + 5
  builder.createAndAddOp<arith::SubOp>(addInner, x);

  builder.createAndAddOp<ctrlflow::ReturnOp>(addOuter);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  runSinglePass<opt::arith::ArithPeepHolePass>(*region);
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  ASSERT_FALSE(ops.empty());

  auto* retAfter = dynamic_cast<const ctrlflow::ReturnOp*>(ops.back().get());
  ASSERT_NE(retAfter, nullptr);

  auto* outerAfter =
    dynamic_cast<const arith::AddOp*>(retAfter->getInput(0).getDefiningOp());
  ASSERT_NE(outerAfter, nullptr);

  auto* innerAfter =
    dynamic_cast<const arith::AddOp*>(outerAfter->getInputX().getDefiningOp());
  ASSERT_NE(innerAfter, nullptr);

  auto* lhsDef = innerAfter->getInputX().getDefiningOp();
  auto* rhsDef = innerAfter->getInputY().getDefiningOp();

  bool lhsIsX = (lhsDef == static_cast<Operation*>(x));
  bool rhsIsX = (rhsDef == static_cast<Operation*>(x));

  ASSERT_TRUE(lhsIsX ^ rhsIsX); // exactly one is x

  auto* lhsConst = dynamic_cast<const arith::ConstantOp*>(lhsDef);
  auto* rhsConst = dynamic_cast<const arith::ConstantOp*>(rhsDef);
  EXPECT_TRUE(lhsConst == c10 || rhsConst == c10);
}

TEST(ARITH_PEEPHOLES, SUB_ZERO_RHS) {
  IRBuilder builder;
  builder.startNewRegion("ph_sub_zero_rhs");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{0}));

  auto* sub = builder.createAndAddOp<arith::SubOp>(x, zero);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(sub);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  for (const auto& op : ops) {
    EXPECT_EQ(dynamic_cast<const arith::SubOp*>(op.get()), nullptr);
  }

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(x));
}

TEST(ARITH_PEEPHOLES, SUB_ZERO_SELF) {
  IRBuilder builder;
  builder.startNewRegion("ph_sub_self_zero");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);

  auto* sub = builder.createAndAddOp<arith::SubOp>(x, x);
  builder.createAndAddOp<ctrlflow::ReturnOp>(sub);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  const Operation& last = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&last);
  ASSERT_NE(ret, nullptr);

  auto* def = ret->getInput(0).getDefiningOp();
  auto* c = dynamic_cast<const arith::ConstantOp*>(def);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(c->getDataType(), DataType::SINT);
  EXPECT_EQ(dynamic_cast<const SIntegerConstAttribute&>(c->getAttr()).getData(),
            0);
}

TEST(ARITH_PEEPHOLES, MUL_ONE_RHS) {
  IRBuilder builder;
  builder.startNewRegion("ph_mul_one_rhs");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto* one =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{1}));

  auto* mul = builder.createAndAddOp<arith::MulOp>(x, one);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(mul);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(x));
}

TEST(ARITH_PEEPHOLES, MUL_ONE_LHS) {
  IRBuilder builder;
  builder.startNewRegion("ph_mul_one_lhs");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto* one =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{1}));

  auto* mul = builder.createAndAddOp<arith::MulOp>(one, x);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(mul);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(x));
}

TEST(ARITH_PEEPHOLES, MUL_ZERO_RHS) {
  IRBuilder builder;
  builder.startNewRegion("ph_mul_zero_rhs");
  builder.startNewBasicBlock();

  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{0}));
  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);

  auto* mul = builder.createAndAddOp<arith::MulOp>(x, zero);
  builder.createAndAddOp<ctrlflow::ReturnOp>(mul);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  const Operation& last = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&last);
  ASSERT_NE(ret, nullptr);

  auto* def = ret->getInput(0).getDefiningOp();
  auto* c = dynamic_cast<const arith::ConstantOp*>(def);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(dynamic_cast<const SIntegerConstAttribute&>(c->getAttr()).getData(),
            0);
}

TEST(ARITH_PEEPHOLES, MUL_ZERO_LHS) {
  IRBuilder builder;
  builder.startNewRegion("ph_mul_zero_lhs");
  builder.startNewBasicBlock();

  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{0}));
  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);

  auto* mul = builder.createAndAddOp<arith::MulOp>(zero, x);
  builder.createAndAddOp<ctrlflow::ReturnOp>(mul);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  const Operation& last = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&last);
  ASSERT_NE(ret, nullptr);

  auto* def = ret->getInput(0).getDefiningOp();
  auto* c = dynamic_cast<const arith::ConstantOp*>(def);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(dynamic_cast<const SIntegerConstAttribute&>(c->getAttr()).getData(),
            0);
}

TEST(ARITH_PEEPHOLES, MUL_NOT_APPLIED) {
  IRBuilder builder;
  builder.startNewRegion("ph_mul_not_applied");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto* y = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);

  auto* mul = builder.createAndAddOp<arith::MulOp>(x, y);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(mul);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  EXPECT_FALSE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(mul));
}

TEST(ARITH_PEEPHOLES, MUL_ROTATE_CONSTANTS) {
  IRBuilder builder;
  builder.startNewRegion("ph_mul_rotate_constants");
  builder.startNewBasicBlock();

  auto* y = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto* c2 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{2}));
  auto* c3 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{3}));

  auto* mulInner = builder.createAndAddOp<arith::MulOp>(c2, y); // 2 * y
  auto* mulOuter = builder.createAndAddOp<arith::MulOp>(mulInner,
                                                        c3); // (2 * y) * 3
  builder.createAndAddOp<ctrlflow::ReturnOp>(mulOuter);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  ASSERT_FALSE(ops.empty());

  auto* retAfter = dynamic_cast<const ctrlflow::ReturnOp*>(ops.back().get());
  ASSERT_NE(retAfter, nullptr);

  auto* outerAfter =
    dynamic_cast<const arith::MulOp*>(retAfter->getInput(0).getDefiningOp());
  ASSERT_NE(outerAfter, nullptr);
  EXPECT_EQ(outerAfter, mulOuter);

  auto* outerLhs = outerAfter->getInputX().getDefiningOp();
  auto* outerRhs = outerAfter->getInputY().getDefiningOp();

  ASSERT_EQ(outerLhs, static_cast<Operation*>(y));
  auto* innerAfter = dynamic_cast<const arith::MulOp*>(outerRhs);
  ASSERT_NE(innerAfter, nullptr);
  EXPECT_EQ(innerAfter, mulInner);

  auto* innerLhsConst = dynamic_cast<const arith::ConstantOp*>(
    innerAfter->getInputX().getDefiningOp());
  auto* innerRhsConst = dynamic_cast<const arith::ConstantOp*>(
    innerAfter->getInputY().getDefiningOp());
  ASSERT_NE(innerLhsConst, nullptr);
  ASSERT_NE(innerRhsConst, nullptr);

  EXPECT_NE(innerAfter->getInputX().getDefiningOp(),
            static_cast<Operation*>(y));
  EXPECT_NE(innerAfter->getInputY().getDefiningOp(),
            static_cast<Operation*>(y));

  EXPECT_TRUE((innerLhsConst == c2 && innerRhsConst == c3) ||
              (innerLhsConst == c3 && innerRhsConst == c2));
}

TEST(ARITH_PEEPHOLES, MUL_ROTATE_NOT_APPLIED_MULTIUSE) {
  IRBuilder builder;
  builder.startNewRegion("ph_mul_rotate_multiuse");
  builder.startNewBasicBlock();

  auto* y = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto* c2 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{2}));
  auto* c3 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{3}));

  auto* mulInner = builder.createAndAddOp<arith::MulOp>(y, c2); // 2 * y
  auto* mulOuter = builder.createAndAddOp<arith::MulOp>(mulInner,
                                                        c3); // (2 * y) * 3
  builder.createAndAddOp<arith::SubOp>(mulInner, y);

  builder.createAndAddOp<ctrlflow::ReturnOp>(mulOuter);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  runSinglePass<opt::arith::ArithPeepHolePass>(*region);
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  ASSERT_FALSE(ops.empty());

  auto* retAfter = dynamic_cast<const ctrlflow::ReturnOp*>(ops.back().get());
  ASSERT_NE(retAfter, nullptr);

  auto* outerAfter =
    dynamic_cast<const arith::MulOp*>(retAfter->getInput(0).getDefiningOp());
  ASSERT_NE(outerAfter, nullptr);

  auto* innerAfter =
    dynamic_cast<const arith::MulOp*>(outerAfter->getInputX().getDefiningOp());
  ASSERT_NE(innerAfter, nullptr);

  auto* lhsDef = innerAfter->getInputX().getDefiningOp();
  auto* rhsDef = innerAfter->getInputY().getDefiningOp();

  bool lhsIsX = (lhsDef == static_cast<Operation*>(y));
  bool rhsIsX = (rhsDef == static_cast<Operation*>(y));

  ASSERT_TRUE(lhsIsX ^ rhsIsX); // exactly one is x

  auto* lhsConst = dynamic_cast<const arith::ConstantOp*>(lhsDef);
  auto* rhsConst = dynamic_cast<const arith::ConstantOp*>(rhsDef);
  EXPECT_TRUE(lhsConst == c2 || rhsConst == c2);
}

TEST(ARITH_PEEPHOLES, DIV_ONE_RHS) {
  IRBuilder builder;
  builder.startNewRegion("ph_div_one_rhs");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto* one =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{1}));

  auto* div = builder.createAndAddOp<arith::DivOp>(x, one);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(div);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(x));
}

TEST(ARITH_PEEPHOLES, DIV_NOT_APPLIED) {
  IRBuilder builder;
  builder.startNewRegion("ph_div_one_not_applied");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto* two =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{2}));

  auto* div = builder.createAndAddOp<arith::DivOp>(x, two);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(div);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  EXPECT_FALSE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(div));
}

TEST(ARITH_PEEPHOLES, AND_ZERO_RHS) {
  IRBuilder builder;
  builder.startNewRegion("ph_and_zero_rhs");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{0}));

  auto* op = builder.createAndAddOp<arith::AndOp>(x, zero);
  builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  const Operation& last = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&last);
  ASSERT_NE(ret, nullptr);

  auto* def = ret->getInput(0).getDefiningOp();
  auto* c = dynamic_cast<const arith::ConstantOp*>(def);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(dynamic_cast<const UIntegerConstAttribute&>(c->getAttr()).getData(),
            0u);
}

TEST(ARITH_PEEPHOLES, AND_ZERO_LHS) {
  IRBuilder builder;
  builder.startNewRegion("ph_and_zero_lhs");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{0}));

  auto* op = builder.createAndAddOp<arith::AndOp>(zero, x);
  builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  const Operation& last = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&last);
  ASSERT_NE(ret, nullptr);

  auto* def = ret->getInput(0).getDefiningOp();
  auto* c = dynamic_cast<const arith::ConstantOp*>(def);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(dynamic_cast<const UIntegerConstAttribute&>(c->getAttr()).getData(),
            0u);
}

TEST(ARITH_PEEPHOLES, AND_ALL_ONES_RHS) {
  IRBuilder builder;
  builder.startNewRegion("ph_and_allones_rhs");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* all = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(std::numeric_limits<uint64_t>::max()));

  auto* op = builder.createAndAddOp<arith::AndOp>(x, all);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(x));
}

TEST(ARITH_PEEPHOLES, AND_ALL_ONES_LHS) {
  IRBuilder builder;
  builder.startNewRegion("ph_and_allones_lhs");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* all = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(std::numeric_limits<uint64_t>::max()));

  auto* op = builder.createAndAddOp<arith::AndOp>(all, x);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(x));
}

TEST(ARITH_PEEPHOLES, AND_SELF) {
  IRBuilder builder;
  builder.startNewRegion("ph_and_self");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);

  auto* op = builder.createAndAddOp<arith::AndOp>(x, x);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(x));
}

TEST(ARITH_PEEPHOLES, OR_ZERO_RHS) {
  IRBuilder builder;
  builder.startNewRegion("ph_or_zero_rhs");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{0}));

  auto* op = builder.createAndAddOp<arith::OrOp>(x, zero);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(x));
}

TEST(ARITH_PEEPHOLES, OR_ZERO_LHS) {
  IRBuilder builder;
  builder.startNewRegion("ph_or_zero_lhs");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{0}));

  auto* op = builder.createAndAddOp<arith::OrOp>(zero, x);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(x));
}

TEST(ARITH_PEEPHOLES, OR_ALL_ONES_RHS) {
  IRBuilder builder;
  builder.startNewRegion("ph_or_allones_rhs");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* all = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(std::numeric_limits<uint64_t>::max()));

  auto* op = builder.createAndAddOp<arith::OrOp>(x, all);
  builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  const Operation& last = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&last);
  ASSERT_NE(ret, nullptr);

  auto* def = ret->getInput(0).getDefiningOp();
  auto* c = dynamic_cast<const arith::ConstantOp*>(def);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(dynamic_cast<const UIntegerConstAttribute&>(c->getAttr()).getData(),
            std::numeric_limits<uint64_t>::max());
}

TEST(ARITH_PEEPHOLES, OR_ALL_ONES_LHS) {
  IRBuilder builder;
  builder.startNewRegion("ph_or_allones_lhs");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* all = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(std::numeric_limits<uint64_t>::max()));

  auto* op = builder.createAndAddOp<arith::OrOp>(all, x);
  builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  const Operation& last = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&last);
  ASSERT_NE(ret, nullptr);

  auto* def = ret->getInput(0).getDefiningOp();
  auto* c = dynamic_cast<const arith::ConstantOp*>(def);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(dynamic_cast<const UIntegerConstAttribute&>(c->getAttr()).getData(),
            std::numeric_limits<uint64_t>::max());
}

TEST(ARITH_PEEPHOLES, XOR_SELF) {
  IRBuilder builder;
  builder.startNewRegion("ph_xor_self_zero");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);

  auto* op = builder.createAndAddOp<arith::XorOp>(x, x);
  builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  const Operation& last = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&last);
  ASSERT_NE(ret, nullptr);

  auto* def = ret->getInput(0).getDefiningOp();
  auto* c = dynamic_cast<const arith::ConstantOp*>(def);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(dynamic_cast<const UIntegerConstAttribute&>(c->getAttr()).getData(),
            0u);
}

TEST(ARITH_PEEPHOLES, XOR_ALL_ONES) {
  IRBuilder builder;
  builder.startNewRegion("ph_xor_allones_rhs_not");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* all = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(std::numeric_limits<uint64_t>::max()));

  auto* op = builder.createAndAddOp<arith::XorOp>(x, all);
  builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  const Operation& last = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&last);
  ASSERT_NE(ret, nullptr);

  auto* def = ret->getInput(0).getDefiningOp();
  auto* notOp = dynamic_cast<const arith::NotOp*>(def);
  ASSERT_NE(notOp, nullptr);
}

TEST(ARITH_PEEPHOLES, SAL_ZERO_SHIFT) {
  IRBuilder builder;
  builder.startNewRegion("ph_sal_zero_shift");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{0}));

  auto* op = builder.createAndAddOp<arith::SalOp>(x, zero);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(x));
}

TEST(ARITH_PEEPHOLES, SAR_ZERO_SHIFT) {
  IRBuilder builder;
  builder.startNewRegion("ph_sar_zero_shift");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{0}));

  auto* op = builder.createAndAddOp<arith::SarOp>(x, zero);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(x));
}

TEST(ARITH_PEEPHOLES, SHL_ZERO_SHIFT) {
  IRBuilder builder;
  builder.startNewRegion("ph_shl_zero_shift");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{0}));

  auto* op = builder.createAndAddOp<arith::ShlOp>(x, zero);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(x));
}

TEST(ARITH_PEEPHOLES, SHR_ZERO_SHIFT) {
  IRBuilder builder;
  builder.startNewRegion("ph_sal_zero_shift");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{0}));

  auto* op = builder.createAndAddOp<arith::ShrOp>(x, zero);
  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(x));
}

TEST(ARITH_PEEPHOLES, SHL_ZERO_ARG) {
  IRBuilder builder;
  builder.startNewRegion("ph_shl_zero_arg");
  builder.startNewBasicBlock();

  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{0}));
  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);

  auto* op = builder.createAndAddOp<arith::ShlOp>(zero, x);
  builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  const Operation& last = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&last);
  ASSERT_NE(ret, nullptr);

  auto* def = ret->getInput(0).getDefiningOp();
  auto* c = dynamic_cast<const arith::ConstantOp*>(def);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(dynamic_cast<const UIntegerConstAttribute&>(c->getAttr()).getData(),
            0u);
}

TEST(ARITH_PEEPHOLES, SHR_ZERO_ARG) {
  IRBuilder builder;
  builder.startNewRegion("ph_shr_zero_arg");
  builder.startNewBasicBlock();

  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(uint64_t{0}));
  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);

  auto* op = builder.createAndAddOp<arith::ShrOp>(zero, x);
  builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  const Operation& last = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&last);
  ASSERT_NE(ret, nullptr);

  auto* def = ret->getInput(0).getDefiningOp();
  auto* c = dynamic_cast<const arith::ConstantOp*>(def);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(dynamic_cast<const UIntegerConstAttribute&>(c->getAttr()).getData(),
            0u);
}

TEST(ARITH_PEEPHOLES, SAL_ZERO_ARG) {
  IRBuilder builder;
  builder.startNewRegion("ph_sal_zero_arg");
  builder.startNewBasicBlock();

  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{0}));
  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);

  auto* op = builder.createAndAddOp<arith::SalOp>(zero, x);
  builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  const Operation& last = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&last);
  ASSERT_NE(ret, nullptr);

  auto* def = ret->getInput(0).getDefiningOp();
  auto* c = dynamic_cast<const arith::ConstantOp*>(def);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(dynamic_cast<const SIntegerConstAttribute&>(c->getAttr()).getData(),
            0u);
}

TEST(ARITH_PEEPHOLES, SAR_ZERO_ARG) {
  IRBuilder builder;
  builder.startNewRegion("ph_sar_zero_arg");
  builder.startNewBasicBlock();

  auto* zero =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(int64_t{0}));
  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);

  auto* op = builder.createAndAddOp<arith::SarOp>(zero, x);
  builder.createAndAddOp<ctrlflow::ReturnOp>(op);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  auto* bb = region->getStartBasicBlock();
  const auto& ops = bb->getOps();
  const Operation& last = *ops.back();
  auto* ret = dynamic_cast<const ctrlflow::ReturnOp*>(&last);
  ASSERT_NE(ret, nullptr);

  auto* def = ret->getInput(0).getDefiningOp();
  auto* c = dynamic_cast<const arith::ConstantOp*>(def);
  ASSERT_NE(c, nullptr);
  EXPECT_EQ(dynamic_cast<const SIntegerConstAttribute&>(c->getAttr()).getData(),
            0u);
}

TEST(ARITH_PEEPHOLES, NOT_DOUBLE_NEGATION) {
  IRBuilder builder;
  builder.startNewRegion("ph_not_double");
  builder.startNewBasicBlock();

  auto* x = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto* n0 = builder.createAndAddOp<arith::NotOp>(x);
  auto* n1 = builder.createAndAddOp<arith::NotOp>(n0);

  auto* ret = builder.createAndAddOp<ctrlflow::ReturnOp>(n1);

  auto& bb0 = builder.finalizeCurBasicBlock();
  auto region = builder.obtainRegion();
  ASSERT_TRUE(region);
  ASSERT_TRUE(region->setStartBasicBlock(&bb0));
  ASSERT_TRUE(region->setFinalBasicBlock(&bb0));

  verifyRegion(*region);
  ASSERT_TRUE(runSinglePass<opt::arith::ArithPeepHolePass>(*region));
  verifyRegion(*region);

  EXPECT_EQ(ret->getInput(0).getDefiningOp(), static_cast<Operation*>(x));
}
