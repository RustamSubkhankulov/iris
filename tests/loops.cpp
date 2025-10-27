#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(LOOPS, EXAMPLE_01) {
  /*
  0 → 1 → 2 → 3 → 1
      ↓
      4
  */
  Region region("foo");

  auto blockA = std::make_unique<BasicBlock>(0);
  auto blockB = std::make_unique<BasicBlock>(1);
  auto blockC = std::make_unique<BasicBlock>(2);
  auto blockD = std::make_unique<BasicBlock>(3);
  auto blockE = std::make_unique<BasicBlock>(4);

  auto blockAPtr = blockA.get();
  auto blockBPtr = blockB.get();
  auto blockCPtr = blockC.get();
  auto blockDPtr = blockD.get();
  auto blockEPtr = blockE.get();

  // A -> B
  blockA->linkSucc(blockB.get());

  // B -> C
  // B -> E
  blockB->linkSucc(blockC.get(), true);
  blockB->linkSucc(blockE.get(), false);

  // C -> D
  blockC->linkSucc(blockD.get());

  // D -> B
  blockD->linkSucc(blockB.get());

  region.addStartBasicBlock(std::move(blockA));
  region.addBasicBlock(std::move(blockB));
  region.addBasicBlock(std::move(blockC));
  region.addBasicBlock(std::move(blockD));
  region.addFinalBasicBlock(std::move(blockE));

  region.collectLoopInfo();
  auto& loopInfo = region.getLoopInfo();

  //---

  const auto& root = loopInfo.getRootLoop();

  EXPECT_EQ(root.getHeader(), nullptr);
  EXPECT_EQ(root.getDepth(), 0);

  EXPECT_TRUE(root.isReducible());
  EXPECT_TRUE(root.isRoot());

  EXPECT_EQ(root.getBlocks().size(), 2);
  EXPECT_TRUE(root.getBlocks().contains(blockAPtr));
  EXPECT_TRUE(root.getBlocks().contains(blockEPtr));

  EXPECT_EQ(root.getNestedLoops().size(), 1);

  const auto& topLevel = loopInfo.getTopLevelLoops();
  EXPECT_EQ(topLevel.size(), 1);

  const auto* loop = topLevel.front();

  //---

  EXPECT_EQ(loop->getHeader(), blockBPtr);
  EXPECT_EQ(loop->getDepth(), 1);

  EXPECT_TRUE(loop->isReducible());
  EXPECT_FALSE(loop->isRoot());

  EXPECT_EQ(loop->getLatches().size(), 1);
  EXPECT_TRUE(loop->getLatches().contains(blockDPtr));

  EXPECT_EQ(loop->getBlocks().size(), 1);
  EXPECT_TRUE(loop->getBlocks().contains(blockCPtr));

  EXPECT_EQ(loop->getExits().size(), 1);
  EXPECT_EQ(loop->getExits().front().src()->getID(), 1);
  EXPECT_EQ(loop->getExits().front().dst()->getID(), 4);

  EXPECT_EQ(loop->getNestedLoops().size(), 0);
}

TEST(LOOPS, EXAMPLE_02) {
  /*
  0 → 1 → 2 → 3 → 4 → 1
          ↓   ↓
          5   5
  */
  Region region("foo");

  auto blockA = std::make_unique<BasicBlock>(0);
  auto blockB = std::make_unique<BasicBlock>(1);
  auto blockC = std::make_unique<BasicBlock>(2);
  auto blockD = std::make_unique<BasicBlock>(3);
  auto blockE = std::make_unique<BasicBlock>(4);
  auto blockF = std::make_unique<BasicBlock>(5);

  auto blockAPtr = blockA.get();
  auto blockBPtr = blockB.get();
  auto blockCPtr = blockC.get();
  auto blockDPtr = blockD.get();
  auto blockEPtr = blockE.get();
  auto blockFPtr = blockF.get();

  // A -> B
  blockA->linkSucc(blockB.get());

  // B -> C
  blockB->linkSucc(blockC.get());

  // C -> D
  // C -> F
  blockC->linkSucc(blockD.get(), true);
  blockC->linkSucc(blockF.get(), false);

  // D -> E
  // D -> F
  blockD->linkSucc(blockE.get(), true);
  blockD->linkSucc(blockF.get(), false);

  // E -> B
  blockE->linkSucc(blockB.get());

  region.addStartBasicBlock(std::move(blockA));
  region.addBasicBlock(std::move(blockB));
  region.addBasicBlock(std::move(blockC));
  region.addBasicBlock(std::move(blockD));
  region.addBasicBlock(std::move(blockE));
  region.addFinalBasicBlock(std::move(blockF));

  region.collectLoopInfo();
  auto& loopInfo = region.getLoopInfo();

  //---

  const auto& root = loopInfo.getRootLoop();

  EXPECT_EQ(root.getHeader(), nullptr);
  EXPECT_EQ(root.getDepth(), 0);

  EXPECT_TRUE(root.isReducible());
  EXPECT_TRUE(root.isRoot());

  EXPECT_EQ(root.getBlocks().size(), 2);
  EXPECT_TRUE(root.getBlocks().contains(blockAPtr));
  EXPECT_TRUE(root.getBlocks().contains(blockFPtr));

  EXPECT_EQ(root.getNestedLoops().size(), 1);

  const auto& topLevel = loopInfo.getTopLevelLoops();
  EXPECT_EQ(topLevel.size(), 1);

  //---

  const auto* loop = topLevel.front();

  EXPECT_EQ(loop->getHeader(), blockBPtr);
  EXPECT_EQ(loop->getDepth(), 1);

  EXPECT_TRUE(loop->isReducible());
  EXPECT_FALSE(loop->isRoot());

  EXPECT_EQ(loop->getLatches().size(), 1);
  EXPECT_TRUE(loop->getLatches().contains(blockEPtr));

  EXPECT_EQ(loop->getBlocks().size(), 2);
  EXPECT_TRUE(loop->getBlocks().contains(blockCPtr));
  EXPECT_TRUE(loop->getBlocks().contains(blockDPtr));

  EXPECT_EQ(loop->getExits().size(), 2);
  EXPECT_EQ(loop->getNestedLoops().size(), 0);
}

TEST(LOOPS, EXAMPLE_03) {
  /*
  0 → 1 → 2 → 4 → 5 → 6
      ↓   ↓       ↓   ↓
      3   7       1   0
      ↓
      4
  */
  Region region("foo");

  auto blockA = std::make_unique<BasicBlock>(0);
  auto blockB = std::make_unique<BasicBlock>(1);
  auto blockC = std::make_unique<BasicBlock>(2);
  auto blockD = std::make_unique<BasicBlock>(3);
  auto blockE = std::make_unique<BasicBlock>(4);
  auto blockF = std::make_unique<BasicBlock>(5);
  auto blockG = std::make_unique<BasicBlock>(6);
  auto blockH = std::make_unique<BasicBlock>(7);

  auto blockAPtr = blockA.get();
  auto blockBPtr = blockB.get();
  auto blockCPtr = blockC.get();
  auto blockDPtr = blockD.get();
  auto blockEPtr = blockE.get();
  auto blockFPtr = blockF.get();
  auto blockGPtr = blockG.get();
  auto blockHPtr = blockH.get();

  // A -> B
  blockA->linkSucc(blockB.get());

  // B -> C
  // B -> D
  blockB->linkSucc(blockC.get(), true);
  blockB->linkSucc(blockD.get(), false);

  // C -> E
  // C -> H
  blockC->linkSucc(blockE.get(), true);
  blockC->linkSucc(blockH.get(), false);

  // D -> E
  blockD->linkSucc(blockE.get());

  // E -> F
  blockE->linkSucc(blockF.get());

  // F -> B
  // F -> G
  blockF->linkSucc(blockB.get(), true);
  blockF->linkSucc(blockG.get(), false);

  // G -> A
  blockG->linkSucc(blockA.get());

  region.addStartBasicBlock(std::move(blockA));
  region.addBasicBlock(std::move(blockB));
  region.addBasicBlock(std::move(blockC));
  region.addBasicBlock(std::move(blockD));
  region.addBasicBlock(std::move(blockE));
  region.addBasicBlock(std::move(blockF));
  region.addBasicBlock(std::move(blockG));
  region.addFinalBasicBlock(std::move(blockH));

  region.collectLoopInfo();
  auto& loopInfo = region.getLoopInfo();

  //---

  const auto& root = loopInfo.getRootLoop();

  EXPECT_EQ(root.getHeader(), nullptr);
  EXPECT_EQ(root.getDepth(), 0);

  EXPECT_TRUE(root.isReducible());
  EXPECT_TRUE(root.isRoot());

  EXPECT_EQ(root.getBlocks().size(), 1);
  EXPECT_TRUE(root.getBlocks().contains(blockHPtr));

  EXPECT_EQ(root.getNestedLoops().size(), 1);

  const auto& topLevel = loopInfo.getTopLevelLoops();
  EXPECT_EQ(topLevel.size(), 1);

  //---

  const auto* loop = topLevel.front();

  EXPECT_EQ(loop->getHeader(), blockAPtr);
  EXPECT_EQ(loop->getDepth(), 1);

  EXPECT_TRUE(loop->isReducible());
  EXPECT_FALSE(loop->isRoot());

  EXPECT_EQ(loop->getLatches().size(), 1);
  EXPECT_TRUE(loop->getLatches().contains(blockGPtr));

  EXPECT_EQ(loop->getBlocks().size(), 0);

  EXPECT_EQ(loop->getExits().size(), 1);
  EXPECT_EQ(loop->getExits().front().src()->getID(), 2);
  EXPECT_EQ(loop->getExits().front().dst()->getID(), 7);

  EXPECT_EQ(loop->getNestedLoops().size(), 1);

  //---

  loop = loop->getNestedLoops().front();

  EXPECT_EQ(loop->getHeader(), blockBPtr);
  EXPECT_EQ(loop->getDepth(), 2);

  EXPECT_TRUE(loop->isReducible());
  EXPECT_FALSE(loop->isRoot());

  EXPECT_EQ(loop->getLatches().size(), 1);
  EXPECT_TRUE(loop->getLatches().contains(blockFPtr));

  EXPECT_EQ(loop->getBlocks().size(), 3);
  EXPECT_TRUE(loop->getBlocks().contains(blockCPtr));
  EXPECT_TRUE(loop->getBlocks().contains(blockDPtr));
  EXPECT_TRUE(loop->getBlocks().contains(blockEPtr));

  EXPECT_EQ(loop->getExits().size(), 2);

  EXPECT_EQ(loop->getNestedLoops().size(), 0);
}

TEST(LOOPS, EXAMPLE_04) {
  /*
  A → B → C → D
      ↓
      F → E → D
      ↓
      G → D
  */
  Region region("foo");

  auto blockA = std::make_unique<BasicBlock>(0);
  auto blockB = std::make_unique<BasicBlock>(1);
  auto blockC = std::make_unique<BasicBlock>(2);
  auto blockD = std::make_unique<BasicBlock>(3);
  auto blockE = std::make_unique<BasicBlock>(4);
  auto blockF = std::make_unique<BasicBlock>(5);
  auto blockG = std::make_unique<BasicBlock>(6);

  auto blockAPtr = blockA.get();
  auto blockBPtr = blockB.get();
  auto blockCPtr = blockC.get();
  auto blockDPtr = blockD.get();
  auto blockEPtr = blockE.get();
  auto blockFPtr = blockF.get();
  auto blockGPtr = blockG.get();

  // A -> B
  blockA->linkSucc(blockB.get());

  // B -> C, F
  blockB->linkSucc(blockC.get(), true);
  blockB->linkSucc(blockF.get(), false);

  // C -> D
  blockC->linkSucc(blockD.get());

  // E -> D
  blockE->linkSucc(blockD.get());

  // F -> E, G
  blockF->linkSucc(blockE.get(), true);
  blockF->linkSucc(blockG.get(), false);

  // G -> D
  blockG->linkSucc(blockD.get());

  region.addStartBasicBlock(std::move(blockA));
  region.addBasicBlock(std::move(blockB));
  region.addBasicBlock(std::move(blockC));
  region.addBasicBlock(std::move(blockE));
  region.addBasicBlock(std::move(blockF));
  region.addBasicBlock(std::move(blockG));
  region.addFinalBasicBlock(std::move(blockD));

  region.collectLoopInfo();
  auto& loopInfo = region.getLoopInfo();

  const auto& root = loopInfo.getRootLoop();

  EXPECT_EQ(root.getHeader(), nullptr);
  EXPECT_EQ(root.getDepth(), 0);

  EXPECT_TRUE(root.isReducible());
  EXPECT_TRUE(root.isRoot());

  EXPECT_EQ(root.getBlocks().size(), 7);
  EXPECT_TRUE(root.getBlocks().contains(blockAPtr));
  EXPECT_TRUE(root.getBlocks().contains(blockBPtr));
  EXPECT_TRUE(root.getBlocks().contains(blockCPtr));
  EXPECT_TRUE(root.getBlocks().contains(blockDPtr));
  EXPECT_TRUE(root.getBlocks().contains(blockEPtr));
  EXPECT_TRUE(root.getBlocks().contains(blockFPtr));
  EXPECT_TRUE(root.getBlocks().contains(blockGPtr));

  EXPECT_EQ(root.getNestedLoops().size(), 0);

  const auto& topLevel = loopInfo.getTopLevelLoops();
  EXPECT_EQ(topLevel.size(), 0);
}

TEST(LOOPS, EXAMPLE_05) {
  /*
  A → B → C → D → E → F → G → I → K
      ↓       ↓       ↓   ↓
      J → C   C       E   H → B
  */
  Region region("foo");

  auto blockA = std::make_unique<BasicBlock>(0);
  auto blockB = std::make_unique<BasicBlock>(1);
  auto blockC = std::make_unique<BasicBlock>(2);
  auto blockD = std::make_unique<BasicBlock>(3);
  auto blockE = std::make_unique<BasicBlock>(4);
  auto blockF = std::make_unique<BasicBlock>(5);
  auto blockG = std::make_unique<BasicBlock>(6);
  auto blockH = std::make_unique<BasicBlock>(7);
  auto blockI = std::make_unique<BasicBlock>(8);
  auto blockJ = std::make_unique<BasicBlock>(9);
  auto blockK = std::make_unique<BasicBlock>(10);

  auto blockAPtr = blockA.get();
  auto blockBPtr = blockB.get();
  auto blockCPtr = blockC.get();
  auto blockDPtr = blockD.get();
  auto blockEPtr = blockE.get();
  auto blockFPtr = blockF.get();
  auto blockGPtr = blockG.get();
  auto blockHPtr = blockH.get();
  auto blockIPtr = blockI.get();
  auto blockJPtr = blockJ.get();
  auto blockKPtr = blockK.get();

  // A -> B
  blockA->linkSucc(blockB.get());

  // B -> C, J
  blockB->linkSucc(blockC.get(), true);
  blockB->linkSucc(blockJ.get(), false);

  // C -> D
  blockC->linkSucc(blockD.get());

  // D -> C, E
  blockD->linkSucc(blockC.get(), true);
  blockD->linkSucc(blockE.get(), false);

  // E -> F
  blockE->linkSucc(blockF.get());

  // F -> E, G
  blockF->linkSucc(blockE.get(), true);
  blockF->linkSucc(blockG.get(), false);

  // G -> H, I
  blockG->linkSucc(blockH.get(), true);
  blockG->linkSucc(blockI.get(), false);

  // H -> B
  blockH->linkSucc(blockB.get());

  // I -> K
  blockI->linkSucc(blockK.get());

  // J -> C
  blockJ->linkSucc(blockC.get());

  region.addStartBasicBlock(std::move(blockA));
  region.addBasicBlock(std::move(blockB));
  region.addBasicBlock(std::move(blockC));
  region.addBasicBlock(std::move(blockD));
  region.addBasicBlock(std::move(blockE));
  region.addBasicBlock(std::move(blockF));
  region.addBasicBlock(std::move(blockG));
  region.addBasicBlock(std::move(blockH));
  region.addBasicBlock(std::move(blockI));
  region.addBasicBlock(std::move(blockJ));
  region.addFinalBasicBlock(std::move(blockK));

  region.collectLoopInfo();
  auto& loopInfo = region.getLoopInfo();

  const auto& root = loopInfo.getRootLoop();

  EXPECT_EQ(root.getHeader(), nullptr);
  EXPECT_EQ(root.getDepth(), 0);

  EXPECT_TRUE(root.isReducible());
  EXPECT_TRUE(root.isRoot());

  EXPECT_EQ(root.getBlocks().size(), 3);
  EXPECT_TRUE(root.getBlocks().contains(blockAPtr));
  EXPECT_TRUE(root.getBlocks().contains(blockIPtr));
  EXPECT_TRUE(root.getBlocks().contains(blockKPtr));

  EXPECT_EQ(root.getNestedLoops().size(), 1);

  const auto& topLevel = loopInfo.getTopLevelLoops();
  EXPECT_EQ(topLevel.size(), 1);

  //---

  const auto* loop = topLevel.front();

  EXPECT_EQ(loop->getHeader(), blockBPtr);
  EXPECT_EQ(loop->getDepth(), 1);

  EXPECT_TRUE(loop->isReducible());
  EXPECT_FALSE(loop->isRoot());

  EXPECT_EQ(loop->getLatches().size(), 1);
  EXPECT_TRUE(loop->getLatches().contains(blockHPtr));

  EXPECT_EQ(loop->getBlocks().size(), 2);
  EXPECT_TRUE(loop->getBlocks().contains(blockGPtr));
  EXPECT_TRUE(loop->getBlocks().contains(blockJPtr));

  EXPECT_EQ(loop->getExits().size(), 1);
  EXPECT_EQ(loop->getExits().front().src()->getID(), 6);
  EXPECT_EQ(loop->getExits().front().dst()->getID(), 8);

  EXPECT_EQ(loop->getNestedLoops().size(), 2);

  //---

  const auto& nested = loop->getNestedLoops();
  auto res = std::find_if(nested.begin(), nested.end(), [=](auto* loop) {
    return (loop->getHeader() == blockCPtr);
  });

  EXPECT_TRUE(res != nested.end());

  loop = *res;

  EXPECT_EQ(loop->getDepth(), 2);

  EXPECT_TRUE(loop->isReducible());
  EXPECT_FALSE(loop->isRoot());

  EXPECT_EQ(loop->getLatches().size(), 1);
  EXPECT_TRUE(loop->getLatches().contains(blockDPtr));

  EXPECT_EQ(loop->getBlocks().size(), 0);

  EXPECT_EQ(loop->getExits().size(), 1);
  EXPECT_EQ(loop->getExits().front().src()->getID(), 3);
  EXPECT_EQ(loop->getExits().front().dst()->getID(), 4);

  EXPECT_EQ(loop->getNestedLoops().size(), 0);

  //---

  res = std::find_if(nested.begin(), nested.end(), [=](auto* loop) {
    return (loop->getHeader() == blockEPtr);
  });

  EXPECT_TRUE(res != nested.end());

  loop = *res;

  EXPECT_EQ(loop->getDepth(), 2);

  EXPECT_TRUE(loop->isReducible());
  EXPECT_FALSE(loop->isRoot());

  EXPECT_EQ(loop->getLatches().size(), 1);
  EXPECT_TRUE(loop->getLatches().contains(blockFPtr));

  EXPECT_EQ(loop->getBlocks().size(), 0);

  EXPECT_EQ(loop->getExits().size(), 1);
  EXPECT_EQ(loop->getExits().front().src()->getID(), 5);
  EXPECT_EQ(loop->getExits().front().dst()->getID(), 6);

  EXPECT_EQ(loop->getNestedLoops().size(), 0);
}

TEST(LOOPS, EXAMPLE_06) {
  /*
  A → B → C → D → G → I
      ↓           ↓
      E → D       C
      ↓
      F → B
      ↓
      H → G
      ↓
      I
  */
  Region region("foo");

  auto blockA = std::make_unique<BasicBlock>(0);
  auto blockB = std::make_unique<BasicBlock>(1);
  auto blockC = std::make_unique<BasicBlock>(2);
  auto blockD = std::make_unique<BasicBlock>(3);
  auto blockE = std::make_unique<BasicBlock>(4);
  auto blockF = std::make_unique<BasicBlock>(5);
  auto blockG = std::make_unique<BasicBlock>(6);
  auto blockH = std::make_unique<BasicBlock>(7);
  auto blockI = std::make_unique<BasicBlock>(8);

  auto blockAPtr = blockA.get();
  auto blockBPtr = blockB.get();
  auto blockCPtr = blockC.get();
  auto blockDPtr = blockD.get();
  auto blockEPtr = blockE.get();
  auto blockFPtr = blockF.get();
  auto blockGPtr = blockG.get();
  auto blockHPtr = blockH.get();
  auto blockIPtr = blockI.get();

  // A -> B
  blockA->linkSucc(blockB.get());

  // B -> C, E
  blockB->linkSucc(blockC.get(), true);
  blockB->linkSucc(blockE.get(), false);

  // C -> D
  blockC->linkSucc(blockD.get());

  // D -> G
  blockD->linkSucc(blockG.get());

  // E -> D, F
  blockE->linkSucc(blockD.get(), true);
  blockE->linkSucc(blockF.get(), false);

  // F -> B, H
  blockF->linkSucc(blockB.get(), true);
  blockF->linkSucc(blockH.get(), false);

  // G -> C, I
  blockG->linkSucc(blockC.get(), true);
  blockG->linkSucc(blockI.get(), false);

  // H -> G, I
  blockH->linkSucc(blockG.get(), true);
  blockH->linkSucc(blockI.get(), false);

  region.addStartBasicBlock(std::move(blockA));
  region.addBasicBlock(std::move(blockB));
  region.addBasicBlock(std::move(blockC));
  region.addBasicBlock(std::move(blockD));
  region.addBasicBlock(std::move(blockE));
  region.addBasicBlock(std::move(blockF));
  region.addBasicBlock(std::move(blockG));
  region.addBasicBlock(std::move(blockH));
  region.addFinalBasicBlock(std::move(blockI));

  region.collectLoopInfo();
  auto& loopInfo = region.getLoopInfo();

  const auto& root = loopInfo.getRootLoop();

  EXPECT_EQ(root.getHeader(), nullptr);
  EXPECT_EQ(root.getDepth(), 0);

  EXPECT_TRUE(root.isReducible());
  EXPECT_TRUE(root.isRoot());

  EXPECT_EQ(root.getBlocks().size(), 4);
  EXPECT_TRUE(root.getBlocks().contains(blockAPtr));
  EXPECT_TRUE(root.getBlocks().contains(blockDPtr));
  EXPECT_TRUE(root.getBlocks().contains(blockHPtr));
  EXPECT_TRUE(root.getBlocks().contains(blockIPtr));

  EXPECT_EQ(root.getNestedLoops().size(), 2);

  const auto& topLevel = loopInfo.getTopLevelLoops();
  EXPECT_EQ(topLevel.size(), 2);

  //---

  const auto& nested = root.getNestedLoops();
  auto res = std::find_if(nested.begin(), nested.end(), [=](auto* loop) {
    return (loop->getHeader() == blockBPtr);
  });

  EXPECT_TRUE(res != nested.end());

  const auto* loop = *res;

  EXPECT_EQ(loop->getDepth(), 1);

  EXPECT_TRUE(loop->isReducible());
  EXPECT_FALSE(loop->isRoot());

  EXPECT_EQ(loop->getLatches().size(), 1);
  EXPECT_TRUE(loop->getLatches().contains(blockFPtr));

  EXPECT_EQ(loop->getBlocks().size(), 1);
  EXPECT_TRUE(loop->getBlocks().contains(blockEPtr));

  EXPECT_EQ(loop->getExits().size(), 3);
  EXPECT_EQ(loop->getNestedLoops().size(), 0);

  //---

  res = std::find_if(nested.begin(), nested.end(), [=](auto* loop) {
    return (loop->getHeader() == blockCPtr);
  });

  EXPECT_TRUE(res != nested.end());

  loop = *res;

  EXPECT_EQ(loop->getDepth(), 1);

  EXPECT_FALSE(loop->isReducible());
  EXPECT_FALSE(loop->isRoot());

  EXPECT_EQ(loop->getLatches().size(), 1);
  EXPECT_TRUE(loop->getLatches().contains(blockGPtr));

  EXPECT_EQ(loop->getNestedLoops().size(), 0);
}
