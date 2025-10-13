#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(BUILTIN, PARAM) {
  builtin::ParamOp prmOp(DataType::SI32);

  EXPECT_EQ(prmOp.getDataType(), DataType::SI32);
  EXPECT_EQ(prmOp.getInputsNum(), 0);
  EXPECT_FALSE(prmOp.hasInputs());

  std::string msg;
  bool vres = prmOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}

TEST(BUILTIN, COPY) {
  builtin::ParamOp prmOp(DataType::SI32);
  builtin::CopyOp copyOp(&prmOp);

  EXPECT_EQ(copyOp.getDataType(), DataType::SI32);
  EXPECT_EQ(copyOp.getInputsNum(), 1);
  EXPECT_TRUE(copyOp.hasInputs());

  std::string msg;
  bool vres = copyOp.verify(msg);

  EXPECT_TRUE(vres);
  EXPECT_TRUE(msg.empty());
}
