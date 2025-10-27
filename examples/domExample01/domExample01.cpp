#include <iris.hpp>

int main() {
  using namespace iris;

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

  domInfo.dump(std::cout);
  return 0;
}