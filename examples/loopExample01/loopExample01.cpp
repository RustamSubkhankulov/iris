#include <iris.hpp>

int main() {
  using namespace iris;

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

  loopInfo.dump(std::cout);
  return 0;
}