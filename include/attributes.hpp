#ifndef INCLUDE_ATTRIBUTES_HPP
#define INCLUDE_ATTRIBUTES_HPP

#include <concepts>
#include <cstdint>
#include <ostream>

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

  virtual void print(std::ostream& os) const = 0;
};

namespace detail {

template <std::floating_point F, DataType DTy>
  requires(isFloatingPoint(DTy) == true)
class FloatConstAttribute : public ConstAttribute {
public:
  constexpr FloatConstAttribute(F data)
    : ConstAttribute(DTy)
    , m_data(data) {}

  constexpr F getData() const {
    return m_data;
  }

  void print(std::ostream& os) const override {
    os << m_data;
  }

private:
  F m_data;
};

template <std::signed_integral S, DataType DTy>
  requires(isSignedInteger(DTy) == true)
class SIntegerConstAttribute : public ConstAttribute {
public:
  constexpr SIntegerConstAttribute(S data)
    : ConstAttribute(DTy)
    , m_data(data) {}

  constexpr S getData() const {
    return m_data;
  }

  void print(std::ostream& os) const override {
    os << m_data;
  }

private:
  S m_data;
};

template <std::unsigned_integral U, DataType DTy>
  requires(isUnsignedInteger(DTy) == true)
class UIntegerConstAttribute : public ConstAttribute {
public:
  constexpr UIntegerConstAttribute(U data)
    : ConstAttribute(DTy)
    , m_data(data) {}

  constexpr U getData() const {
    return m_data;
  }

  void print(std::ostream& os) const override {
    os << m_data;
  }

private:
  U m_data;
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
public:
  constexpr BoolConstAttribute(bool data)
    : ConstAttribute(DataType::BOOL)
    , m_data(data) {}

  constexpr bool getData() const {
    return m_data;
  }

  void print(std::ostream& os) const override {
    auto flags = os.flags();
    os << std::boolalpha << m_data;
    os.flags(flags);
  }

private:
  bool m_data;
};

inline std::unique_ptr<ConstAttribute> makeConstAttribute(float value) {
  return std::make_unique<Float32ConstAttribute>(value);
}

inline std::unique_ptr<ConstAttribute> makeConstAttribute(double value) {
  return std::make_unique<Float64ConstAttribute>(value);
}

inline std::unique_ptr<ConstAttribute> makeConstAttribute(int8_t value) {
  return std::make_unique<SInteger8ConstAttribute>(value);
}

inline std::unique_ptr<ConstAttribute> makeConstAttribute(int16_t value) {
  return std::make_unique<SInteger16ConstAttribute>(value);
}

inline std::unique_ptr<ConstAttribute> makeConstAttribute(int32_t value) {
  return std::make_unique<SInteger32ConstAttribute>(value);
}

inline std::unique_ptr<ConstAttribute> makeConstAttribute(int64_t value) {
  return std::make_unique<SInteger64ConstAttribute>(value);
}

inline std::unique_ptr<ConstAttribute> makeConstAttribute(uint8_t value) {
  return std::make_unique<UInteger8ConstAttribute>(value);
}

inline std::unique_ptr<ConstAttribute> makeConstAttribute(uint16_t value) {
  return std::make_unique<UInteger16ConstAttribute>(value);
}

inline std::unique_ptr<ConstAttribute> makeConstAttribute(uint32_t value) {
  return std::make_unique<UInteger32ConstAttribute>(value);
}

inline std::unique_ptr<ConstAttribute> makeConstAttribute(uint64_t value) {
  return std::make_unique<UInteger64ConstAttribute>(value);
}

inline std::unique_ptr<ConstAttribute> makeConstAttribute(bool value) {
  return std::make_unique<BoolConstAttribute>(value);
}

} // namespace iris

#endif // INCLUDE_ATTRIBUTES_HPP
