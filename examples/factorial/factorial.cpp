#include <iris.hpp>

int main() {
  using namespace iris;

  IRBuilder builder;
  builder.startNewRegion("factorial");

  // bb0: parameters & constants basic block
  builder.startNewBasicBlock();
  auto a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::UI32);
  auto c1 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<uint32_t>(1)));
  auto c2 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<uint32_t>(2)));
  auto& bb0 = builder.finalizeBasicBlock();

  // bb1: checking whether recursion is needed for computation
  builder.startNewBasicBlock();
  auto v3 = builder.createAndAddOp<arith::CmpOp>(a0, c2, arith::CmpOp::Pred::B);
  auto done = builder.obtainIdForBasicBlock();
  /* n4 */ builder.createAndAddOp<ctrlflow::JumpcOp>(v3);
  auto& bb1 = builder.finalizeBasicBlock();
  bb0.linkSucc(&bb1);

  // bb3:
  builder.startNewBasicBlock();
  auto v5 = builder.createAndAddOp<arith::SubOp>(a0, c1);
  auto v6 = builder.createAndAddOp<ctrlflow::CallOp>(
    builder.getCurRegion().getName(), DataType::UI32, InputList{v5});
  auto v7 = builder.createAndAddOp<arith::MulOp>(a0, v6);
  auto& bb3 = builder.finalizeBasicBlock();
  bb1.linkSucc(&bb3, false);

  // bb2:
  builder.startNewBasicBlock(done);
  auto v8 = builder.createAndAddOp<ctrlflow::PhiOp>(c1, v7);
  /* n9 */ builder.createAndAddOp<ctrlflow::ReturnOp>(v8);
  auto& bb2 = builder.finalizeBasicBlock();
  bb3.linkSucc(&bb2);
  bb1.linkSucc(&bb2, true);

  auto regionPtr = builder.obtainRegion();
  regionPtr->dump(std::cout);

  std::string msg;
  if (!regionPtr->verify(msg)) {
    std::cerr << "Verification failed:" << std::endl;
    std::cerr << msg;
  }

  return 0;
}