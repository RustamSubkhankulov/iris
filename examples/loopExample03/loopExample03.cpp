#include <iris.hpp>

int main() {
  using namespace iris;

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

  loopInfo.dump(std::cout);
  return 0;
}