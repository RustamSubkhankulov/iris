#include <iris.hpp>

int main() {
  using namespace iris;

  IRBuilder builder;
  builder.startNewRegion("foo");

  builder.startNewBasicBlock();

  auto a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::UI32);
  auto a1 = builder.createAndAddOp<builtin::ParamOp>(DataType::UI32);

  auto a2 = builder.createAndAddOp<builtin::ParamOp>(DataType::UI32);
  auto a3 = builder.createAndAddOp<builtin::ParamOp>(DataType::UI32);

  auto v4 = builder.createAndAddOp<arith::MulOp>(a0, a1);
  /* v5 */ builder.createAndAddOp<arith::AddOp>(a2, a3);

  /* n6 */ builder.createAndAddOp<ctrlflow::ReturnOp>(v4);
  auto& bb0 = builder.finalizeCurBasicBlock();

  auto regionPtr = builder.obtainRegion();
  regionPtr->setStartBasicBlock(&bb0);
  regionPtr->setFinalBasicBlock(&bb0);

  std::string msg;
  if (!regionPtr->verify(msg)) {
    std::cerr << "Verification failed: " << msg << std::endl;
  }

  std::cout << "==================" << std::endl;
  std::cout << "Region before DCE:" << std::endl;
  regionPtr->dump(std::cout);

  opt::PassManager pm;
  pm.addPass(std::make_unique<opt::common::DCEPass>());

  pm.run(*regionPtr);

  if (!regionPtr->verify(msg)) {
    std::cerr << "Verification failed: " << msg << std::endl;
  }

  std::cout << "=================" << std::endl;
  std::cout << "Region after DCE:" << std::endl;
  regionPtr->dump(std::cout);
}