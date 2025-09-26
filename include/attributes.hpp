#ifndef INCLUDE_ATTRIBUTES_HPP
#define INCLUDE_ATTRIBUTES_HPP

#include <concepts>
#include <cstdint>

#include <data_types.hpp>

namespace iris {

class ConstAttribute {
protected:
  DataType m_dataType;

  constexpr ConstAttribute(DataType dataType)
    : m_dataType(dataType) {}

public:
  virtual ~ConstAttribute() = default;

  constexpr DataType getDataType() const {
    return m_dataType;
  }
};

namespace detail {

template <std::floating_point F, DataType DTy>
  requires(isFloatingPoint(DTy) == true)
class FloatConstAttribute : public ConstAttribute {
private:
  F m_data;

public:
  constexpr FloatConstAttribute(F data)
    : ConstAttribute(DTy)
    , m_data(data) {}

  constexpr F getData() const {
    return m_data;
  }
};

template <std::signed_integral S, DataType DTy>
  requires(isSignedInteger(DTy) == true)
class SIntegerConstAttribute : public ConstAttribute {
private:
  S m_data;

public:
  constexpr SIntegerConstAttribute(S data)
    : ConstAttribute(DTy)
    , m_data(data) {}

  constexpr S getData() const {
    return m_data;
  }
};

template <std::unsigned_integral U, DataType DTy>
  requires(isUnsignedInteger(DTy) == true)
class UIntegerConstAttribute : public ConstAttribute {
private:
  U m_data;

public:
  constexpr UIntegerConstAttribute(U data)
    : ConstAttribute(DTy)
    , m_data(data) {}

  constexpr U getData() const {
    return m_data;
  }
};

} // namespace detail

using Float32ConstAttribute = detail::FloatConstAttribute<float, DataType::F32>;
using Float64ConstAttribute =
  detail::FloatConstAttribute<double, DataType::F64>;

using UInteger8ConstAttribute =
  detail::UIntegerConstAttribute<uint8_t, DataType::UI8>;
using UInteger16ConstAttribute =
  detail::UIntegerConstAttribute<uint16_t, DataType::UI16>;
using UInteger32ConstAttribute =
  detail::UIntegerConstAttribute<uint32_t, DataType::UI32>;
using UInteger64ConstAttribute =
  detail::UIntegerConstAttribute<uint64_t, DataType::UI64>;

using SInteger8ConstAttribute =
  detail::SIntegerConstAttribute<int8_t, DataType::SI8>;
using SInteger16ConstAttribute =
  detail::SIntegerConstAttribute<int16_t, DataType::SI16>;
using SInteger32ConstAttribute =
  detail::SIntegerConstAttribute<int32_t, DataType::SI32>;
using SInteger64ConstAttribute =
  detail::SIntegerConstAttribute<int64_t, DataType::SI64>;

class BoolConstAttribute : public ConstAttribute {
private:
  bool m_data;

public:
  constexpr BoolConstAttribute(bool data)
    : ConstAttribute(DataType::BOOL)
    , m_data(data) {}

  constexpr bool getData() const {
    return m_data;
  }
};

constexpr Float32ConstAttribute makeConstAttribute(float value) {
  return Float32ConstAttribute{value};
}

constexpr Float64ConstAttribute makeConstAttribute(double value) {
  return Float64ConstAttribute{value};
}

constexpr SInteger8ConstAttribute makeConstAttribute(int8_t value) {
  return SInteger8ConstAttribute{value};
}

constexpr SInteger16ConstAttribute makeConstAttribute(int16_t value) {
  return SInteger16ConstAttribute{value};
}

constexpr SInteger32ConstAttribute makeConstAttribute(int32_t value) {
  return SInteger32ConstAttribute{value};
}

constexpr SInteger64ConstAttribute makeConstAttribute(int64_t value) {
  return SInteger64ConstAttribute{value};
}

constexpr UInteger8ConstAttribute makeConstAttribute(uint8_t value) {
  return UInteger8ConstAttribute{value};
}

constexpr UInteger16ConstAttribute makeConstAttribute(uint16_t value) {
  return UInteger16ConstAttribute{value};
}

constexpr UInteger32ConstAttribute makeConstAttribute(uint32_t value) {
  return UInteger32ConstAttribute{value};
}

constexpr UInteger64ConstAttribute makeConstAttribute(uint64_t value) {
  return UInteger64ConstAttribute{value};
}

constexpr BoolConstAttribute makeConstAttribute(bool value) {
  return BoolConstAttribute{value};
}

} // namespace iris

#endif // INCLUDE_ATTRIBUTES_HPP
