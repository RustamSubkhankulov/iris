#include <cstdint>

#include <iris.hpp>

int main() {
  using namespace iris;

  IRBuilder builder;
  builder.startNewRegion("Foo");

  // bb0: parameters & constants basic block
  builder.startNewBasicBlock();
  auto* a0 = builder.createAndAddOp<builtin::ParamOp>(DataType::UI32);
  auto* v0 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<uint64_t>(1)));
  auto* v1 = builder.createAndAddOp<arith::ConstantOp>(
    makeConstAttribute(static_cast<uint64_t>(2)));
  auto& bb0 = builder.finalizeBasicBlock();

  // bb1
  builder.startNewBasicBlock();
  auto& bb1 = builder.getCurBasicBlock();
  bb0.setSucc(bb1);

  auto* v2 = builder.createAndAddOp<arith::CastOp>(DataType::UI64, a0);
  builder.finalizeBasicBlock();

  // bb2
  builder.startNewBasicBlock();
  auto& bb2 = builder.getCurBasicBlock();
  bb1.setSucc(bb2);

  auto* cmpRes =
    builder.createAndAddOp<arith::CmpOp>(v1, v2, arith::CmpOp::Pred::EQ);
  auto done = builder.obtainIdForBasicBlock();
  builder.createAndAddOp<ctrlflow::JumpcOp>(done, cmpRes);
  builder.finalizeBasicBlock();

  // bb3
  builder.startNewBasicBlock();
  auto& bb3 = builder.getCurBasicBlock();
  bb2.setSucc(bb3, false);

  auto* res = builder.createAndAddOp<arith::MulOp>(v0, v1);
  builder.createAndAddOp<arith::AddOp>(res, v2);
  builder.createAndAddOp<ctrlflow::JumpOp>(bb2.getID());
  builder.finalizeBasicBlock();

  // bb4
  builder.startNewBasicBlock(done);
  auto& bb4 = builder.getCurBasicBlock();
  bb3.setSucc(bb4);
  bb2.setSucc(bb4, true);

  auto* bar = builder.createAndAddOp<ctrlflow::CallOp>("bar", DataType::BOOL,
                                                       InputList{v0, v1});
  builder.createAndAddOp<ctrlflow::ReturnOp>(bar);
  builder.finalizeBasicBlock();

  auto regionPtr = builder.obtainRegion();
  regionPtr->dump(std::cout);

  std::string msg;
  if (!regionPtr->verify(msg)) {
    std::cerr << "Verification failed:" << std::endl;
    std::cerr << msg;
  }

  return 0;
}