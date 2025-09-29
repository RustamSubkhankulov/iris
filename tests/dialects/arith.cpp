#include "ops/dialects/arith/ops.hpp"
#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(ARITH, CMP_EXFAIL_NON_HOMOGEN_INPUTS1) {
  arith::ConstantOp cstI(makeConstAttribute(1));
  arith::ConstantOp cstF(makeConstAttribute(1.));

  arith::CmpOp cmp(&cstI, &cstF, arith::CmpOp::Pred::EQ);

  std::string msg;
  bool vres = cmp.verify(msg);

  EXPECT_EQ(vres, false);
  EXPECT_TRUE(msg.contains("inputs must have same data types"));
}

TEST(ARITH, CMP_EXFAIL_EMPTY_INPUT_1) {
  arith::ConstantOp cstI(makeConstAttribute(1));
  arith::CmpOp cmp(&cstI, nullptr, arith::CmpOp::Pred::EQ);

  std::string msg;
  bool vres = cmp.verify(msg);

  EXPECT_EQ(vres, false);
  EXPECT_TRUE(msg.contains("input #1 is empty!"));
}

TEST(ARITH, CMP_EXFAIL_EMPTY_INPUT_2) {
  arith::ConstantOp cstF(makeConstAttribute(1.));
  arith::CmpOp cmp(nullptr, &cstF, arith::CmpOp::Pred::EQ);

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
