#ifndef INCLUDE_DATA_TYPE_HPP
#define INCLUDE_DATA_TYPE_HPP

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>

namespace iris {

enum class DataType : std::uint8_t {
  UINT,  // canonical unsigned integer (storage: std::uint64_t)
  SINT,  // canonical signed integer   (storage: std::int64_t)
  FLOAT, // canonical floating point   (storage: double)
  BOOL,
  NONE
};

constexpr bool isBool(DataType DTy) noexcept {
  return DTy == DataType::BOOL;
}

constexpr bool isFloatingPoint(DataType DTy) noexcept {
  return DTy == DataType::FLOAT;
}

constexpr bool isUInteger(DataType DTy) noexcept {
  return DTy == DataType::UINT;
}

constexpr bool isSInteger(DataType DTy) noexcept {
  return DTy == DataType::SINT;
}

constexpr bool isInteger(DataType DTy) noexcept {
  return isUInteger(DTy) || isSInteger(DTy);
}

constexpr DataType getFloatDataType() noexcept {
  return DataType::FLOAT;
}

constexpr DataType getUIntegerDataType() noexcept {
  return DataType::UINT;
}

constexpr DataType getSIntegerDataType() noexcept {
  return DataType::SINT;
}

std::string toString(DataType dataType);

inline std::ostream& operator<<(std::ostream& out, DataType dataType) {
  out << toString(dataType);
  return out;
}

} // namespace iris

#endif // INCLUDE_DATA_TYPE_HPP
