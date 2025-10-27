#include <iris.hpp>

int main() {
  using namespace iris;

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

  loopInfo.dump(std::cout);
  return 0;
}