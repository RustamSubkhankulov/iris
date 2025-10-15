#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(BUILDER, DEFAULT) {
  IRBuilder builder;

  EXPECT_FALSE(builder.isRegionBuilding());
  EXPECT_FALSE(builder.isBasicBlockBuilding());

  EXPECT_EQ(builder.getCurBasicBlockID(), -1);
}

TEST(BUILDER, EXFAIL_GET_NO_REGION) {
  IRBuilder builder;

  try {
    [[maybe_unused]] auto& reg = builder.getCurRegion();
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(!str.compare("No region is building!"));
    return;
  }
  FAIL();
}

TEST(BUILDER, EXFAIL_DROP_NO_REGION) {
  IRBuilder builder;

  try {
    builder.dropCurRegion();
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(!str.compare("No region is building!"));
    return;
  }
  FAIL();
}

TEST(BUILDER, EXFAIL_OBTAIN_NO_REGION) {
  IRBuilder builder;

  try {
    [[maybe_unused]] auto reg = builder.obtainRegion();
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(!str.compare("No region is building!"));
    return;
  }
  FAIL();
}

TEST(BUILDER, EXFAIL_OBTAIN_BB_ID_NO_REGION) {
  IRBuilder builder;

  try {
    [[maybe_unused]] auto id = builder.obtainIdForBasicBlock();
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(!str.compare("No region is building!"));
    return;
  }
  FAIL();
}

TEST(BUILDER, EXFAIL_START_BB_NO_REGION) {
  IRBuilder builder;

  try {
    builder.startNewBasicBlock();
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(!str.compare("No region is building!"));
    return;
  }
  FAIL();
}

TEST(BUILDER, START_NEW_REGION) {
  IRBuilder builder;

  builder.startNewRegion("foo");
  EXPECT_TRUE(builder.isRegionBuilding());
  EXPECT_FALSE(builder.isBasicBlockBuilding());

  const auto& reg = builder.getCurRegion();
  EXPECT_TRUE(!reg.getName().compare("foo"));
}

TEST(BUILDER, EXFAIL_GET_NO_BB) {
  IRBuilder builder;
  builder.startNewRegion("foo");

  try {
    [[maybe_unused]] auto& bb = builder.getCurBasicBlock();
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(!str.compare("No basic block is building!"));
    return;
  }
}

TEST(BUILDER, EXFAIL_DROP_NO_BB) {
  IRBuilder builder;
  builder.startNewRegion("foo");

  try {
    builder.dropCurBasicBlock();
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(!str.compare("No basic block is building!"));
    return;
  }
}

TEST(BUILDER, EXFAIL_FINALIZE_NO_BB) {
  IRBuilder builder;
  builder.startNewRegion("foo");

  try {
    [[maybe_unused]] auto& bb = builder.finalizeCurBasicBlock();
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(!str.compare("No basic block is building!"));
    return;
  }
}

TEST(BUILDER, OBTAIN_REGION) {
  IRBuilder builder;
  builder.startNewRegion("foo");

  auto reg = builder.obtainRegion();
  EXPECT_TRUE(static_cast<bool>(reg));
  EXPECT_TRUE(!reg->getName().compare("foo"));

  EXPECT_FALSE(builder.isRegionBuilding());
}

TEST(BUILDER, OBTAIN_BB_ID) {
  IRBuilder builder;
  builder.startNewRegion("foo");

  auto id = builder.obtainIdForBasicBlock();

  EXPECT_EQ(builder.obtainIdForBasicBlock(), id + 1);
  EXPECT_EQ(builder.obtainIdForBasicBlock(), id + 2);
}

TEST(BUILDER, START_NEW_BB) {
  IRBuilder builder;
  builder.startNewRegion("foo");
  builder.startNewBasicBlock();

  EXPECT_TRUE(builder.isBasicBlockBuilding());

  const auto& bb = builder.getCurBasicBlock();
  EXPECT_EQ(builder.getCurBasicBlockID(), bb.getID());
}

TEST(BUILDER, SETTING_BB_ID) {
  IRBuilder builder;
  builder.startNewRegion("foo");

  auto id = builder.obtainIdForBasicBlock();
  builder.startNewBasicBlock(id);

  const auto& bb = builder.getCurBasicBlock();
  EXPECT_EQ(bb.getID(), id);
  EXPECT_EQ(builder.getCurBasicBlockID(), id);
}

TEST(BUILDER, DROP_BB) {
  IRBuilder builder;
  builder.startNewRegion("foo");
  builder.startNewBasicBlock();

  builder.dropCurBasicBlock();

  EXPECT_FALSE(builder.isBasicBlockBuilding());
}

TEST(BUILDER, FINALIZE_BB) {
  IRBuilder builder;
  builder.startNewRegion("foo");
  builder.startNewBasicBlock();

  auto& reg = builder.getCurRegion();
  auto& bb = builder.getCurBasicBlock();

  auto& result = builder.finalizeCurBasicBlock();

  EXPECT_EQ(&bb, &result);
  EXPECT_EQ(reg.getBasicBlocks().size(), 1);
  EXPECT_EQ(reg.getBasicBlocks().front().get(), &bb);

  EXPECT_FALSE(builder.isBasicBlockBuilding());
}

TEST(BUILDER, ADDING_OPS) {
  IRBuilder builder;
  builder.startNewRegion("foo");
  builder.startNewBasicBlock();

  auto* phiOp1 = builder.createAndAddOp<ctrlflow::PhiOp>(nullptr, nullptr);
  auto* phiOp2 = builder.createAndAddOp<ctrlflow::PhiOp>(nullptr, nullptr);

  auto* op1 = builder.createAndAddOp<builtin::ParamOp>(DataType::F32);
  auto* op2 = builder.createAndAddOp<builtin::ParamOp>(DataType::F32);
  auto* op3 = builder.createAndAddOp<builtin::ParamOp>(DataType::F32);

  auto& bb = builder.getCurBasicBlock();

  EXPECT_EQ(bb.getPhiOps().size(), 2);
  EXPECT_EQ(std::next(bb.getPhiOps().begin(), 0).get(), phiOp1);
  EXPECT_EQ(std::next(bb.getPhiOps().begin(), 1).get(), phiOp2);

  EXPECT_EQ(bb.getOps().size(), 3);
  EXPECT_EQ(std::next(bb.getOps().begin(), 0).get(), op1);
  EXPECT_EQ(std::next(bb.getOps().begin(), 1).get(), op2);
  EXPECT_EQ(std::next(bb.getOps().begin(), 2).get(), op3);
}
