#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(REGION, DEFAULT) {
  Region region("foo");

  EXPECT_EQ(region.getBasicBlocks().size(), 0);

  EXPECT_FALSE(region.hasStartBasicBlock());
  EXPECT_FALSE(region.hasFinalBasicBlock());

  EXPECT_EQ(region.getStartBasicBlock(), nullptr);
  EXPECT_EQ(region.getFinalBasicBlock(), nullptr);
}

TEST(REGION, NAMING) {
  Region region("foo");
  EXPECT_EQ(region.getName(), "foo");
}

TEST(REGION, EXFAIL_INVALID_NAME) {
  try {
    Region region("");
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(!str.compare("Region is assigned with an empty name!"));
    return;
  }
  FAIL();
}

TEST(REGION, BASIC_BLOCK_ID_PROVIDERS) {
  Region region("foo");

  EXPECT_EQ(region.obtainIDForBasicBlock(), 0);
  EXPECT_EQ(region.obtainIDForBasicBlock(), 1);
  EXPECT_EQ(region.obtainIDForBasicBlock(), 2);
}

TEST(REGION, OPERATION_ID_PROVIDERS) {
  Region region("foo");

  EXPECT_EQ(region.obtainIDForOperation(), 0);
  EXPECT_EQ(region.obtainIDForOperation(), 1);
  EXPECT_EQ(region.obtainIDForOperation(), 2);
}

TEST(REGION, ADD_BASIC_BLOCK) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  auto bb1Ptr = bb1.get();

  region.addBasicBlock(std::move(bb1));

  EXPECT_TRUE(bb1Ptr->hasParentRegion());
  EXPECT_EQ(bb1Ptr->getParentRegion(), &region);

  EXPECT_EQ(region.getBasicBlocks().size(), 1);
  EXPECT_EQ(region.getBasicBlocks().front().get(), bb1Ptr);

  EXPECT_FALSE(region.hasStartBasicBlock());
  EXPECT_FALSE(region.hasFinalBasicBlock());

  auto bb2 = std::make_unique<BasicBlock>(0);
  auto bb2Ptr = bb2.get();

  region.addBasicBlock(std::move(bb2));

  EXPECT_TRUE(bb2Ptr->hasParentRegion());
  EXPECT_EQ(bb2Ptr->getParentRegion(), &region);

  EXPECT_EQ(region.getBasicBlocks().size(), 2);
  EXPECT_EQ(region.getBasicBlocks().front().get(), bb1Ptr);
  EXPECT_EQ(region.getBasicBlocks().back().get(), bb2Ptr);

  EXPECT_FALSE(region.hasStartBasicBlock());
  EXPECT_FALSE(region.hasFinalBasicBlock());
}

TEST(REGION, ADD_START_BB) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  auto bb1Ptr = bb1.get();

  region.addStartBasicBlock(std::move(bb1));

  EXPECT_TRUE(bb1Ptr->hasParentRegion());
  EXPECT_EQ(bb1Ptr->getParentRegion(), &region);

  EXPECT_EQ(region.getBasicBlocks().size(), 1);
  EXPECT_EQ(region.getBasicBlocks().front().get(), bb1Ptr);

  EXPECT_TRUE(region.hasStartBasicBlock());
  EXPECT_FALSE(region.hasFinalBasicBlock());

  EXPECT_EQ(region.getStartBasicBlock(), bb1Ptr);
  EXPECT_EQ(region.getFinalBasicBlock(), nullptr);

  auto bb2 = std::make_unique<BasicBlock>(0);
  auto bb2Ptr = bb2.get();

  region.addStartBasicBlock(std::move(bb2));

  EXPECT_TRUE(bb2Ptr->hasParentRegion());
  EXPECT_EQ(bb2Ptr->getParentRegion(), &region);

  EXPECT_EQ(region.getBasicBlocks().size(), 2);
  EXPECT_EQ(region.getBasicBlocks().front().get(), bb1Ptr);
  EXPECT_EQ(region.getBasicBlocks().back().get(), bb2Ptr);

  EXPECT_TRUE(region.hasStartBasicBlock());
  EXPECT_FALSE(region.hasFinalBasicBlock());

  EXPECT_EQ(region.getStartBasicBlock(), bb2Ptr);
  EXPECT_EQ(region.getFinalBasicBlock(), nullptr);
}

TEST(REGION, ADD_FINAL_BB) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  auto bb1Ptr = bb1.get();

  region.addFinalBasicBlock(std::move(bb1));

  EXPECT_TRUE(bb1Ptr->hasParentRegion());
  EXPECT_EQ(bb1Ptr->getParentRegion(), &region);

  EXPECT_EQ(region.getBasicBlocks().size(), 1);
  EXPECT_EQ(region.getBasicBlocks().front().get(), bb1Ptr);

  EXPECT_FALSE(region.hasStartBasicBlock());
  EXPECT_TRUE(region.hasFinalBasicBlock());

  EXPECT_EQ(region.getStartBasicBlock(), nullptr);
  EXPECT_EQ(region.getFinalBasicBlock(), bb1Ptr);

  auto bb2 = std::make_unique<BasicBlock>(0);
  auto bb2Ptr = bb2.get();

  region.addFinalBasicBlock(std::move(bb2));

  EXPECT_TRUE(bb2Ptr->hasParentRegion());
  EXPECT_EQ(bb2Ptr->getParentRegion(), &region);

  EXPECT_EQ(region.getBasicBlocks().size(), 2);
  EXPECT_EQ(region.getBasicBlocks().front().get(), bb1Ptr);
  EXPECT_EQ(region.getBasicBlocks().back().get(), bb2Ptr);

  EXPECT_FALSE(region.hasStartBasicBlock());
  EXPECT_TRUE(region.hasFinalBasicBlock());

  EXPECT_EQ(region.getStartBasicBlock(), nullptr);
  EXPECT_EQ(region.getFinalBasicBlock(), bb2Ptr);
}

TEST(REGION, SET_START_BB) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  auto bb1Ptr = bb1.get();

  region.addBasicBlock(std::move(bb1));
  auto res = region.setStartBasicBlock(bb1Ptr);

  EXPECT_TRUE(res);
  EXPECT_TRUE(region.hasStartBasicBlock());
  EXPECT_EQ(region.getStartBasicBlock(), bb1Ptr);
}

TEST(REGION, SET_START_BB_BY_ID) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  auto bb1Ptr = bb1.get();

  region.addBasicBlock(std::move(bb1));
  auto res = region.setStartBasicBlockByID(0);

  EXPECT_TRUE(res);
  EXPECT_TRUE(region.hasStartBasicBlock());
  EXPECT_EQ(region.getStartBasicBlock(), bb1Ptr);
}

TEST(REGION, EXFAIL_SET_START_BB_NOT_PRESENT) {
  Region region("foo");

  BasicBlock bb;
  auto res = region.setStartBasicBlock(&bb);

  EXPECT_FALSE(res);
  EXPECT_FALSE(region.hasStartBasicBlock());
  EXPECT_EQ(region.getStartBasicBlock(), nullptr);
}

TEST(REGION, EXFAIL_SET_START_BB_BY_ID_NOT_PRESENT) {
  Region region("foo");

  auto res = region.setStartBasicBlockByID(0);

  EXPECT_FALSE(res);
  EXPECT_FALSE(region.hasStartBasicBlock());
  EXPECT_EQ(region.getStartBasicBlock(), nullptr);
}

TEST(REGION, SET_FINAL_BB) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  auto bb1Ptr = bb1.get();

  region.addBasicBlock(std::move(bb1));
  auto res = region.setFinalBasicBlock(bb1Ptr);

  EXPECT_TRUE(res);
  EXPECT_TRUE(region.hasFinalBasicBlock());
  EXPECT_EQ(region.getFinalBasicBlock(), bb1Ptr);
}

TEST(REGION, SET_FINAL_BB_BY_ID) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  auto bb1Ptr = bb1.get();

  region.addBasicBlock(std::move(bb1));
  auto res = region.setFinalBasicBlockByID(0);

  EXPECT_TRUE(res);
  EXPECT_TRUE(region.hasFinalBasicBlock());
  EXPECT_EQ(region.getFinalBasicBlock(), bb1Ptr);
}

TEST(REGION, EXFAIL_SET_FINAL_BB_NOT_PRESENT) {
  Region region("foo");

  BasicBlock bb;
  auto res = region.setFinalBasicBlock(&bb);

  EXPECT_FALSE(res);
  EXPECT_FALSE(region.hasFinalBasicBlock());
  EXPECT_EQ(region.getFinalBasicBlock(), nullptr);
}

TEST(REGION, EXFAIL_SET_FINAL_BB_BY_ID_NOT_PRESENT) {
  Region region("foo");

  auto res = region.setFinalBasicBlockByID(0);

  EXPECT_FALSE(res);
  EXPECT_FALSE(region.hasFinalBasicBlock());
  EXPECT_EQ(region.getFinalBasicBlock(), nullptr);
}

TEST(REGION, GET_BB_BY_ID) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  auto bb1Ptr = bb1.get();

  region.addBasicBlock(std::move(bb1));

  EXPECT_EQ(region.getBasicBlockByID(0), bb1Ptr);
}

TEST(REGION, IS_BB_PRESENT) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  auto bb1Ptr = bb1.get();

  BasicBlock bb;

  region.addBasicBlock(std::move(bb1));

  EXPECT_TRUE(region.isBasicBlockPresent(bb1Ptr));
  EXPECT_FALSE(region.isBasicBlockPresent(&bb));
}

TEST(REGION, IS_BB_PRESENT_BY_ID) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  region.addBasicBlock(std::move(bb1));

  EXPECT_TRUE(region.isBasicBlockPresentByID(0));
  EXPECT_FALSE(region.isBasicBlockPresentByID(1));
}

TEST(REGION, REMOVE_BB) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  auto bb1Ptr = bb1.get();

  region.addBasicBlock(std::move(bb1));
  auto res = region.removeBasicBlock(bb1Ptr);

  EXPECT_TRUE(res);
  EXPECT_EQ(region.getBasicBlocks().size(), 0);
}

TEST(REGION, EXFAIL_REMOVE_BB_NOT_PRESENT) {
  Region region("foo");

  BasicBlock bb;
  auto res = region.removeBasicBlock(&bb);

  EXPECT_FALSE(res);
  EXPECT_EQ(region.getBasicBlocks().size(), 0);
}

TEST(REGION, REMOVE_START_BB) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  auto bb1Ptr = bb1.get();

  region.addStartBasicBlock(std::move(bb1));

  EXPECT_TRUE(region.hasStartBasicBlock());
  EXPECT_EQ(region.getStartBasicBlock(), bb1Ptr);

  auto res = region.removeBasicBlock(bb1Ptr);

  EXPECT_TRUE(res);
  EXPECT_EQ(region.getBasicBlocks().size(), 0);
  EXPECT_FALSE(region.hasStartBasicBlock());
  EXPECT_EQ(region.getStartBasicBlock(), nullptr);
}

TEST(REGION, REMOVE_FINAL_BB) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  auto bb1Ptr = bb1.get();

  region.addFinalBasicBlock(std::move(bb1));

  EXPECT_TRUE(region.hasFinalBasicBlock());
  EXPECT_EQ(region.getFinalBasicBlock(), bb1Ptr);

  auto res = region.removeBasicBlock(bb1Ptr);

  EXPECT_TRUE(res);
  EXPECT_EQ(region.getBasicBlocks().size(), 0);
  EXPECT_FALSE(region.hasFinalBasicBlock());
  EXPECT_EQ(region.getFinalBasicBlock(), nullptr);
}

TEST(REGION, REMOVE_BB_BY_ID) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);

  region.addBasicBlock(std::move(bb1));
  auto res = region.removeBasicBlockByID(0);

  EXPECT_TRUE(res);
  EXPECT_EQ(region.getBasicBlocks().size(), 0);
}

TEST(REGION, EXFAIL_REMOVE_BB_BY_ID_NOT_PRESENT) {
  Region region("foo");

  auto res = region.removeBasicBlockByID(0);

  EXPECT_FALSE(res);
  EXPECT_EQ(region.getBasicBlocks().size(), 0);
}

TEST(REGION, REMOVE_START_BB_BY_ID) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  auto bb1Ptr = bb1.get();

  region.addStartBasicBlock(std::move(bb1));

  EXPECT_TRUE(region.hasStartBasicBlock());
  EXPECT_EQ(region.getStartBasicBlock(), bb1Ptr);

  auto res = region.removeBasicBlockByID(0);

  EXPECT_TRUE(res);
  EXPECT_EQ(region.getBasicBlocks().size(), 0);
  EXPECT_FALSE(region.hasStartBasicBlock());
  EXPECT_EQ(region.getStartBasicBlock(), nullptr);
}

TEST(REGION, REMOVE_FINAL_BB_BY_ID) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  auto bb1Ptr = bb1.get();

  region.addFinalBasicBlock(std::move(bb1));

  EXPECT_TRUE(region.hasFinalBasicBlock());
  EXPECT_EQ(region.getFinalBasicBlock(), bb1Ptr);

  auto res = region.removeBasicBlockByID(0);

  EXPECT_TRUE(res);
  EXPECT_EQ(region.getBasicBlocks().size(), 0);
  EXPECT_FALSE(region.hasFinalBasicBlock());
  EXPECT_EQ(region.getFinalBasicBlock(), nullptr);
}

TEST(REGION, REPLACE_BB) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  auto bb2 = std::make_unique<BasicBlock>(1);
  auto bb3 = std::make_unique<BasicBlock>(2);
  auto bb4 = std::make_unique<BasicBlock>(3);

  // bb1 -> bb2
  bb1->linkSucc(bb2.get());
  // bb2 -> bb3, bb4
  bb2->linkSucc(bb3.get(), true);
  bb2->linkSucc(bb4.get(), false);

  auto bb1Ptr = bb1.get();
  auto bb2Ptr = bb2.get();
  auto bb3Ptr = bb3.get();
  auto bb4Ptr = bb4.get();

  region.addBasicBlock(std::move(bb1));
  region.addBasicBlock(std::move(bb2));
  region.addBasicBlock(std::move(bb3));
  region.addBasicBlock(std::move(bb4));

  auto newBB = std::make_unique<BasicBlock>(4);
  auto newBBPtr = newBB.get();

  auto res = region.replaceBasicBlockWith(bb2Ptr, std::move(newBB));

  EXPECT_TRUE(res);

  EXPECT_TRUE(region.isBasicBlockPresent(bb1Ptr));
  EXPECT_TRUE(region.isBasicBlockPresent(bb3Ptr));
  EXPECT_TRUE(region.isBasicBlockPresent(bb4Ptr));

  EXPECT_FALSE(region.isBasicBlockPresent(bb2Ptr));
  EXPECT_TRUE(region.isBasicBlockPresent(newBBPtr));

  EXPECT_EQ(bb1Ptr->getSucc(true), newBBPtr);
  EXPECT_EQ(bb1Ptr->getSucc(false), nullptr);

  EXPECT_EQ(newBBPtr->getPreds().size(), 1);
  EXPECT_EQ(newBBPtr->getPreds().front(), bb1Ptr);

  EXPECT_EQ(newBBPtr->getSucc(true), bb3Ptr);
  EXPECT_EQ(newBBPtr->getSucc(false), bb4Ptr);

  EXPECT_EQ(bb3Ptr->getPreds().front(), newBBPtr);
  EXPECT_EQ(bb4Ptr->getPreds().front(), newBBPtr);
}

TEST(REGION, EXFAIL_REPLACE_BB_NOT_PRESENT) {
  Region region("foo");

  BasicBlock bb;
  auto res = region.replaceBasicBlockWith(&bb, std::make_unique<BasicBlock>(0));

  EXPECT_FALSE(res);
}

TEST(REGION, REPLACE_START_BB) {
  Region region("foo");

  auto bb = std::make_unique<BasicBlock>(0);
  auto bbPtr = bb.get();

  region.addStartBasicBlock(std::move(bb));

  auto newBB = std::make_unique<BasicBlock>(1);
  auto newBBPtr = newBB.get();

  auto res = region.replaceBasicBlockWith(bbPtr, std::move(newBB));
  EXPECT_TRUE(res);

  EXPECT_TRUE(region.hasStartBasicBlock());
  EXPECT_EQ(region.getStartBasicBlock(), newBBPtr);
}

TEST(REGION, REPLACE_FINAL_BB) {
  Region region("foo");

  auto bb = std::make_unique<BasicBlock>(0);
  auto bbPtr = bb.get();

  region.addFinalBasicBlock(std::move(bb));

  auto newBB = std::make_unique<BasicBlock>(1);
  auto newBBPtr = newBB.get();

  auto res = region.replaceBasicBlockWith(bbPtr, std::move(newBB));
  EXPECT_TRUE(res);

  EXPECT_TRUE(region.hasFinalBasicBlock());
  EXPECT_EQ(region.getFinalBasicBlock(), newBBPtr);
}

TEST(REGION, REPLACE_BB_BY_ID) {
  Region region("foo");

  auto bb1 = std::make_unique<BasicBlock>(0);
  auto bb2 = std::make_unique<BasicBlock>(1);
  auto bb3 = std::make_unique<BasicBlock>(2);
  auto bb4 = std::make_unique<BasicBlock>(3);

  // bb1 -> bb2
  bb1->linkSucc(bb2.get());
  // bb2 -> bb3, bb4
  bb2->linkSucc(bb3.get(), true);
  bb2->linkSucc(bb4.get(), false);

  auto bb1Ptr = bb1.get();
  auto bb2Ptr = bb2.get();
  auto bb3Ptr = bb3.get();
  auto bb4Ptr = bb4.get();

  region.addBasicBlock(std::move(bb1));
  region.addBasicBlock(std::move(bb2));
  region.addBasicBlock(std::move(bb3));
  region.addBasicBlock(std::move(bb4));

  auto newBB = std::make_unique<BasicBlock>(4);
  auto newBBPtr = newBB.get();

  auto res = region.replaceBasicBlockWithByID(1, std::move(newBB));

  EXPECT_TRUE(res);

  EXPECT_TRUE(region.isBasicBlockPresent(bb1Ptr));
  EXPECT_TRUE(region.isBasicBlockPresent(bb3Ptr));
  EXPECT_TRUE(region.isBasicBlockPresent(bb4Ptr));

  EXPECT_FALSE(region.isBasicBlockPresent(bb2Ptr));
  EXPECT_TRUE(region.isBasicBlockPresent(newBBPtr));

  EXPECT_EQ(bb1Ptr->getSucc(true), newBBPtr);
  EXPECT_EQ(bb1Ptr->getSucc(false), nullptr);

  EXPECT_EQ(newBBPtr->getPreds().size(), 1);
  EXPECT_EQ(newBBPtr->getPreds().front(), bb1Ptr);

  EXPECT_EQ(newBBPtr->getSucc(true), bb3Ptr);
  EXPECT_EQ(newBBPtr->getSucc(false), bb4Ptr);

  EXPECT_EQ(bb3Ptr->getPreds().front(), newBBPtr);
  EXPECT_EQ(bb4Ptr->getPreds().front(), newBBPtr);
}

TEST(REGION, EXFAIL_REPLACE_BB_BY_ID_NOT_PRESENT) {
  Region region("foo");

  auto res =
    region.replaceBasicBlockWithByID(0, std::make_unique<BasicBlock>(0));
  EXPECT_FALSE(res);
}

TEST(REGION, REPLACE_START_BB_BY_ID) {
  Region region("foo");

  auto bb = std::make_unique<BasicBlock>(0);
  region.addStartBasicBlock(std::move(bb));

  auto newBB = std::make_unique<BasicBlock>(1);
  auto newBBPtr = newBB.get();

  auto res = region.replaceBasicBlockWithByID(0, std::move(newBB));
  EXPECT_TRUE(res);

  EXPECT_TRUE(region.hasStartBasicBlock());
  EXPECT_EQ(region.getStartBasicBlock(), newBBPtr);
}

TEST(REGION, REPLACE_FINAL_BB_BY_ID) {
  Region region("foo");

  auto bb = std::make_unique<BasicBlock>(0);
  region.addFinalBasicBlock(std::move(bb));

  auto newBB = std::make_unique<BasicBlock>(1);
  auto newBBPtr = newBB.get();

  auto res = region.replaceBasicBlockWithByID(0, std::move(newBB));
  EXPECT_TRUE(res);

  EXPECT_TRUE(region.hasFinalBasicBlock());
  EXPECT_EQ(region.getFinalBasicBlock(), newBBPtr);
}

TEST(REGION, DOM_INFO_BASIC) {
  Region region("foo");
  EXPECT_TRUE(region.isDomInfoExpired());

  region.addStartBasicBlock(std::make_unique<BasicBlock>(0));

  region.collectDomInfo();
  EXPECT_FALSE(region.isDomInfoExpired());

  region.addBasicBlock(std::make_unique<BasicBlock>(1));
  EXPECT_TRUE(region.isDomInfoExpired());
}

TEST(REGION, EXFAIL_COLLECT_DOM_INFO_NO_START_BB) {
  Region region("foo");

  try {
    region.collectDomInfo();
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(!str.compare(
      "Cannot collect dom info with no start basic block specified!"));
    return;
  }
  FAIL();
}

TEST(REGION, EXFAIL_GET_DFS_NO_START_BB) {
  Region region("foo");

  try {
    [[maybe_unused]] auto dfs = region.getDFS();
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(
      !str.compare("Cannot run DFS with no start basic block specified!"));
    return;
  }
  FAIL();
}

TEST(REGION, EXFAIL_GET_RPO_NO_START_BB) {
  Region region("foo");

  try {
    [[maybe_unused]] auto rpo = region.getRPO();
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(
      !str.compare("Cannot run RPO with no start basic block specified!"));
    return;
  }
  FAIL();
}

TEST(REGION, EXFAIL_GET_IDOM_EXPIRED_DOM_INFO) {
  Region region("foo");

  auto bb0 = std::make_unique<BasicBlock>(0);
  auto bb0Ptr = bb0.get();
  region.addStartBasicBlock(std::move(bb0));

  region.collectDomInfo();
  region.addBasicBlock(std::make_unique<BasicBlock>(1));

  try {
    [[maybe_unused]] auto idom = region.getIDom(bb0Ptr);
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(!str.compare("Dom info is expired!"));
    return;
  }
  FAIL();
}

TEST(REGION, EXFAIL_GET_IDOM_BY_ID_EXPIRED_DOM_INFO) {
  Region region("foo");

  auto bb0 = std::make_unique<BasicBlock>(0);
  region.addStartBasicBlock(std::move(bb0));

  region.collectDomInfo();
  region.addBasicBlock(std::make_unique<BasicBlock>(1));

  try {
    [[maybe_unused]] auto idom = region.getIDomByID(0);
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(!str.compare("Dom info is expired!"));
    return;
  }
  FAIL();
}

TEST(REGION, EXFAIL_GET_DOM_BLOCKS_EXPIRED_DOM_INFO) {
  Region region("foo");

  auto bb0 = std::make_unique<BasicBlock>(0);
  auto bb0Ptr = bb0.get();
  region.addStartBasicBlock(std::move(bb0));

  region.collectDomInfo();
  region.addBasicBlock(std::make_unique<BasicBlock>(1));

  try {
    [[maybe_unused]] auto bbs = region.getDominatedBlocks(bb0Ptr);
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(!str.compare("Dom info is expired!"));
    return;
  }
  FAIL();
}

TEST(REGION, EXFAIL_GET_DOM_BLOCKS_BY_IDEXPIRED_DOM_INFO) {
  Region region("foo");

  auto bb0 = std::make_unique<BasicBlock>(0);
  region.addStartBasicBlock(std::move(bb0));

  region.collectDomInfo();
  region.addBasicBlock(std::make_unique<BasicBlock>(1));

  try {
    [[maybe_unused]] auto bbs = region.getDominatedBlocksByID(0);
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(!str.compare("Dom info is expired!"));
    return;
  }
  FAIL();
}

TEST(REGION, EXFAIL_GET_DOM_CHAIN_EXPIRED_DOM_INFO) {
  Region region("foo");

  auto bb0 = std::make_unique<BasicBlock>(0);
  auto bb0Ptr = bb0.get();
  region.addStartBasicBlock(std::move(bb0));

  region.collectDomInfo();
  region.addBasicBlock(std::make_unique<BasicBlock>(1));

  try {
    [[maybe_unused]] auto chain = region.getDominatorsChain(bb0Ptr);
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(!str.compare("Dom info is expired!"));
    return;
  }
  FAIL();
}

TEST(REGION, EXFAIL_GET_DOM_CHAIN_BY_ID_EXPIRED_DOM_INFO) {
  Region region("foo");

  auto bb0 = std::make_unique<BasicBlock>(0);
  region.addStartBasicBlock(std::move(bb0));

  region.collectDomInfo();
  region.addBasicBlock(std::make_unique<BasicBlock>(1));

  try {
    [[maybe_unused]] auto chain = region.getDominatorsChainByID(0);
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_TRUE(!str.compare("Dom info is expired!"));
    return;
  }
  FAIL();
}
