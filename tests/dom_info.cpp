#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(DOM_INFO, DEFAULT) {
  doms::DomInfo domInfo;

  ASSERT_TRUE(domInfo.isExpired());
}

TEST(DOM_INFO, BASIC) {
  doms::DomInfo domInfo;

  Region region("foo");
  region.addStartBasicBlock(std::make_unique<BasicBlock>(0));

  domInfo.analyze(region);
  ASSERT_FALSE(domInfo.isExpired());
}

TEST(DOM_INFO, EXFAIL_GET_DFS_NO_START_BB) {
  Region region("foo");

  try {
    [[maybe_unused]] auto dfs = doms::DomInfo::getDFS(region);
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_EQ(
      std::string("Cannot run DFS with no start basic block specified!"), str);
    return;
  }
  FAIL();
}

TEST(DOM_INFO, EXFAIL_GET_RPO_NO_START_BB) {
  Region region("foo");

  try {
    [[maybe_unused]] auto rpo = doms::DomInfo::getRPO(region);
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_EQ(
      std::string("Cannot run RPO with no start basic block specified!"), str);
    return;
  }
  FAIL();
}

TEST(DOM_INFO, EXFAIL_GET_IDOM_EXPIRED_DOM_INFO) {
  Region region("foo");

  auto bb0 = std::make_unique<BasicBlock>(0);
  auto bb0Ptr = bb0.get();
  region.addStartBasicBlock(std::move(bb0));

  region.collectDomInfo();
  region.addBasicBlock(std::make_unique<BasicBlock>(1));

  try {
    auto& domInfo = region.getDomInfo();
    [[maybe_unused]] auto idom = domInfo.getIDom(bb0Ptr);
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_EQ(std::string("Dom info is expired!"), str);
    return;
  }
  FAIL();
}

TEST(DOM_INFO, EXFAIL_GET_IDOM_BY_ID_EXPIRED_DOM_INFO) {
  Region region("foo");

  auto bb0 = std::make_unique<BasicBlock>(0);
  region.addStartBasicBlock(std::move(bb0));

  region.collectDomInfo();
  region.addBasicBlock(std::make_unique<BasicBlock>(1));

  try {
    auto& domInfo = region.getDomInfo();
    [[maybe_unused]] auto idom = domInfo.getIDomByID(0, region);
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_EQ(std::string("Dom info is expired!"), str);
    return;
  }
  FAIL();
}

TEST(DOM_INFO, EXFAIL_GET_DOM_BLOCKS_EXPIRED_DOM_INFO) {
  Region region("foo");

  auto bb0 = std::make_unique<BasicBlock>(0);
  auto bb0Ptr = bb0.get();
  region.addStartBasicBlock(std::move(bb0));

  region.collectDomInfo();
  region.addBasicBlock(std::make_unique<BasicBlock>(1));

  try {
    auto& domInfo = region.getDomInfo();
    [[maybe_unused]] auto bbs = domInfo.getDominatedBlocks(bb0Ptr);
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_EQ(std::string("Dom info is expired!"), str);
    return;
  }
  FAIL();
}

TEST(DOM_INFO, EXFAIL_GET_DOM_BLOCKS_BY_IDEXPIRED_DOM_INFO) {
  Region region("foo");

  auto bb0 = std::make_unique<BasicBlock>(0);
  region.addStartBasicBlock(std::move(bb0));

  region.collectDomInfo();
  region.addBasicBlock(std::make_unique<BasicBlock>(1));

  try {
    auto& domInfo = region.getDomInfo();
    [[maybe_unused]] auto bbs = domInfo.getDominatedBlocksByID(0, region);
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_EQ(std::string("Dom info is expired!"), str);
    return;
  }
  FAIL();
}

TEST(DOM_INFO, EXFAIL_GET_DOM_CHAIN_EXPIRED_DOM_INFO) {
  Region region("foo");

  auto bb0 = std::make_unique<BasicBlock>(0);
  auto bb0Ptr = bb0.get();
  region.addStartBasicBlock(std::move(bb0));

  region.collectDomInfo();
  region.addBasicBlock(std::make_unique<BasicBlock>(1));

  try {
    auto& domInfo = region.getDomInfo();
    [[maybe_unused]] auto chain = domInfo.getDominatorsChain(bb0Ptr);
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_EQ(std::string("Dom info is expired!"), str);
    return;
  }
  FAIL();
}

TEST(DOM_INFO, EXFAIL_GET_DOM_CHAIN_BY_ID_EXPIRED_DOM_INFO) {
  Region region("foo");

  auto bb0 = std::make_unique<BasicBlock>(0);
  region.addStartBasicBlock(std::move(bb0));

  region.collectDomInfo();
  region.addBasicBlock(std::make_unique<BasicBlock>(1));

  try {
    auto& domInfo = region.getDomInfo();
    [[maybe_unused]] auto chain = domInfo.getDominatorsChainByID(0, region);
  } catch (const IrisException& exc) {
    std::string str = exc.what();
    EXPECT_EQ(std::string("Dom info is expired!"), str);
    return;
  }
  FAIL();
}
