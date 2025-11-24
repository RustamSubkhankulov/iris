#include <iris.hpp>
#include <limits>

int main() {
  using namespace iris;

  IRBuilder builder;
  builder.startNewRegion("peephole");

  builder.startNewBasicBlock();

  // Parameters: signed and unsigned.
  auto a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto a1 = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto a2 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto a3 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);

  // Constants for various peephole patterns.
  auto c4 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::int64_t>(0)));
  auto c5 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::int64_t>(1)));

  auto c6 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::uint64_t>(0)));
  /* c7 = */ builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::uint64_t>(1)));

  /* c8 = */ builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::int64_t>(-1)));
  auto c9 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(std::numeric_limits<std::uint64_t>::max()));

  // --- add peephole patterns ---
  auto v10 = builder.createAndAddOp<arith::AddOp>(a0, c4);  // x + 0 -> x
  /* v11 = */ builder.createAndAddOp<arith::AddOp>(c4, a0); // 0 + x -> x

  // --- sub peephole patterns ---
  auto v12 = builder.createAndAddOp<arith::SubOp>(a0, c4);  // x - 0 -> x
  /* v13 = */ builder.createAndAddOp<arith::SubOp>(a1, a1); // x - x -> 0

  // --- mul peephole patterns ---
  auto v14 = builder.createAndAddOp<arith::MulOp>(a0, c5);  // x * 1 -> x
  /* v15 = */ builder.createAndAddOp<arith::MulOp>(c5, a0); // 1 * x -> x
  /* v16 = */ builder.createAndAddOp<arith::MulOp>(a0, c4); // x * 0 -> 0
  /* v17 = */ builder.createAndAddOp<arith::MulOp>(c4, a1); // 0 * x -> 0

  // --- div peephole patterns ---
  auto v18 = builder.createAndAddOp<arith::DivOp>(a0, c5); // x / 1 -> x

  // --- and peephole patterns (unsigned) ---
  /* v19 = */ builder.createAndAddOp<arith::AndOp>(a2, c6); // x & 0 -> 0
  /* v20 = */ builder.createAndAddOp<arith::AndOp>(c6, a2); // 0 & x -> 0
  auto v21 = builder.createAndAddOp<arith::AndOp>(a2, c9);  // x & all1 -> x
  /* v22 = */ builder.createAndAddOp<arith::AndOp>(c9, a2); // all1 & x -> x
  /* v23 = */ builder.createAndAddOp<arith::AndOp>(a3, a3); // x & x -> x

  // --- or peephole patterns (unsigned) ---
  /* v24 = */ builder.createAndAddOp<arith::OrOp>(a2, c6); // x | 0 -> x
  /* v25 = */ builder.createAndAddOp<arith::OrOp>(c6, a2); // 0 | x -> x
  auto v26 = builder.createAndAddOp<arith::OrOp>(a2, c9);  // x | all1 -> all1
  /* v27 = */ builder.createAndAddOp<arith::OrOp>(c9, a2); // all1 | x -> all1
  /* v28 = */ builder.createAndAddOp<arith::OrOp>(a3, a3); // x | x -> x

  // --- xor peephole patterns (unsigned) ---
  /* v29 = */ builder.createAndAddOp<arith::XorOp>(a2, c6); // x ^ 0 -> x
  /* v30 = */ builder.createAndAddOp<arith::XorOp>(c6, a2); // 0 ^ x -> x
  /* v31 = */ builder.createAndAddOp<arith::XorOp>(a3, a3); // x ^ x -> 0
  /* v32 = */ builder.createAndAddOp<arith::XorOp>(a2,
                                                   c9); // x ^ all1 -> not(x)
  /* v33 = */ builder.createAndAddOp<arith::XorOp>(c9,
                                                   a2); // all1 ^ x -> not(x)

  // --- shift peephole patterns ---
  /* v34 = */ builder.createAndAddOp<arith::SalOp>(a0, c4); // x << 0 -> x
  /* v35 = */ builder.createAndAddOp<arith::SalOp>(c4, a0); // 0 << x -> 0

  /* v36 = */ builder.createAndAddOp<arith::SarOp>(a0, c4); // x >> 0 -> x
  /* v37 = */ builder.createAndAddOp<arith::SarOp>(c4, a0); // 0 >> x -> 0

  /* v38 = */ builder.createAndAddOp<arith::ShlOp>(a2, c6); // x << 0 -> x
  /* v39 = */ builder.createAndAddOp<arith::ShlOp>(c6, a2); // 0 << x -> 0

  /* v40 = */ builder.createAndAddOp<arith::ShrOp>(a2, c6); // x >> 0 -> x
  /* v41 = */ builder.createAndAddOp<arith::ShrOp>(c6, a2); // 0 >> x -> 0

  // --- not peephole pattern: double negation ---
  auto v42 = builder.createAndAddOp<arith::NotOp>(a2);
  auto v43 = builder.createAndAddOp<arith::NotOp>(v42);

  auto v44 = builder.createAndAddOp<arith::AddOp>(v10, v12);
  auto v45 = builder.createAndAddOp<arith::AddOp>(v14, v18);
  /* v46 = */ builder.createAndAddOp<arith::AndOp>(v21, v26);

  /* v47 = */ builder.createAndAddOp<arith::AddOp>(v44, v45);

  builder.createAndAddOp<ctrlflow::ReturnOp>(v43);

  auto& bb0 = builder.finalizeCurBasicBlock();

  auto regionPtr = builder.obtainRegion();
  regionPtr->setStartBasicBlock(&bb0);
  regionPtr->setFinalBasicBlock(&bb0);

  std::string msg;
  if (!regionPtr->verify(msg)) {
    std::cerr << "Verification failed (before PeepHole): " << msg << std::endl;
  }

  std::cout << "==============================" << std::endl;
  std::cout << "Region before ArithPeepHole:" << std::endl;
  regionPtr->dump(std::cout);

  opt::PassManager pmPH;
  pmPH.addPass(std::make_unique<opt::arith::ArithPeepHolePass>());
  pmPH.run(*regionPtr);

  if (!regionPtr->verify(msg)) {
    std::cerr << "Verification failed (after PeepHole): " << msg << std::endl;
  }

  std::cout << "==============================" << std::endl;
  std::cout << "Region after ArithPeepHole:" << std::endl;
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
