#ifndef INCLUDE_OPS_GENERIC_DATA_TYPE_HPP
#define INCLUDE_OPS_GENERIC_DATA_TYPE_HPP

#include <iostream>
#include <string>

namespace iris {

namespace detail {

constexpr uint8_t BoolFlag          = 0b10000000U;
constexpr uint8_t FloatFlag         = 0b01000000U;
constexpr uint8_t UnsignedIntFlag   = 0b00100000U;
constexpr uint8_t SignedIntFlag     = 0b00010000U;
constexpr uint8_t BitWidthPowerMask = 0b00001111U;

} // namespace detail

enum class DataType : uint8_t {
  UI8  = detail::UnsignedIntFlag | 3U,
  UI16 = detail::UnsignedIntFlag | 4U,
  UI32 = detail::UnsignedIntFlag | 5U,
  UI64 = detail::UnsignedIntFlag | 6U,
  SI8  = detail::SignedIntFlag | 3U,
  SI16 = detail::SignedIntFlag | 4U,
  SI32 = detail::SignedIntFlag | 5U,
  SI64 = detail::SignedIntFlag | 6U,
  F32  = detail::FloatFlag | 5U,
  F64  = detail::FloatFlag | 6U,
  BOOL = detail::BoolFlag,
  NONE
};

constexpr bool isBool(DataType DTy) {
  return static_cast<uint8_t>(DTy) & detail::BoolFlag;
}

constexpr bool isFloatingPoint(DataType DTy) {
  return static_cast<uint8_t>(DTy) & detail::FloatFlag;
}

constexpr bool isInteger(DataType DTy) {
  return static_cast<uint8_t>(DTy) & (detail::UnsignedIntFlag | detail::SignedIntFlag);
}

constexpr bool isUnsignedInteger(DataType DTy) {
  return static_cast<uint8_t>(DTy) & (detail::UnsignedIntFlag);
}

constexpr bool isSignedInteger(DataType DTy) {
  return static_cast<uint8_t>(DTy) & (detail::SignedIntFlag);
}

constexpr uint8_t getBitWidth(DataType DTy) {
    return 1U << (static_cast<uint8_t>(DTy) & detail::BitWidthPowerMask);
}

constexpr size_t getSizeInBytes(DataType DTy) {
    if (DTy == DataType::BOOL) return 1U;
    return getBitWidth(DTy) / 8U;
}

constexpr bool isValidDataType(DataType DTy) {
    return DTy != DataType::NONE && (isBool(DTy) || isFloatingPoint(DTy) || isInteger(DTy)) && (getBitWidth(DTy) != 0);
}

static_assert(isValidDataType(DataType::UI8), "DataType is not valid, fatal");
static_assert(isValidDataType(DataType::UI16), "DataType is not valid, fatal");
static_assert(isValidDataType(DataType::UI32), "DataType is not valid, fatal");
static_assert(isValidDataType(DataType::UI64), "DataType is not valid, fatal");

static_assert(isValidDataType(DataType::SI8), "DataType is not valid, fatal");
static_assert(isValidDataType(DataType::SI16), "DataType is not valid, fatal");
static_assert(isValidDataType(DataType::SI32), "DataType is not valid, fatal");
static_assert(isValidDataType(DataType::SI64), "DataType is not valid, fatal");

static_assert(isValidDataType(DataType::F32), "DataType is not valid, fatal");
static_assert(isValidDataType(DataType::F64), "DataType is not valid, fatal");

static_assert(isValidDataType(DataType::BOOL), "DataType is not valid, fatal");

std::string toString(DataType dataType) {
  if (dataType == DataType::NONE) {
    return "none";
  }

  if (dataType == DataType::BOOL) {
    return "bool";
  }

  std::string prefix;

  auto value = static_cast<uint8_t>(dataType);
  if (detail::FloatFlag & value) {
    prefix = "f";
  } else if (detail::UnsignedIntFlag & value) {
    prefix = "ui";
  } else if (detail::SignedIntFlag & value) {
    prefix =  "si";
  }

  return prefix + std::to_string(getBitWidth(dataType));
}

std::ostream& operator<<(std::ostream& out, DataType dataType) {
  out << toString(dataType);
  return out;
}

} // namespace iris

#endif // INCLUDE_OPS_GENERIC_DATA_TYPE_HPP
