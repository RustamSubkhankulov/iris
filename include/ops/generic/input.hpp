#ifndef INCLUDE_COMMON_INPUT_HPP
#define INCLUDE_COMMON_INPUT_HPP

namespace iris {

class Operation;

class Input {
private:
  Operation* m_op;

public:
  Input(Operation* op = nullptr):
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
};

} // namespace iris

#endif // INCLUDE_COMMON_INPUT_HPP
