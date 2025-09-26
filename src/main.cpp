#include <iostream>

#include <ops/dialects/arith/ops.hpp>
#include <ops/dialects/builtin/ops.hpp>
#include <ops/dialects/ctrlflow/ops.hpp>

int main() {
  iris::Float32ConstAttribute attr1(3.14f);
  iris::Float32ConstAttribute attr2(2.27f);

  iris::builtin::ParamOp paramOp(iris::DataType::F32);
  paramOp.setID(0);

  iris::arith::ConstantOp constOp(attr1);
  constOp.setID(1);

  iris::arith::AddOp addOp(&constOp, &paramOp);
  addOp.setID(2);
  constOp.addUser(iris::User(&addOp, 0));
  paramOp.addUser(iris::User(&addOp, 1));

  iris::arith::CastOp castOp(iris::DataType::SI32, &addOp);
  castOp.setID(3);
  addOp.addUser(iris::User(&castOp, 0));

  iris::builtin::CopyOp copyOpOld(&castOp);
  copyOpOld.setID(4);
  castOp.addUser(iris::User(&copyOpOld, 0));

  iris::builtin::CopyOp copyOpNew(std::move(copyOpOld));
  copyOpOld.setID(7);

  iris::ctrlflow::PhiOp phiOp(&castOp, &copyOpNew);
  phiOp.setID(5);
  castOp.addUser(iris::User(&phiOp, 0));
  copyOpNew.addUser(iris::User(&phiOp, 1));

  iris::ctrlflow::ReturnOp returnOp;
  returnOp.setID(6);

  paramOp.print(std::cout);
  std::cout << std::endl;
  constOp.print(std::cout);
  std::cout << std::endl;
  addOp.print(std::cout);
  std::cout << std::endl;
  castOp.print(std::cout);
  std::cout << std::endl;
  copyOpOld.print(std::cout);
  std::cout << std::endl;
  phiOp.print(std::cout);
  std::cout << std::endl;
  returnOp.print(std::cout);
  std::cout << std::endl;
  copyOpNew.print(std::cout);
  std::cout << std::endl;

  return 0;
}