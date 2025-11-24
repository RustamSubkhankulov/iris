#include <gtest/gtest.h>
#include <string>

#include <iris.hpp>
using namespace iris;

TEST(ARITH, CONSTANT_INVARIANTS) {
  arith::ConstantOp op(makeConstAttribute(1));

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::CONSTANT));

  EXPECT_FALSE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 0u);
}

TEST(ARITH, ADD_INVARIANTS) {
  arith::AddOp op(nullptr, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::ADD));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2u);
}

TEST(ARITH, SUB_INVARIANTS) {
  arith::SubOp op(nullptr, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::SUB));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2u);
}

TEST(ARITH, MUL_INVARIANTS) {
  arith::MulOp op(nullptr, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::MUL));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2u);
}

TEST(ARITH, DIV_INVARIANTS) {
  arith::DivOp op(nullptr, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::DIV));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2u);
}

TEST(ARITH, CAST_INVARIANTS) {
  arith::CastOp op(DataType::FLOAT, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::CAST));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 1u);
}

TEST(ARITH, COMPARE_INVARIANTS) {
  arith::CompareOp op(nullptr, nullptr, arith::CompareOp::Pred::EQ);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::COMPARE));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2u);
}

TEST(ARITH, AND_INVARIANTS) {
  arith::AndOp op(nullptr, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::AND));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2u);
}

TEST(ARITH, OR_INVARIANTS) {
  arith::OrOp op(nullptr, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::OR));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2u);
}

TEST(ARITH, XOR_INVARIANTS) {
  arith::XorOp op(nullptr, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::XOR));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2u);
}

TEST(ARITH, NOT_INVARIANTS) {
  arith::NotOp op(nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::NOT));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 1u);
}

TEST(ARITH, SAL_INVARIANTS) {
  arith::SalOp op(nullptr, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::SAL));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2u);
}

TEST(ARITH, SAR_INVARIANTS) {
  arith::SarOp op(nullptr, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::SAR));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2u);
}

TEST(ARITH, SHL_INVARIANTS) {
  arith::ShlOp op(nullptr, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::SHL));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2u);
}

TEST(ARITH, SHR_INVARIANTS) {
  arith::ShrOp op(nullptr, nullptr);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::SHR));

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 2u);
}

TEST(ARITH, CONSTANT_BASIC) {
  arith::ConstantOp constOp(makeConstAttribute(1));

  EXPECT_TRUE(constOp.hasResult());
  EXPECT_EQ(constOp.getDataType(), DataType::SINT);

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
  EXPECT_EQ(addOp.getDataType(), DataType::SINT);

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
  EXPECT_EQ(subOp.getDataType(), DataType::SINT);

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
  EXPECT_EQ(mulOp.getDataType(), DataType::SINT);

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
  EXPECT_EQ(divOp.getDataType(), DataType::SINT);

  std::string msg;
  bool vres = divOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, CAST_BASIC) {
  arith::ConstantOp cstOp(makeConstAttribute(1));
  arith::CastOp castOp(DataType::FLOAT, &cstOp);

  EXPECT_TRUE(castOp.hasResult());
  EXPECT_EQ(castOp.getDataType(), DataType::FLOAT);

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

TEST(ARITH, AND_BASIC) {
  arith::ConstantOp constOp1(makeConstAttribute(1));
  arith::ConstantOp constOp2(makeConstAttribute(2));

  arith::AndOp andOp(&constOp1, &constOp2);

  EXPECT_TRUE(andOp.hasResult());
  EXPECT_EQ(andOp.getDataType(), DataType::SINT);

  std::string msg;
  bool vres = andOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, OR_BASIC) {
  arith::ConstantOp constOp1(makeConstAttribute(1));
  arith::ConstantOp constOp2(makeConstAttribute(2));

  arith::OrOp orOp(&constOp1, &constOp2);

  EXPECT_TRUE(orOp.hasResult());
  EXPECT_EQ(orOp.getDataType(), DataType::SINT);

  std::string msg;
  bool vres = orOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, XOR_BASIC) {
  arith::ConstantOp constOp1(makeConstAttribute(1));
  arith::ConstantOp constOp2(makeConstAttribute(2));

  arith::XorOp xorOp(&constOp1, &constOp2);

  EXPECT_TRUE(xorOp.hasResult());
  EXPECT_EQ(xorOp.getDataType(), DataType::SINT);

  std::string msg;
  bool vres = xorOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, NOT_BASIC) {
  arith::ConstantOp constOp(makeConstAttribute(1));

  arith::NotOp notOp(&constOp);

  EXPECT_TRUE(notOp.hasResult());
  EXPECT_EQ(notOp.getDataType(), DataType::SINT);

  std::string msg;
  bool vres = notOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, SAL_BASIC) {
  arith::ConstantOp constOp1(makeConstAttribute(1));
  arith::ConstantOp constOp2(makeConstAttribute(2));

  arith::SalOp salOp(&constOp1, &constOp2);

  EXPECT_TRUE(salOp.hasResult());
  EXPECT_EQ(salOp.getDataType(), DataType::SINT);

  std::string msg;
  bool vres = salOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, SAR_BASIC) {
  arith::ConstantOp constOp1(makeConstAttribute(1));
  arith::ConstantOp constOp2(makeConstAttribute(2));

  arith::SarOp sarOp(&constOp1, &constOp2);

  EXPECT_TRUE(sarOp.hasResult());
  EXPECT_EQ(sarOp.getDataType(), DataType::SINT);

  std::string msg;
  bool vres = sarOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, SHL_BASIC) {
  arith::ConstantOp constOp1(makeConstAttribute(static_cast<uint32_t>(1)));
  arith::ConstantOp constOp2(makeConstAttribute(static_cast<uint32_t>(2)));

  arith::ShlOp shlOp(&constOp1, &constOp2);

  EXPECT_TRUE(shlOp.hasResult());
  EXPECT_EQ(shlOp.getDataType(), constOp1.getDataType());

  std::string msg;
  bool vres = shlOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, SHR_BASIC) {
  arith::ConstantOp constOp1(makeConstAttribute(static_cast<uint32_t>(1)));
  arith::ConstantOp constOp2(makeConstAttribute(static_cast<uint32_t>(2)));

  arith::ShrOp shrOp(&constOp1, &constOp2);

  EXPECT_TRUE(shrOp.hasResult());
  EXPECT_EQ(shrOp.getDataType(), constOp1.getDataType());

  std::string msg;
  bool vres = shrOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(ARITH, COMPARE_EXFAIL_UNEXPECTED_PRED) {
  try {
    arith::CompareOp cmpOp(nullptr, nullptr,
                           static_cast<arith::CompareOp::Pred>(255U));
    FAIL() << "Expected IrisException for unexpected CompareOp predicate value";
  } catch (const IrisException& exc) {
    const std::string msg = exc.what();
    EXPECT_EQ(msg, "Unexpected predicate in CompareOp operation!");
  }
}

TEST(ARITH, COMPARE_EXFAIL_NON_HOMOGEN_INPUTS1) {
  arith::ConstantOp cstI(makeConstAttribute(1));
  arith::ConstantOp cstF(makeConstAttribute(1.));

  arith::CompareOp cmp(&cstI, &cstF, arith::CompareOp::Pred::EQ);

  std::string msg;
  bool vres = cmp.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs must have same data types"));
}

TEST(ARITH, COMPARE_EXFAIL_EMPTY_INPUT_1) {
  arith::ConstantOp cstI(makeConstAttribute(1));
  arith::CompareOp cmp(&cstI, nullptr, arith::CompareOp::Pred::EQ);

  std::string msg;
  bool vres = cmp.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("input #1 is empty!"));
}

TEST(ARITH, COMPARE_EXFAIL_EMPTY_INPUT_2) {
  arith::ConstantOp cstF(makeConstAttribute(1.));
  arith::CompareOp cmp(nullptr, &cstF, arith::CompareOp::Pred::EQ);

  std::string msg;
  bool vres = cmp.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("input #0 is empty!"));
}

TEST(ARITH, ADD_EXFAIL_NON_HOMOGEN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(1));
  arith::ConstantOp cstF(makeConstAttribute(1.));

  arith::AddOp add(&cstI, &cstF);

  std::string msg;
  bool vres = add.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs must have same data types"));
}

TEST(ARITH, ADD_EXFAIL_BOOLEAN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(true));
  arith::ConstantOp cstF(makeConstAttribute(false));

  arith::AddOp add(&cstI, &cstF);

  std::string msg;
  bool vres = add.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs cannot have boolean type"));
}

TEST(ARITH, SUB_EXFAIL_NON_HOMOGEN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(1));
  arith::ConstantOp cstF(makeConstAttribute(1.));

  arith::SubOp sub(&cstI, &cstF);

  std::string msg;
  bool vres = sub.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs must have same data types"));
}

TEST(ARITH, SUB_EXFAIL_BOOLEAN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(true));
  arith::ConstantOp cstF(makeConstAttribute(false));

  arith::SubOp sub(&cstI, &cstF);

  std::string msg;
  bool vres = sub.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs cannot have boolean type"));
}

TEST(ARITH, MUL_EXFAIL_NON_HOMOGEN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(1));
  arith::ConstantOp cstF(makeConstAttribute(1.));

  arith::MulOp mul(&cstI, &cstF);

  std::string msg;
  bool vres = mul.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs must have same data types"));
}

TEST(ARITH, MUL_EXFAIL_BOOLEAN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(true));
  arith::ConstantOp cstF(makeConstAttribute(false));

  arith::MulOp mul(&cstI, &cstF);

  std::string msg;
  bool vres = mul.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs cannot have boolean type"));
}

TEST(ARITH, DIV_EXFAIL_NON_HOMOGEN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(1));
  arith::ConstantOp cstF(makeConstAttribute(1.));

  arith::DivOp div(&cstI, &cstF);

  std::string msg;
  bool vres = div.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs must have same data types"));
}

TEST(ARITH, DIV_EXFAIL_BOOLEAN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(true));
  arith::ConstantOp cstF(makeConstAttribute(false));

  arith::DivOp div(&cstI, &cstF);

  std::string msg;
  bool vres = div.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs cannot have boolean type"));
}

TEST(ARITH, AND_EXFAIL_BOOLEAN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(true));
  arith::ConstantOp cstF(makeConstAttribute(false));

  arith::AndOp op(&cstI, &cstF);

  std::string msg;
  bool vres = op.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs cannot have boolean type"));
}

TEST(ARITH, AND_EXFAIL_NON_INTEGER_INPUTS) {
  arith::ConstantOp cstF1(makeConstAttribute(1.));
  arith::ConstantOp cstF2(makeConstAttribute(2.));

  arith::AndOp op(&cstF1, &cstF2);

  std::string msg;
  bool vres = op.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs must be integer types"));
}

TEST(ARITH, OR_EXFAIL_BOOLEAN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(true));
  arith::ConstantOp cstF(makeConstAttribute(false));

  arith::OrOp op(&cstI, &cstF);

  std::string msg;
  bool vres = op.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs cannot have boolean type"));
}

TEST(ARITH, OR_EXFAIL_NON_INTEGER_INPUTS) {
  arith::ConstantOp cstF1(makeConstAttribute(1.));
  arith::ConstantOp cstF2(makeConstAttribute(2.));

  arith::OrOp op(&cstF1, &cstF2);

  std::string msg;
  bool vres = op.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs must be integer types"));
}

TEST(ARITH, XOR_EXFAIL_BOOLEAN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(true));
  arith::ConstantOp cstF(makeConstAttribute(false));

  arith::XorOp op(&cstI, &cstF);

  std::string msg;
  bool vres = op.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs cannot have boolean type"));
}

TEST(ARITH, XOR_EXFAIL_NON_INTEGER_INPUTS) {
  arith::ConstantOp cstF1(makeConstAttribute(1.));
  arith::ConstantOp cstF2(makeConstAttribute(2.));

  arith::XorOp op(&cstF1, &cstF2);

  std::string msg;
  bool vres = op.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs must be integer types"));
}

TEST(ARITH, NOT_EXFAIL_NON_INTEGER_INPUT) {
  arith::ConstantOp cstF(makeConstAttribute(1.));

  arith::NotOp op(&cstF);

  std::string msg;
  bool vres = op.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("input must be an integer type"));
}

TEST(ARITH, SAL_EXFAIL_BOOLEAN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(true));
  arith::ConstantOp cstF(makeConstAttribute(false));

  arith::SalOp op(&cstI, &cstF);

  std::string msg;
  bool vres = op.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs cannot have boolean type"));
}

TEST(ARITH, SAL_EXFAIL_UNSIGNED_INPUTS) {
  arith::ConstantOp cstU1(makeConstAttribute(static_cast<uint32_t>(1)));
  arith::ConstantOp cstU2(makeConstAttribute(static_cast<uint32_t>(2)));

  arith::SalOp op(&cstU1, &cstU2);

  std::string msg;
  bool vres = op.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs must be signed integer types"));
}

TEST(ARITH, SAR_EXFAIL_BOOLEAN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(true));
  arith::ConstantOp cstF(makeConstAttribute(false));

  arith::SarOp op(&cstI, &cstF);

  std::string msg;
  bool vres = op.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs cannot have boolean type"));
}

TEST(ARITH, SAR_EXFAIL_UNSIGNED_INPUTS) {
  arith::ConstantOp cstU1(makeConstAttribute(static_cast<uint32_t>(1)));
  arith::ConstantOp cstU2(makeConstAttribute(static_cast<uint32_t>(2)));

  arith::SarOp op(&cstU1, &cstU2);

  std::string msg;
  bool vres = op.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs must be signed integer types"));
}

TEST(ARITH, SHL_EXFAIL_BOOLEAN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(true));
  arith::ConstantOp cstF(makeConstAttribute(false));

  arith::ShlOp op(&cstI, &cstF);

  std::string msg;
  bool vres = op.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs cannot have boolean type"));
}

TEST(ARITH, SHL_EXFAIL_SIGNED_INPUTS) {
  arith::ConstantOp cstI1(makeConstAttribute(1));
  arith::ConstantOp cstI2(makeConstAttribute(2));

  arith::ShlOp op(&cstI1, &cstI2);

  std::string msg;
  bool vres = op.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs must be unsigned integer types"));
}

TEST(ARITH, SHR_EXFAIL_BOOLEAN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(true));
  arith::ConstantOp cstF(makeConstAttribute(false));

  arith::ShrOp op(&cstI, &cstF);

  std::string msg;
  bool vres = op.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs cannot have boolean type"));
}

TEST(ARITH, SHR_EXFAIL_SIGNED_INPUTS) {
  arith::ConstantOp cstI1(makeConstAttribute(1));
  arith::ConstantOp cstI2(makeConstAttribute(2));

  arith::ShrOp op(&cstI1, &cstI2);

  std::string msg;
  bool vres = op.verify(msg);

  EXPECT_FALSE(vres);
  EXPECT_TRUE(msg.contains("inputs must be unsigned integer types"));
}
