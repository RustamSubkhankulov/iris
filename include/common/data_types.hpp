#ifndef INCLUDE_COMMON_DATA_TYPE_HPP
#define INCLUDE_COMMON_DATA_TYPE_HPP

#include <iostream>

namespace {

constexpr uint8_t FloatFlag         = 0b10000000U;
constexpr uint8_t UnsignedIntFlag   = 0b01000000U;
constexpr uint8_t SignedIntFlag     = 0b00100000U;
constexpr uint8_t BitWidthPowerMask = 0b00011111U;

} // namespace

namespace iris {

enum class DataType : uint8_t {
  UI8  = UnsignedIntFlag | 3,
  UI16 = UnsignedIntFlag | 4,
  UI32 = UnsignedIntFlag | 5,
  UI64 = UnsignedIntFlag | 6,
  SI8  = SignedIntFlag | 3,
  SI16 = SignedIntFlag | 4,
  SI32 = SignedIntFlag | 5,
  SI64 = SignedIntFlag | 6,
  F32  = FloatFlag | 5,
  F64  = FloatFlag | 6,
  NONE
};

std::ostream& operator<<(std::ostream& out, DataType dataType) {
  if (dataType == DataType::NONE) {
    out << "none";
    return out;
  }

  auto value = static_cast<uint8_t>(dataType);
  if (FloatFlag & value) {
    out << "f";
  }

  if (UnsignedIntFlag & value) {
    out << "ui";
  }

  if (SignedIntFlag & value) {
    out << "si";
  }

  uint8_t power = value & BitWidthPowerMask;
  out << (2 << power);

  return out;
}

} // namespace iris

#endif // INCLUDE_COMMON_DATA_TYPE_HPP
