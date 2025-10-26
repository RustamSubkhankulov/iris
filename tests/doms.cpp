#include <gtest/gtest.h>

#include <iris.hpp>
using namespace iris;

TEST(DOMS, EXAMPLE_1) {
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

  region.collectDomInfo();
  auto& domInfo = region.getDomInfo();

  EXPECT_EQ(domInfo.getIDomByID(0, region)->getID(), 0); // idom(A) = A
  EXPECT_EQ(domInfo.getIDomByID(1, region)->getID(), 0); // idom(B) = A
  EXPECT_EQ(domInfo.getIDomByID(2, region)->getID(), 1); // idom(C) = B
  EXPECT_EQ(domInfo.getIDomByID(3, region)->getID(), 1); // idom(D) = B
  EXPECT_EQ(domInfo.getIDomByID(4, region)->getID(), 5); // idom(E) = F
  EXPECT_EQ(domInfo.getIDomByID(5, region)->getID(), 1); // idom(F) = B
  EXPECT_EQ(domInfo.getIDomByID(6, region)->getID(), 5); // idom(G) = F
}

TEST(DOMS, EXAMPLE_2) {
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

  region.collectDomInfo();
  auto& domInfo = region.getDomInfo();

  EXPECT_EQ(domInfo.getIDomByID(0, region)->getID(), 0);  // idom(A) = A
  EXPECT_EQ(domInfo.getIDomByID(1, region)->getID(), 0);  // idom(B) = A
  EXPECT_EQ(domInfo.getIDomByID(2, region)->getID(), 1);  // idom(C) = B
  EXPECT_EQ(domInfo.getIDomByID(3, region)->getID(), 2);  // idom(D) = C
  EXPECT_EQ(domInfo.getIDomByID(4, region)->getID(), 3);  // idom(E) = D
  EXPECT_EQ(domInfo.getIDomByID(5, region)->getID(), 4);  // idom(F) = E
  EXPECT_EQ(domInfo.getIDomByID(6, region)->getID(), 5);  // idom(G) = F
  EXPECT_EQ(domInfo.getIDomByID(7, region)->getID(), 6);  // idom(H) = G
  EXPECT_EQ(domInfo.getIDomByID(8, region)->getID(), 6);  // idom(I) = G
  EXPECT_EQ(domInfo.getIDomByID(9, region)->getID(), 1);  // idom(J) = B
  EXPECT_EQ(domInfo.getIDomByID(10, region)->getID(), 8); // idom(K) = I
}

TEST(DOMS, EXAMPLE_3) {
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

  region.collectDomInfo();
  auto& domInfo = region.getDomInfo();

  EXPECT_EQ(domInfo.getIDomByID(0, region)->getID(), 0); // idom(A) = A
  EXPECT_EQ(domInfo.getIDomByID(1, region)->getID(), 0); // idom(B) = A
  EXPECT_EQ(domInfo.getIDomByID(2, region)->getID(), 1); // idom(C) = B
  EXPECT_EQ(domInfo.getIDomByID(3, region)->getID(), 1); // idom(D) = B
  EXPECT_EQ(domInfo.getIDomByID(4, region)->getID(), 1); // idom(E) = B
  EXPECT_EQ(domInfo.getIDomByID(5, region)->getID(), 4); // idom(F) = E
  EXPECT_EQ(domInfo.getIDomByID(6, region)->getID(), 1); // idom(G) = B
  EXPECT_EQ(domInfo.getIDomByID(7, region)->getID(), 5); // idom(H) = F
  EXPECT_EQ(domInfo.getIDomByID(8, region)->getID(), 1); // idom(I) = B
}