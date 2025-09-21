#ifndef INCLUDE_COMMON_USER_HPP
#define INCLUDE_COMMON_USER_HPP

namespace iris {

class Operation;

class User {
private:
  Operation* m_op;

public:
  User(Operation* op = nullptr):
    m_op(op) {}

  User(const User&) = default;
  User& operator=(const User&) = default;

  User(User&& other):
    m_op(std::exchange(other.m_op, nullptr)) {}
  
  User& operator=(User&& other) {
    m_op = std::exchange(other.m_op, nullptr);
    return *this;
  }

  virtual ~User() = default;

  operator bool() const {
    return (m_op != nullptr);
  }

  Operation* getDefiningOp() const {
    return m_op;
  }
};

} // namespace iris

#endif // INCLUDE_COMMON_USER_HPP
