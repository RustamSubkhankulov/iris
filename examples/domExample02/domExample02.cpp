#include <iris.hpp>

int main() {
  using namespace iris;

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

  domInfo.dump(std::cout);
  return 0;
}