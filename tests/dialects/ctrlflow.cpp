#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(CTRLFLOW, CALL_NO_INPUTS) {
  ctrlflow::CallOp callOp("foo", DataType::SI32, {});

  EXPECT_EQ(callOp.getDataType(), DataType::SI32);
  EXPECT_EQ(callOp.getInputsNum(), 0);
  EXPECT_FALSE(callOp.hasInputs());
  EXPECT_FALSE(callOp.isTerminator());

  std::string msg;
  bool vres = callOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(CTRLFLOW, CALL_ONE_INPUT) {
  builtin::ParamOp prmOp(DataType::F32);

  ctrlflow::CallOp callOp("foo", DataType::SI32, {&prmOp});

  EXPECT_EQ(callOp.getDataType(), DataType::SI32);
  EXPECT_EQ(callOp.getInputsNum(), 1);
  EXPECT_TRUE(callOp.hasInputs());
  EXPECT_FALSE(callOp.isTerminator());

  std::string msg;
  bool vres = callOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(CTRLFLOW, CALL_MULTIPLE_INPUT) {
  builtin::ParamOp prmOp1(DataType::F32);
  builtin::ParamOp prmOp2(DataType::UI64);

  ctrlflow::CallOp callOp("foo", DataType::SI32, {&prmOp1, &prmOp2});

  EXPECT_EQ(callOp.getDataType(), DataType::SI32);
  EXPECT_EQ(callOp.getInputsNum(), 2);
  EXPECT_TRUE(callOp.hasInputs());
  EXPECT_FALSE(callOp.isTerminator());

  std::string msg;
  bool vres = callOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(CTRLFLOW, CALL_NONE_RETURN_DATATYPE) {
  ctrlflow::CallOp callOp("foo", DataType::NONE, {});

  EXPECT_EQ(callOp.getDataType(), DataType::NONE);
  EXPECT_EQ(callOp.getInputsNum(), 0);
  EXPECT_FALSE(callOp.hasInputs());
  EXPECT_FALSE(callOp.isTerminator());

  std::string msg;
  bool vres = callOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(CTRLFLOW, JUMP) {
  ctrlflow::JumpOp jumpOp;

  EXPECT_EQ(jumpOp.getDataType(), DataType::NONE);
  EXPECT_EQ(jumpOp.getInputsNum(), 0);
  EXPECT_FALSE(jumpOp.hasInputs());
  EXPECT_TRUE(jumpOp.isTerminator());

  std::string msg;
  bool vres = jumpOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(CTRLFLOW, JUMPC) {
  builtin::ParamOp prmOp(DataType::BOOL);

  ctrlflow::JumpcOp jumpcOp(&prmOp);

  EXPECT_EQ(jumpcOp.getDataType(), DataType::NONE);
  EXPECT_EQ(jumpcOp.getInputsNum(), 1);
  EXPECT_TRUE(jumpcOp.hasInputs());
  EXPECT_TRUE(jumpcOp.isTerminator());

  std::string msg;
  bool vres = jumpcOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(CTRLFLOW, RETURN_NO_INPUT) {
  ctrlflow::ReturnOp retOp;

  EXPECT_EQ(retOp.getDataType(), DataType::NONE);
  EXPECT_EQ(retOp.getInputsNum(), 0);
  EXPECT_FALSE(retOp.hasInputs());
  EXPECT_TRUE(retOp.isTerminator());

  std::string msg;
  bool vres = retOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(CTRLFLOW, RETURN_WITH_INPUT) {
  builtin::ParamOp prmOp(DataType::SI32);

  ctrlflow::ReturnOp retOp(&prmOp);

  EXPECT_EQ(retOp.getDataType(), DataType::NONE);
  EXPECT_EQ(retOp.getInputsNum(), 1);
  EXPECT_TRUE(retOp.hasInputs());
  EXPECT_TRUE(retOp.isTerminator());

  std::string msg;
  bool vres = retOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(CTRLFLOW, PHI_ONE_INPUT) {
  builtin::ParamOp prmOp(DataType::SI32);

  ctrlflow::PhiOp phiOp({&prmOp});

  EXPECT_EQ(phiOp.getDataType(), DataType::SI32);
  EXPECT_EQ(phiOp.getInputsNum(), 1);
  EXPECT_TRUE(phiOp.hasInputs());
  EXPECT_FALSE(phiOp.isTerminator());

  std::string msg;
  bool vres = phiOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(CTRLFLOW, PHI_TWO_INPUTS) {
  builtin::ParamOp prmOp1(DataType::SI32);
  builtin::ParamOp prmOp2(DataType::SI32);

  ctrlflow::PhiOp phiOp(&prmOp1, &prmOp2);

  EXPECT_EQ(phiOp.getDataType(), DataType::SI32);
  EXPECT_EQ(phiOp.getInputsNum(), 2);
  EXPECT_TRUE(phiOp.hasInputs());
  EXPECT_FALSE(phiOp.isTerminator());

  std::string msg;
  bool vres = phiOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(CTRLFLOW, PHI_MULTIPLE_INPUTS) {
  builtin::ParamOp prmOp1(DataType::SI32);
  builtin::ParamOp prmOp2(DataType::SI32);
  builtin::ParamOp prmOp3(DataType::SI32);

  ctrlflow::PhiOp phiOp({&prmOp1, &prmOp2, &prmOp3});

  EXPECT_EQ(phiOp.getDataType(), DataType::SI32);
  EXPECT_EQ(phiOp.getInputsNum(), 3);
  EXPECT_TRUE(phiOp.hasInputs());
  EXPECT_FALSE(phiOp.isTerminator());

  std::string msg;
  bool vres = phiOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(CTRLFLOW, JUMPC_EXFAIL_NON_BOOLEAN_INPUT) {
  arith::ConstantOp cstF(makeConstAttribute(1.));

  ctrlflow::JumpcOp jumpc(&cstF);

  std::string msg;
  bool vres = jumpc.verify(msg);

  EXPECT_EQ(vres, false);
  EXPECT_TRUE(msg.contains("Input's data type must be bool"));
}

TEST(CTRLFLOW, CALL_EXPECTED_EXCEPTION_INVALID_FUNC_NAME) {
  try {
    ctrlflow::CallOp call("", DataType::NONE);
  } catch (const IrisException& exc) {
    std::string msg = exc.what();
    EXPECT_TRUE(!msg.compare("Invalid function name!"));
    return;
  }
  FAIL();
}

TEST(CTRLFLOW, PHI_EXPECTED_EXCEPTION_EMPTY_INPUTS) {
  try {
    ctrlflow::PhiOp phi(InputList{});
  } catch (const IrisException& exc) {
    std::string msg = exc.what();
    EXPECT_TRUE(!msg.compare("Operation must have at least one input!"));
    return;
  }
  FAIL();
}

TEST(CTRLFLOW, PHI_EXFAIL_NON_HOMOGEN_INPUTS) {
  arith::ConstantOp cstI(makeConstAttribute(1));
  arith::ConstantOp cstF(makeConstAttribute(1.));

  ctrlflow::PhiOp phi(&cstI, &cstF);

  std::string msg;
  bool vres = phi.verify(msg);

  EXPECT_EQ(vres, false);
  EXPECT_TRUE(msg.contains("inputs must have same data types"));
}
