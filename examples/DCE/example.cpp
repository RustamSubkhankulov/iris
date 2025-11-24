#include <iris.hpp>

int main() {
  using namespace iris;

  IRBuilder builder;
  builder.startNewRegion("dce");

  // Single basic block with a lot of dead computations.
  builder.startNewBasicBlock();

  // Live parameters.
  auto a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto a1 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto a2 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto a3 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);

  // Some constants.
  auto c4 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(10u));
  auto c5 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(20u));

  // Live computation chain: will be preserved by DCE.
  auto v6 = builder.createAndAddOp<arith::AddOp>(a0, a1);
  auto v7 = builder.createAndAddOp<arith::MulOp>(v6, c4);
  auto v8 = builder.createAndAddOp<arith::SubOp>(v7, a2);
  auto v9 = builder.createAndAddOp<builtin::CopyOp>(v8);

  // Dead chain #1: all results unused.
  auto v10 = builder.createAndAddOp<arith::MulOp>(a2, a3);
  auto v11 = builder.createAndAddOp<arith::AddOp>(v10, c5);
  /* v12 = */ builder.createAndAddOp<arith::MulOp>(v11, v11);

  // Dead chain #2: constants feeding into each other only.
  auto c13 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(5u));
  auto с14 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(7u));
  auto v15 = builder.createAndAddOp<arith::AddOp>(c13, с14);
  auto v16 = builder.createAndAddOp<builtin::CopyOp>(v15);
  /* auto v17 = */ builder.createAndAddOp<arith::SubOp>(v16, с14);

  // Use only v9 in the return, everything else that is not on this path
  // should be eliminated by DCE.
  builder.createAndAddOp<ctrlflow::ReturnOp>(v9);

  auto& bb0 = builder.finalizeCurBasicBlock();

  auto regionPtr = builder.obtainRegion();
  regionPtr->setStartBasicBlock(&bb0);
  regionPtr->setFinalBasicBlock(&bb0);

  std::string msg;
  if (!regionPtr->verify(msg)) {
    std::cerr << "Verification failed (before DCE): " << msg << std::endl;
  }

  std::cout << "==========================" << std::endl;
  std::cout << "Region before DCE (complex):" << std::endl;
  regionPtr->dump(std::cout);

  opt::PassManager pm;
  pm.addPass(std::make_unique<opt::common::DCEPass>());
  pm.run(*regionPtr);

  if (!regionPtr->verify(msg)) {
    std::cerr << "Verification failed (after DCE): " << msg << std::endl;
  }

  std::cout << "==========================" << std::endl;
  std::cout << "Region after DCE (complex):" << std::endl;
  regionPtr->dump(std::cout);

  return 0;
}
