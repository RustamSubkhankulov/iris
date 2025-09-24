#ifndef INCLUDE_OPS_GENERIC_INPUT_HPP
#define INCLUDE_OPS_GENERIC_INPUT_HPP

#include <cassert>
#include <utility>

namespace iris {

class Operation;

class Input {
private:
  Operation* m_op;

public:
  Input(Operation* op)
    : m_op(op) {
    assert(m_op != nullptr && "Invalid pointer to source operation");
  }

  Input(const Input&) = default;
  Input& operator=(const Input&) = default;

  Input(Input&& other)
    : m_op(std::exchange(other.m_op, nullptr)) {}

  Input& operator=(Input&& other) {
    std::swap(m_op, other.m_op);
    return *this;
  }

  virtual ~Input() = default;

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
