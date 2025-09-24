#ifndef INCLUDE_OPS_GENERIC_ATTRIBUTES_HPP
#define INCLUDE_OPS_GENERIC_ATTRIBUTES_HPP

#include <concepts>
#include <cstdint>

#include <ops/generic/data_types.hpp>

namespace iris {

class ConstAttribute {
protected:
  DataType m_dataType;

  ConstAttribute(DataType dataType):
    m_dataType(dataType) {}

public:
  virtual ~ConstAttribute() = default;

  DataType getDataType() const {
    return m_dataType;
  }
};

namespace detail {

template<std::floating_point F, DataType DTy>
  requires (isFloatingPoint(DTy) == true)
class FloatConstAttribute : public ConstAttribute {
private:
  F m_data;
public:
  FloatConstAttribute(F data):
    ConstAttribute(DTy), m_data(data) {}

  F getData() const {
    return m_data;
  }
};

template<std::signed_integral S, DataType DTy>
  requires (isSignedInteger(DTy) == true)
class SIntegerConstAttribute : public ConstAttribute {
private:
  S m_data;
public:
  SIntegerConstAttribute(S data):
    ConstAttribute(DTy), m_data(data) {}

  S getData() const {
    return m_data;
  }
};

template<std::unsigned_integral U, DataType DTy>
  requires (isUnsignedInteger(DTy) == true)
class UIntegerConstAttribute : public ConstAttribute {
private:
  U m_data;
public:
  UIntegerConstAttribute(U data):
    ConstAttribute(DTy), m_data(data) {}

  U getData() const {
    return m_data;
  }
};

} // namespace detail

using Float32ConstAttribute = detail::FloatConstAttribute<float, DataType::F32>;
using Float64ConstAttribute = detail::FloatConstAttribute<double, DataType::F64>;

using UInteger8ConstAttribute  = detail::UIntegerConstAttribute<uint8_t,  DataType::UI8>;
using UInteger16ConstAttribute = detail::UIntegerConstAttribute<uint16_t, DataType::UI16>;
using UInteger32ConstAttribute = detail::UIntegerConstAttribute<uint32_t, DataType::UI32>;
using UInteger64ConstAttribute = detail::UIntegerConstAttribute<uint64_t, DataType::UI64>;

using SInteger8ConstAttribute  = detail::SIntegerConstAttribute<int8_t,  DataType::SI8>;
using SInteger16ConstAttribute = detail::SIntegerConstAttribute<int16_t, DataType::SI16>;
using SInteger32ConstAttribute = detail::SIntegerConstAttribute<int32_t, DataType::SI32>;
using SInteger64ConstAttribute = detail::SIntegerConstAttribute<int64_t, DataType::SI64>;

class BoolConstAttribute : public ConstAttribute {
private:
  bool m_data;
public:
  BoolConstAttribute(bool data):
    ConstAttribute(DataType::BOOL), m_data(data) {}

  bool getData() const {
    return m_data;
  }
};

} // namespace iris

#endif // INCLUDE_OPS_GENERIC_ATTRIBUTES_HPP
