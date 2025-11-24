#include <iris.hpp>

int main() {
  using namespace iris;

  IRBuilder builder;
  builder.startNewRegion("constfold");

  builder.startNewBasicBlock();

  // Some parameters to keep part of the graph non-constant.
  auto a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto a1 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);

  // Integer constants (signed / unsigned).
  auto c2 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::int64_t>(10)));
  auto c3 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::int64_t>(3)));

  auto c4 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::uint64_t>(7)));
  auto c5 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::uint64_t>(2)));

  // Floating-point constants.
  auto c6 = builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(1.5));
  auto c7 = builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(0.5));

  // Boolean constants.
  auto c8 = builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(true));
  auto c9 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(false));

  auto v10 = builder.createAndAddOp<arith::AddOp>(c2, c3);
  auto v11 = builder.createAndAddOp<arith::MulOp>(v10, c3);

  auto v12 = builder.createAndAddOp<arith::AndOp>(c4, c5);
  auto v13 = builder.createAndAddOp<arith::XorOp>(v12, c4);

  auto v14 = builder.createAndAddOp<arith::AddOp>(c6, c7);
  auto v15 = builder.createAndAddOp<arith::DivOp>(v14, c7);

  /* v16 = */ builder.createAndAddOp<arith::ShlOp>(c4, c5);
  /* v17 = */ builder.createAndAddOp<arith::SarOp>(c2, c3);

  auto v18 = builder.createAndAddOp<arith::CompareOp>(
    c8, c9, arith::CompareOp::Pred::EQ);

  auto v19 = builder.createAndAddOp<arith::AddOp>(a0, c3);
  auto v20 = builder.createAndAddOp<arith::AddOp>(v11, v19);
  /* v21 = */ builder.createAndAddOp<arith::AndOp>(v13, a1);

  auto v22 = builder.createAndAddOp<arith::CastOp>(DataType::SINT, v18);
  auto v23 = builder.createAndAddOp<arith::CastOp>(DataType::SINT, v15);

  auto v24 = builder.createAndAddOp<arith::AddOp>(v20, v22);
  auto v25 = builder.createAndAddOp<arith::AddOp>(v24, v23);
  builder.createAndAddOp<ctrlflow::ReturnOp>(v25);

  auto& bb0 = builder.finalizeCurBasicBlock();

  auto regionPtr = builder.obtainRegion();
  regionPtr->setStartBasicBlock(&bb0);
  regionPtr->setFinalBasicBlock(&bb0);

  std::string msg;
  if (!regionPtr->verify(msg)) {
    std::cerr << "Verification failed (before ConstFold): " << msg << std::endl;
  }

  std::cout << "==============================" << std::endl;
  std::cout << "Region before ArithConstFold:" << std::endl;
  regionPtr->dump(std::cout);

  opt::PassManager pmCF;
  pmCF.addPass(std::make_unique<opt::arith::ArithConstFoldPass>());
  pmCF.run(*regionPtr);

  if (!regionPtr->verify(msg)) {
    std::cerr << "Verification failed (after ConstFold): " << msg << std::endl;
  }

  std::cout << "==============================" << std::endl;
  std::cout << "Region after ArithConstFold:" << std::endl;
  regionPtr->dump(std::cout);

  std::cout << "==========================" << std::endl;
  std::cout << "Region before DCE:" << std::endl;
  regionPtr->dump(std::cout);

  opt::PassManager pmDCE;
  pmDCE.addPass(std::make_unique<opt::common::DCEPass>());
  pmDCE.run(*regionPtr);

  if (!regionPtr->verify(msg)) {
    std::cerr << "Verification failed (after DCE): " << msg << std::endl;
  }

  std::cout << "==========================" << std::endl;
  std::cout << "Region after DCE (complex):" << std::endl;
  regionPtr->dump(std::cout);

  return 0;
}
