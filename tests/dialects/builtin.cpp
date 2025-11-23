#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(BUILTIN, PARAM_INVARIANTS) {
  builtin::ParamOp op(DataType::SINT);

  EXPECT_FALSE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 0u);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::PARAM));
}

TEST(BUILTIN, PARAM_BASIC) {
  builtin::ParamOp prmOp(DataType::SINT);

  EXPECT_TRUE(prmOp.hasResult());
  EXPECT_EQ(prmOp.getDataType(), DataType::SINT);

  std::string msg;
  bool vres = prmOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(BUILTIN, COPY_INVARIANTS) {
  builtin::CopyOp op(nullptr);

  EXPECT_TRUE(op.hasInputs());
  EXPECT_EQ(op.getInputsNum(), 1u);

  EXPECT_FALSE(op.isTerminator());
  EXPECT_FALSE(op.hasSideEffects());
  EXPECT_TRUE(op.isa(GlobalOpcodes::COPY));
}

TEST(BUILTIN, COPY) {
  builtin::ParamOp prmOp(DataType::SINT);
  builtin::CopyOp copyOp(&prmOp);

  EXPECT_TRUE(copyOp.hasResult());
  EXPECT_EQ(copyOp.getDataType(), DataType::SINT);

  std::string msg;
  bool vres = copyOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}
