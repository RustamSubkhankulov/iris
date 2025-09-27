#include <iostream>

#include <ops/dialects/arith/ops.hpp>
#include <ops/dialects/builtin/ops.hpp>
#include <ops/dialects/ctrlflow/ops.hpp>

int main() {
  iris::Float32ConstAttribute attr1(3.14f);
  iris::Float32ConstAttribute attr2(2.27f);

  iris::builtin::ParamOp paramOp(iris::DataType::F32);
  iris::arith::ConstantOp constOp(attr1);

  iris::arith::AddOp addOp(&constOp, &paramOp);

  iris::arith::CastOp castOp(iris::DataType::SI32, &addOp);

  iris::builtin::CopyOp copyOpOld(&castOp);
  iris::builtin::CopyOp copyOpNew(std::move(copyOpOld));

  iris::ctrlflow::PhiOp phiOp(&castOp, &copyOpNew);
  iris::ctrlflow::ReturnOp returnOp;

  std::cout << paramOp << std::endl
            << constOp << std::endl
            << addOp << std::endl
            << castOp << std::endl
            << copyOpOld << std::endl
            << phiOp << std::endl
            << returnOp << std::endl
            << copyOpNew << std::endl;

  return 0;
}