#include <iostream>
#include <ops/generic/operation.hpp>
#include <ops/generic/attributes.hpp>

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

  // iris::arith::ConstantOp constOp1(iris::DataType::F32, attr1);
  // iris::arith::ConstantOp constOp2(iris::DataType::F32, attr2);

  // iris::arith::AddOp addOp(iris::DataType::F32, &constOp1, &constOp2);
  // iris::arith::CastOp castOp(iris::DataType::SI32, &addOp);

  return 0;
}