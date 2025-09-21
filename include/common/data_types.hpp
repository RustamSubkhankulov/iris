#ifndef INCLUDE_COMMON_DATA_TYPE_HPP
#define INCLUDE_COMMON_DATA_TYPE_HPP

#include <iostream>

namespace iris {

namespace detail {

constexpr uint8_t FloatFlag         = 0b10000000U;
constexpr uint8_t UnsignedIntFlag   = 0b01000000U;
constexpr uint8_t SignedIntFlag     = 0b00100000U;
constexpr uint8_t BitWidthPowerMask = 0b00011111U;

} // namespace detail

enum class DataType : uint8_t {
  UI8  = detail::UnsignedIntFlag | 3,
  UI16 = detail::UnsignedIntFlag | 4,
  UI32 = detail::UnsignedIntFlag | 5,
  UI64 = detail::UnsignedIntFlag | 6,
  SI8  = detail::SignedIntFlag | 3,
  SI16 = detail::SignedIntFlag | 4,
  SI32 = detail::SignedIntFlag | 5,
  SI64 = detail::SignedIntFlag | 6,
  F32  = detail::FloatFlag | 5,
  F64  = detail::FloatFlag | 6,
  NONE
};

consteval bool isFloatingPoint(DataType DTy) {
  return static_cast<uint8_t>(DTy) & detail::FloatFlag;
}

consteval bool isInteger(DataType DTy) {
  return static_cast<uint8_t>(DTy) & (detail::UnsignedIntFlag | detail::SignedIntFlag);
}

consteval bool isUnsignedInteger(DataType DTy) {
  return static_cast<uint8_t>(DTy) & (detail::UnsignedIntFlag);
}

consteval bool isSignedInteger(DataType DTy) {
  return static_cast<uint8_t>(DTy) & (detail::SignedIntFlag);
}

std::ostream& operator<<(std::ostream& out, DataType dataType) {
  if (dataType == DataType::NONE) {
    out << "none";
    return out;
  }

  auto value = static_cast<uint8_t>(dataType);
  if (detail::FloatFlag & value) {
    out << "f";
  }

  if (detail::UnsignedIntFlag & value) {
    out << "ui";
  }

  if (detail::SignedIntFlag & value) {
    out << "si";
  }

  uint8_t power = value & detail::BitWidthPowerMask;
  out << (1 << power);

  return out;
}

} // namespace iris

#endif // INCLUDE_COMMON_DATA_TYPE_HPP
