#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(CTRLFLOW, JUMPC_EXFAIL_NON_BOOLEAN_INPUT) {
  bb_id_t targetID = 0;
  arith::ConstantOp cstF(makeConstAttribute(1.));

  ctrlflow::JumpcOp jumpc(targetID, &cstF);

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
