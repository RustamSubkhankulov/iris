#include <data_types.hpp>

namespace iris {

std::string toString(DataType dataType) {
  if (dataType == DataType::NONE) {
    return "none";
  }

  if (dataType == DataType::BOOL) {
    return "b";
  }

  std::string prefix;

  auto value = static_cast<uint8_t>(dataType);
  if (detail::FloatFlag & value) {
    prefix = "f";
  } else if (detail::UnsignedIntFlag & value) {
    prefix = "ui";
  } else if (detail::SignedIntFlag & value) {
    prefix = "si";
  }

  return prefix + std::to_string(getBitWidth(dataType));
}

} // namespace iris
