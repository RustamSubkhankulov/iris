#include <iostream>

#include <ops/generic/attributes.hpp>
#include <ops/generic/operation.hpp>

#include <ops/dialects/arith/ops.hpp>
#include <ops/dialects/builtin/ops.hpp>

int main() {
  std::cout << iris::DataType::F32 << std::endl;
  std::cout << iris::DataType::F64 << std::endl;
  std::cout << iris::DataType::UI8 << std::endl;
  std::cout << iris::DataType::UI16 << std::endl;
  std::cout << iris::DataType::UI32 << std::endl;
  std::cout << iris::DataType::UI64 << std::endl;
  std::cout << iris::DataType::SI8 << std::endl;
  std::cout << iris::DataType::SI16 << std::endl;
  std::cout << iris::DataType::SI32 << std::endl;
  std::cout << iris::DataType::SI64 << std::endl;

  iris::Float32ConstAttribute attr1(3.14f);
  iris::Float32ConstAttribute attr2(2.27f);

  iris::arith::ConstantOp constOp(iris::DataType::F32, attr1);
  iris::builtin::ParamOp paramOp(iris::DataType::F32);

  iris::arith::AddOp addOp(iris::DataType::F32, &constOp, &paramOp);
  iris::arith::CastOp castOp(iris::DataType::SI32, &addOp);

  iris::builtin::CopyOp copyOp(&castOp);

  return 0;
}