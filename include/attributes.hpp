#ifndef INCLUDE_ATTRIBUTES_HPP
#define INCLUDE_ATTRIBUTES_HPP

#include <cstdint>
#include <memory>
#include <ostream>
#include <concepts>

#include <data_types.hpp>

namespace iris {

class ConstAttribute {
protected:
  DataType m_dataType;

  constexpr explicit ConstAttribute(DataType dataType)
    : m_dataType(dataType) {}

public:
  virtual ~ConstAttribute() = default;

  constexpr DataType getDataType() const {
    return m_dataType;
  }

  virtual void print(std::ostream& os) const = 0;
};

class FloatConstAttribute : public ConstAttribute {
public:
  using StorageType = double;

  constexpr explicit FloatConstAttribute(StorageType data)
    : ConstAttribute(DataType::FLOAT)
    , m_data(data) {}

  constexpr StorageType getData() const {
    return m_data;
  }

  void print(std::ostream& os) const override {
    os << m_data;
  }

private:
  StorageType m_data;
};

class SIntegerConstAttribute : public ConstAttribute {
public:
  using StorageType = std::int64_t;

  constexpr explicit SIntegerConstAttribute(StorageType data)
    : ConstAttribute(DataType::SINT)
    , m_data(data) {}

  constexpr StorageType getData() const {
    return m_data;
  }

  void print(std::ostream& os) const override {
    os << m_data;
  }

private:
  StorageType m_data;
};

class UIntegerConstAttribute : public ConstAttribute {
public:
  using StorageType = std::uint64_t;

  constexpr explicit UIntegerConstAttribute(StorageType data)
    : ConstAttribute(DataType::UINT)
    , m_data(data) {}

  constexpr StorageType getData() const {
    return m_data;
  }

  void print(std::ostream& os) const override {
    os << m_data;
  }

private:
  StorageType m_data;
};

class BoolConstAttribute : public ConstAttribute {
public:
  using StorageType = bool;

  constexpr explicit BoolConstAttribute(StorageType data)
    : ConstAttribute(DataType::BOOL)
    , m_data(data) {}

  constexpr StorageType getData() const {
    return m_data;
  }

  void print(std::ostream& os) const override {
    auto flags = os.flags();
    os << std::boolalpha << m_data;
    os.flags(flags);
  }

private:
  StorageType m_data;
};

template <typename T>
concept FloatTy = std::floating_point<T>;

template <typename T>
concept UIntegerTy = std::unsigned_integral<T> && !std::is_same_v<T, bool>;

template <typename T>
concept SIntegerTy = std::signed_integral<T> && !std::is_same_v<T, bool>;

inline std::unique_ptr<ConstAttribute> makeConstAttribute(FloatTy auto value) {
  return std::make_unique<FloatConstAttribute>(
    static_cast<FloatConstAttribute::StorageType>(value));
}

inline std::unique_ptr<ConstAttribute> makeConstAttribute(SIntegerTy auto value) {
  return std::make_unique<SIntegerConstAttribute>(
    static_cast<SIntegerConstAttribute::StorageType>(value));
}

inline std::unique_ptr<ConstAttribute> makeConstAttribute(UIntegerTy auto value) {
  return std::make_unique<UIntegerConstAttribute>(
    static_cast<UIntegerConstAttribute::StorageType>(value));
}

inline std::unique_ptr<ConstAttribute> makeConstAttribute(bool value) {
  return std::make_unique<BoolConstAttribute>(value);
}

} // namespace iris

#endif // INCLUDE_ATTRIBUTES_HPP
