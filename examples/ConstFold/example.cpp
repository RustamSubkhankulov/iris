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
  auto c_si10 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::int64_t>(10)));
  auto c_si3 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::int64_t>(3)));

  auto c_ui7 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::uint64_t>(7)));
  auto c_ui2 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<std::uint64_t>(2)));

  // Floating-point constants.
  auto c_f1_5 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(1.5));
  auto c_f0_5 =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(0.5));

  // Boolean constants.
  auto c_true =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(true));
  auto c_false =
    builder.createAndAddOp<arith::ConstantOp>(makeConstAttribute(false));

  // (10 + 3) -> constant, then * 3 -> constant as well.
  auto add_si = builder.createAndAddOp<arith::AddOp>(c_si10, c_si3);
  auto mul_si = builder.createAndAddOp<arith::MulOp>(add_si, c_si3);

  // (7 & 2) -> constant, then ^ 7 -> constant.
  auto and_ui = builder.createAndAddOp<arith::AndOp>(c_ui7, c_ui2);
  auto xor_ui = builder.createAndAddOp<arith::XorOp>(and_ui, c_ui7);

  // (1.5 + 0.5) / 0.5 -> all folds into a single float constant.
  auto add_f = builder.createAndAddOp<arith::AddOp>(c_f1_5, c_f0_5);
  auto div_f = builder.createAndAddOp<arith::DivOp>(add_f, c_f0_5);

  // Shifts on constants.
  auto shl_ui = builder.createAndAddOp<arith::ShlOp>(c_ui7, c_ui2);
  auto sar_si = builder.createAndAddOp<arith::SarOp>(c_si10, c_si3);

  // Comparison of constants: true == false -> false.
  auto cmp_bool = builder.createAndAddOp<arith::CompareOp>(
    c_true, c_false, arith::CompareOp::Pred::EQ);

  // A mixed expression that cannot be fully folded:
  // add_mixed = a0 + 3
  auto add_mixed = builder.createAndAddOp<arith::AddOp>(a0, c_si3);

  // sum_all = mul_si (constant after folding) + add_mixed (non-constant)
  auto sum_all = builder.createAndAddOp<arith::AddOp>(mul_si, add_mixed);

  // Use some of the constants in a way that keeps them alive:
  // ui_result = xor_ui (constant) & a1 (non-constant)
  auto ui_result = builder.createAndAddOp<arith::AndOp>(xor_ui, a1);

  // Just to make sure folded float and cmp_bool are also "used":
  auto cast_bool_as_int = builder.createAndAddOp<arith::CastOp>(
    DataType::SINT, cmp_bool);
  auto float_as_int =
    builder.createAndAddOp<arith::CastOp>(DataType::SINT, div_f);

  // Final combination: sum_all + cast_bool_as_int + float_as_int (in two steps).
  auto tmp = builder.createAndAddOp<arith::AddOp>(sum_all, cast_bool_as_int);
  auto final_val = builder.createAndAddOp<arith::AddOp>(tmp, float_as_int);

  // Return only the final_val; other const-only subgraphs can be DCE'd later
  // by another pass if desired, but here we only demonstrate const folding.
  builder.createAndAddOp<ctrlflow::ReturnOp>(final_val);

  auto& bb0 = builder.finalizeCurBasicBlock();

  auto regionPtr = builder.obtainRegion();
  regionPtr->setStartBasicBlock(&bb0);
  regionPtr->setFinalBasicBlock(&bb0);

  std::string msg;
  if (!regionPtr->verify(msg)) {
    std::cerr << "Verification failed (before const fold): " << msg
              << std::endl;
  }

  std::cout << "==============================" << std::endl;
  std::cout << "Region before ArithConstFold:" << std::endl;
  regionPtr->dump(std::cout);

  opt::PassManager pm;
  pm.addPass(std::make_unique<opt::arith::ArithConstFoldPass>());
  pm.run(*regionPtr);

  if (!regionPtr->verify(msg)) {
    std::cerr << "Verification failed (after const fold): " << msg
              << std::endl;
  }

  std::cout << "==============================" << std::endl;
  std::cout << "Region after ArithConstFold:" << std::endl;
  regionPtr->dump(std::cout);

  return 0;
}
