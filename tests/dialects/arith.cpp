#include "ops/dialects/arith/ops.hpp"
#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(ARITH, CONSTANT_INVARIANTS) {
  arith::ConstantOp op(makeConstAttribute(1));

  EXPECT_FALSE(op.isTerminator());
  EXPECT_TRUE(op.isa(GlobalOpcodes::CONSTANT));

  EXPECT_FALSE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 0);
}

TEST(ARITH, ADD_INVARIANTS) {
  arith::AddOp op(nullptr, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_TRUE(op.isa(GlobalOpcodes::ADD));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2);
}

TEST(ARITH, SUB_INVARIANTS) {
  arith::SubOp op(nullptr, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_TRUE(op.isa(GlobalOpcodes::SUB));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2);
}

TEST(ARITH, MUL_INVARIANTS) {
  arith::MulOp op(nullptr, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_TRUE(op.isa(GlobalOpcodes::MUL));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2);
}

TEST(ARITH, DIV_INVARIANTS) {
  arith::DivOp op(nullptr, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_TRUE(op.isa(GlobalOpcodes::DIV));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2);
}

TEST(ARITH, CAST_INVARIANTS) {
  arith::CastOp op(DataType::F32, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_TRUE(op.isa(GlobalOpcodes::CAST));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 1);
}

TEST(ARITH, COMPARE_INVARIANTS) {
  arith::CompareOp op(nullptr, nullptr, arith::CompareOp::Pred::EQ);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_TRUE(op.isa(GlobalOpcodes::COMPARE));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2);
}

TEST(ARITH, CONSTANT_BASIC) {
  arith::ConstantOp constOp(makeConstAttribute(1));

  EXPECT_TRUE(constOp.hasResult());
  EXPECT_EQ(constOp.getDataType(), DataType::SI32);

  std::string msg;
  bool vres = constOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, ADD_BASIC) {
  arith::ConstantOp constOp1(makeConstAttribute(1));
  arith::ConstantOp constOp2(makeConstAttribute(2));

  arith::AddOp addOp(&constOp1, &constOp2);

  EXPECT_TRUE(addOp.hasResult());
  EXPECT_EQ(addOp.getDataType(), DataType::SI32);

  std::string msg;
  bool vres = addOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, SUB_BASIC) {
  arith::ConstantOp constOp1(makeConstAttribute(1));
  arith::ConstantOp constOp2(makeConstAttribute(2));

  arith::SubOp subOp(&constOp1, &constOp2);

  EXPECT_TRUE(subOp.hasResult());
  EXPECT_EQ(subOp.getDataType(), DataType::SI32);

  std::string msg;
  bool vres = subOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, MUL_BASIC) {
  arith::ConstantOp constOp1(makeConstAttribute(1));
  arith::ConstantOp constOp2(makeConstAttribute(2));

  arith::MulOp mulOp(&constOp1, &constOp2);

  EXPECT_TRUE(mulOp.hasResult());
  EXPECT_EQ(mulOp.getDataType(), DataType::SI32);

  std::string msg;
  bool vres = mulOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, DIV_BASIC) {
  arith::ConstantOp constOp1(makeConstAttribute(1));
  arith::ConstantOp constOp2(makeConstAttribute(2));

  arith::DivOp divOp(&constOp1, &constOp2);

  EXPECT_TRUE(divOp.hasResult());
  EXPECT_EQ(divOp.getDataType(), DataType::SI32);

  std::string msg;
  bool vres = divOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, CAST_BASIC) {
  arith::ConstantOp cstOp(makeConstAttribute(1));
  arith::CastOp castOp(DataType::F32, &cstOp);

  EXPECT_TRUE(castOp.hasResult());
  EXPECT_EQ(castOp.getDataType(), DataType::F32);

  std::string msg;
  bool vres = castOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, COMPARE_BASIC) {
  arith::ConstantOp constOp1(makeConstAttribute(1));
  arith::ConstantOp constOp2(makeConstAttribute(2));

  arith::CompareOp cmpOp(&constOp1, &constOp2, arith::CompareOp::Pred::EQ);

  EXPECT_TRUE(cmpOp.hasResult());
  EXPECT_EQ(cmpOp.getDataType(), DataType::BOOL);

  std::string msg;
  bool vres = cmpOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, COMPARE_EXFAIL_UNEXPECTED_PRED) {
  try {
    arith::CompareOp cmpOp(nullptr, nullptr,
                           static_cast<arith::CompareOp::Pred>(255U));
  } catch (const IrisException& exc) {
    std::string msg = exc.what();
    EXPECT_TRUE(!msg.compare("Unexpected predicate in CompareOp operation!"));
    return;
  }
  FAIL();
}

TEST(ARITH, COMPARE_EXFAIL_NON_HOMOGEN_INPUTS1) {
  arith::ConstantOp cstI(makeConstAttribute(1));
  arith::ConstantOp cstF(makeConstAttribute(1.));

  arith::CompareOp cmp(&cstI, &cstF, arith::CompareOp::Pred::EQ);

  std::string msg;
  bool vres = cmp.verify(msg);

  EXPECT_EQ(vres, false);
  EXPECT_TRUE(msg.contains("inputs must have same data types"));
}

TEST(ARITH, COMPARE_EXFAIL_EMPTY_INPUT_1) {
  arith::ConstantOp cstI(makeConstAttribute(1));
  arith::CompareOp cmp(&cstI, nullptr, arith::CompareOp::Pred::EQ);

  std::string msg;
  bool vres = cmp.verify(msg);

  EXPECT_EQ(vres, false);
  EXPECT_TRUE(msg.contains("input #1 is empty!"));
}

TEST(ARITH, COMPARE_EXFAIL_EMPTY_INPUT_2) {
  arith::ConstantOp cstF(makeConstAttribute(1.));
  arith::CompareOp cmp(nullptr, &cstF, arith::CompareOp::Pred::EQ);

  std::string msg;
  bool vres = cmp.verify(msg);

  EXPECT_EQ(vres, false);
  EXPECT_TRUE(msg.contains("input #0 is empty!"));
}

TEST(ARITH, ADD_EXFAIL_NON_HOMOGEN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(1));
  arith::ConstantOp cstF(makeConstAttribute(1.));

  arith::AddOp add(&cstI, &cstF);

  std::string msg;
  bool vres = add.verify(msg);

  EXPECT_EQ(vres, false);
  EXPECT_TRUE(msg.contains("inputs must have same data types"));
}

TEST(ARITH, SUB_EXFAIL_NON_HOMOGEN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(1));
  arith::ConstantOp cstF(makeConstAttribute(1.));

  arith::SubOp sub(&cstI, &cstF);

  std::string msg;
  bool vres = sub.verify(msg);

  EXPECT_EQ(vres, false);
  EXPECT_TRUE(msg.contains("inputs must have same data types"));
}

TEST(ARITH, MUL_EXFAIL_NON_HOMOGEN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(1));
  arith::ConstantOp cstF(makeConstAttribute(1.));

  arith::MulOp mul(&cstI, &cstF);

  std::string msg;
  bool vres = mul.verify(msg);

  EXPECT_EQ(vres, false);
  EXPECT_TRUE(msg.contains("inputs must have same data types"));
}

TEST(ARITH, DIV_EXFAIL_NON_HOMOGEN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(1));
  arith::ConstantOp cstF(makeConstAttribute(1.));

  arith::DivOp div(&cstI, &cstF);

  std::string msg;
  bool vres = div.verify(msg);

  EXPECT_EQ(vres, false);
  EXPECT_TRUE(msg.contains("inputs must have same data types"));
}
