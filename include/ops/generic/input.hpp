#ifndef INCLUDE_OPS_GENERIC_INPUT_HPP
#define INCLUDE_OPS_GENERIC_INPUT_HPP

#include <cassert>

namespace iris {

class Operation;

class Input {
private:
  Operation* m_op;

public:
  explicit Input(Operation* op = nullptr):
    m_op(op) {}

  Input(const Input&) = default;
  Input& operator=(const Input&) = default;

  Input(Input&& other):
    m_op(std::exchange(other.m_op, nullptr)) {}
  
  Input& operator=(Input&& other) {
    std::swap(m_op, other.m_op);
    return *this;
  }

  virtual ~Input() = default;

  operator bool() const {
    return (m_op != nullptr);
  }

  Operation* getDefiningOp() const {
    return m_op;
  }

  void setDefiningOp(Operation* newOp) {
    assert(newOp != nullptr);
    m_op = newOp;
  }
};

} // namespace iris

#endif // INCLUDE_OPS_GENERIC_INPUT_HPP
