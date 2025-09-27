#include <cstdint>

#include <iris.hpp>

int main() {
  using namespace iris;

  Builder builder;
  builder.startNewRegion();

  // bb0: parameters & constants basic block
  builder.startNewBasicBlock();
  auto* a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::UI32);
  auto* v0 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<uint64_t>(1)));
  auto* v1 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<uint64_t>(2)));
  builder.finalizeBasicBlock();

  // bb1
  builder.startNewBasicBlock();
  auto* v2 = builder.createAndAddOp<arith::CastOp>(DataType::UI64, a0);
  builder.finalizeBasicBlock();

  // bb2
  builder.startNewBasicBlock();
  auto loop = builder.getCurBasicBlockID();
  builder.createAndAddOp<arith::CmpOp>(v1, v2);
  auto done = builder.obtainIdForBasicBlock();
  builder.createAndAddOp<ctrlflow::JumpOp>(done, ctrlflow::JumpOp::Pred::EQ);
  builder.finalizeBasicBlock();

  // bb3
  builder.startNewBasicBlock();
  auto* res = builder.createAndAddOp<arith::MulOp>(a0, v1);
  builder.createAndAddOp<arith::AddOp>(res, v0);
  builder.createAndAddOp<ctrlflow::JumpOp>(loop);
  builder.finalizeBasicBlock();

  // bb4
  builder.startNewBasicBlock(done);
  builder.createAndAddOp<ctrlflow::ReturnOp>();
  builder.finalizeBasicBlock();

  auto regionPtr = builder.obtainRegion();
  regionPtr->dump(std::cout);

  return 0;
}