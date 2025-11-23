#include <data_types.hpp>

namespace iris {

std::string toString(DataType dataType) {
  switch (dataType) {
  case DataType::NONE:
    return "none";
  case DataType::BOOL:
    return "b";
  case DataType::UINT:
    return "ui";
  case DataType::SINT:
    return "si";
  case DataType::FLOAT:
    return "f";
  default:
    return "none";
  }
}

} // namespace iris
