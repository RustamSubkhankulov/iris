#ifndef INCLUDE_OPS_GENERIC_INPUT_HPP
#define INCLUDE_OPS_GENERIC_INPUT_HPP

#include <initializer_list>
#include <utility>

#include <data_types.hpp>
#include <exception.hpp>

namespace iris {

class Operation;

class Input {
public:
  Input(Operation* op = nullptr)
    : m_op(op) {}

  Input(const Input&) = default;
  Input& operator=(const Input&) = default;

  Input(Input&& other)
    : m_op(std::exchange(other.m_op, nullptr)) {}

  Input& operator=(Input&& other) {
    std::swap(m_op, other.m_op);
    return *this;
  }

  virtual ~Input() = default;

  const Operation* getDefiningOp() const {
    return m_op;
  }

  Operation* getDefiningOp() {
    return m_op;
  }

  void setDefiningOp(Operation* newOp) {
    m_op = newOp;
  }

  operator bool() const {
    return (m_op != nullptr);
  }

  bool isEmpty() const {
    return !(*this);
  }

  DataType getDataType() const;

private:
  Operation* m_op;
};

using InputList = std::initializer_list<Input>;

} // namespace iris

#endif // INCLUDE_OPS_GENERIC_INPUT_HPP
