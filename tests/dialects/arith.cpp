#include "ops/dialects/arith/ops.hpp"
#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(ARITH, CONSTANT) {
  arith::ConstantOp constOp(makeConstAttribute(1));

  EXPECT_EQ(constOp.getDataType(), DataType::SI32);
  EXPECT_EQ(constOp.getInputsNum(), 0);
  EXPECT_FALSE(constOp.hasInputs());

  std::string msg;
  bool vres = constOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, ADD) {
  arith::ConstantOp constOp1(makeConstAttribute(1));
  arith::ConstantOp constOp2(makeConstAttribute(2));

  arith::AddOp addOp(&constOp1, &constOp2);

  EXPECT_EQ(addOp.getDataType(), DataType::SI32);
  EXPECT_EQ(addOp.getInputsNum(), 2);
  EXPECT_TRUE(addOp.hasInputs());

  std::string msg;
  bool vres = addOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, SUB) {
  arith::ConstantOp constOp1(makeConstAttribute(1));
  arith::ConstantOp constOp2(makeConstAttribute(2));

  arith::SubOp subOp(&constOp1, &constOp2);

  EXPECT_EQ(subOp.getDataType(), DataType::SI32);
  EXPECT_EQ(subOp.getInputsNum(), 2);
  EXPECT_TRUE(subOp.hasInputs());

  std::string msg;
  bool vres = subOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, MUL) {
  arith::ConstantOp constOp1(makeConstAttribute(1));
  arith::ConstantOp constOp2(makeConstAttribute(2));

  arith::MulOp mulOp(&constOp1, &constOp2);

  EXPECT_EQ(mulOp.getDataType(), DataType::SI32);
  EXPECT_EQ(mulOp.getInputsNum(), 2);
  EXPECT_TRUE(mulOp.hasInputs());

  std::string msg;
  bool vres = mulOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, DIV) {
  arith::ConstantOp constOp1(makeConstAttribute(1));
  arith::ConstantOp constOp2(makeConstAttribute(2));

  arith::DivOp divOp(&constOp1, &constOp2);

  EXPECT_EQ(divOp.getDataType(), DataType::SI32);
  EXPECT_EQ(divOp.getInputsNum(), 2);
  EXPECT_TRUE(divOp.hasInputs());

  std::string msg;
  bool vres = divOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, CAST) {
  arith::ConstantOp cstOp(makeConstAttribute(1));
  arith::CastOp castOp(DataType::F32, &cstOp);

  EXPECT_EQ(castOp.getDataType(), DataType::F32);
  EXPECT_EQ(castOp.getInputsNum(), 1);
  EXPECT_TRUE(castOp.hasInputs());
}

TEST(ARITH, COMPARE) {
  arith::ConstantOp constOp1(makeConstAttribute(1));
  arith::ConstantOp constOp2(makeConstAttribute(2));

  arith::CompareOp cmpOp(&constOp1, &constOp2, arith::CompareOp::Pred::EQ);

  EXPECT_EQ(cmpOp.getDataType(), DataType::BOOL);
  EXPECT_EQ(cmpOp.getInputsNum(), 2);
  EXPECT_TRUE(cmpOp.hasInputs());

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
