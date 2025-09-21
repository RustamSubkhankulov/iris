#include <iostream>
#include <common/generic_op.hpp>

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

  return 0;
}