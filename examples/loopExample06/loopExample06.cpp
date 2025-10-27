#include <iris.hpp>

int main() {
  using namespace iris;

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

  region.collectLoopInfo();
  auto& loopInfo = region.getLoopInfo();

  loopInfo.dump(std::cout);
  return 0;
}