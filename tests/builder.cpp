#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(BUILDER, DEFAULT) {
  IRBuilder builder;

  EXPECT_FALSE(builder.isRegionBuilding());
  EXPECT_FALSE(builder.isBasicBlockBuilding());

  EXPECT_EQ(-1, builder.getCurBasicBlockID());
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
  ASSERT_TRUE(builder.isRegionBuilding());
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
  FAIL();
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
  FAIL();
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
  FAIL();
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

  EXPECT_EQ(id + 1, builder.obtainIdForBasicBlock());
  EXPECT_EQ(id + 2, builder.obtainIdForBasicBlock());
}

TEST(BUILDER, START_NEW_BB) {
  IRBuilder builder;
  builder.startNewRegion("foo");
  builder.startNewBasicBlock();

  ASSERT_TRUE(builder.isBasicBlockBuilding());

  const auto& bb = builder.getCurBasicBlock();
  EXPECT_EQ(bb.getID(), builder.getCurBasicBlockID());
}

TEST(BUILDER, SETTING_BB_ID) {
  IRBuilder builder;
  builder.startNewRegion("foo");

  auto id = builder.obtainIdForBasicBlock();
  builder.startNewBasicBlock(id);

  const auto& bb = builder.getCurBasicBlock();
  EXPECT_EQ(id, bb.getID());
  EXPECT_EQ(id, builder.getCurBasicBlockID());
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
  EXPECT_EQ(1u, reg.getBasicBlocks().size());
  EXPECT_EQ(&bb, reg.getBasicBlocks().front().get());

  EXPECT_FALSE(builder.isBasicBlockBuilding());
}

TEST(BUILDER, ADDING_OPS) {
  IRBuilder builder;
  builder.startNewRegion("foo");
  builder.startNewBasicBlock();

  auto* phiOp1 = builder.createAndAddOp<ctrlflow::PhiOp>(nullptr, nullptr);
  auto* phiOp2 = builder.createAndAddOp<ctrlflow::PhiOp>(nullptr, nullptr);

  auto* op1 = builder.createAndAddOp<builtin::ParamOp>(DataType::FLOAT);
  auto* op2 = builder.createAndAddOp<builtin::ParamOp>(DataType::FLOAT);
  auto* op3 = builder.createAndAddOp<builtin::ParamOp>(DataType::FLOAT);

  auto& bb = builder.getCurBasicBlock();

  ASSERT_EQ(2u, bb.getPhiOps().size());
  EXPECT_EQ(phiOp1, std::next(bb.getPhiOps().begin(), 0).get());
  EXPECT_EQ(phiOp2, std::next(bb.getPhiOps().begin(), 1).get());

  ASSERT_EQ(3u, bb.getOps().size());
  EXPECT_EQ(op1, std::next(bb.getOps().begin(), 0).get());
  EXPECT_EQ(op2, std::next(bb.getOps().begin(), 1).get());
  EXPECT_EQ(op3, std::next(bb.getOps().begin(), 2).get());
}
