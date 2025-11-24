#include <iris.hpp>
#include <limits>

int main() {
  using namespace iris;

  IRBuilder builder;
  builder.startNewRegion("peephole");

  builder.startNewBasicBlock();

  // Parameters: signed and unsigned.
  auto sx0 = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto sx1 = builder.createAndAddOp<builtin::ParamOp>(DataType::SINT);
  auto ux0 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);
  auto ux1 = builder.createAndAddOp<builtin::ParamOp>(DataType::UINT);

  // Constants for various peephole patterns.
  auto c0_si = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::int64_t>(0)));
  auto c1_si = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::int64_t>(1)));

  auto c0_ui = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::uint64_t>(0)));
  auto c1_ui = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::uint64_t>(1)));

  auto cAllOnes_si = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::int64_t>(-1)));
  auto cAllOnes_ui = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(std::numeric_limits<std::uint64_t>::max()));

  // --- add peephole patterns ---
  auto add1 = builder.createAndAddOp<arith::AddOp>(sx0, c0_si); // x + 0 -> x
  auto add2 = builder.createAndAddOp<arith::AddOp>(c0_si, sx0); // 0 + x -> x

  // --- sub peephole patterns ---
  auto sub1 = builder.createAndAddOp<arith::SubOp>(sx0, c0_si); // x - 0 -> x
  auto sub2 = builder.createAndAddOp<arith::SubOp>(sx1, sx1);   // x - x -> 0

  // --- mul peephole patterns ---
  auto mul1 = builder.createAndAddOp<arith::MulOp>(sx0, c1_si); // x * 1 -> x
  auto mul2 = builder.createAndAddOp<arith::MulOp>(c1_si, sx0); // 1 * x -> x
  auto mul3 = builder.createAndAddOp<arith::MulOp>(sx0, c0_si); // x * 0 -> 0
  auto mul4 = builder.createAndAddOp<arith::MulOp>(c0_si, sx1); // 0 * x -> 0

  // --- div peephole patterns ---
  auto div1 = builder.createAndAddOp<arith::DivOp>(sx0, c1_si); // x / 1 -> x

  // --- and peephole patterns (unsigned) ---
  auto and1 = builder.createAndAddOp<arith::AndOp>(ux0, c0_ui); // x & 0 -> 0
  auto and2 = builder.createAndAddOp<arith::AndOp>(c0_ui, ux0); // 0 & x -> 0
  auto and3 =
    builder.createAndAddOp<arith::AndOp>(ux0, cAllOnes_ui); // x & all1 -> x
  auto and4 =
    builder.createAndAddOp<arith::AndOp>(cAllOnes_ui, ux0);   // all1 & x -> x
  auto and5 = builder.createAndAddOp<arith::AndOp>(ux1, ux1); // x & x -> x

  // --- or peephole patterns (unsigned) ---
  auto or1 = builder.createAndAddOp<arith::OrOp>(ux0, c0_ui); // x | 0 -> x
  auto or2 = builder.createAndAddOp<arith::OrOp>(c0_ui, ux0); // 0 | x -> x
  auto or3 =
    builder.createAndAddOp<arith::OrOp>(ux0, cAllOnes_ui); // x | all1 -> all1
  auto or4 =
    builder.createAndAddOp<arith::OrOp>(cAllOnes_ui, ux0);  // all1 | x -> all1
  auto or5 = builder.createAndAddOp<arith::OrOp>(ux1, ux1); // x | x -> x

  // --- xor peephole patterns (unsigned) ---
  auto xor1 = builder.createAndAddOp<arith::XorOp>(ux0, c0_ui); // x ^ 0 -> x
  auto xor2 = builder.createAndAddOp<arith::XorOp>(c0_ui, ux0); // 0 ^ x -> x
  auto xor3 = builder.createAndAddOp<arith::XorOp>(ux1, ux1);   // x ^ x -> 0
  auto xor4 = builder.createAndAddOp<arith::XorOp>(
    ux0, cAllOnes_ui); // x ^ all1 -> not(x)
  auto xor5 = builder.createAndAddOp<arith::XorOp>(cAllOnes_ui,
                                                   ux0); // all1 ^ x -> not(x)

  // --- shift peephole patterns ---
  // sal / sar: signed
  auto sal_zero_shift =
    builder.createAndAddOp<arith::SalOp>(sx0, c0_si); // x << 0 -> x
  auto sal_zero_arg =
    builder.createAndAddOp<arith::SalOp>(c0_si, sx0); // 0 << x -> 0

  auto sar_zero_shift =
    builder.createAndAddOp<arith::SarOp>(sx0, c0_si); // x >> 0 -> x
  auto sar_zero_arg =
    builder.createAndAddOp<arith::SarOp>(c0_si, sx0); // 0 >> x -> 0

  // shl / shr: unsigned
  auto shl_zero_shift =
    builder.createAndAddOp<arith::ShlOp>(ux0, c0_ui); // x << 0 -> x
  auto shl_zero_arg =
    builder.createAndAddOp<arith::ShlOp>(c0_ui, ux0); // 0 << x -> 0

  auto shr_zero_shift =
    builder.createAndAddOp<arith::ShrOp>(ux0, c0_ui); // x >> 0 -> x
  auto shr_zero_arg =
    builder.createAndAddOp<arith::ShrOp>(c0_ui, ux0); // 0 >> x -> 0

  // --- not peephole pattern: double negation ---
  auto not_inner = builder.createAndAddOp<arith::NotOp>(ux0);
  auto not_outer = builder.createAndAddOp<arith::NotOp>(not_inner);

  // Combine some of the optimized values so that they definitely have users.
  // For example, take:
  //   t0 = add1 + sub1      (both should become sx0)
  //   t1 = mul1 + div1      (should become sx0 + sx0 after peephole)
  //   t2 = and3 & or3       (and3 -> ux0, or3 -> all ones)
  auto t0 = builder.createAndAddOp<arith::AddOp>(add1, sub1);
  auto t1 = builder.createAndAddOp<arith::AddOp>(mul1, div1);
  auto t2 = builder.createAndAddOp<arith::AndOp>(and3, or3);

  // Final value mixes everything a bit, including the outer
  // not-double-negation.
  auto t3 = builder.createAndAddOp<arith::AddOp>(t0, t1);
  auto t4 = builder.createAndAddOp<arith::AddOp>(t3, t2);

  // Return the double-negated value to clearly show not(not(x)) -> x.
  // After peephole, the input of return should be ux0 instead of not-not chain.
  builder.createAndAddOp<ctrlflow::ReturnOp>(not_outer);

  auto& bb0 = builder.finalizeCurBasicBlock();

  auto regionPtr = builder.obtainRegion();
  regionPtr->setStartBasicBlock(&bb0);
  regionPtr->setFinalBasicBlock(&bb0);

  std::string msg;
  if (!regionPtr->verify(msg)) {
    std::cerr << "Verification failed (before peephole): " << msg << std::endl;
  }

  std::cout << "==============================" << std::endl;
  std::cout << "Region before ArithPeepHole:" << std::endl;
  regionPtr->dump(std::cout);

  opt::PassManager pm;
  pm.addPass(std::make_unique<opt::arith::ArithPeepHolePass>());
  pm.run(*regionPtr);

  if (!regionPtr->verify(msg)) {
    std::cerr << "Verification failed (after peephole): " << msg << std::endl;
  }

  std::cout << "==============================" << std::endl;
  std::cout << "Region after ArithPeepHole:" << std::endl;
  regionPtr->dump(std::cout);

  return 0;
}
