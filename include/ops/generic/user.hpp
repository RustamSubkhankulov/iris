#ifndef INCLUDE_OPS_GENERIC_USER_HPP
#define INCLUDE_OPS_GENERIC_USER_HPP

#include <utility>

#include <exception.hpp>

namespace iris {

class Operation;

class User {
private:
  Operation* m_op;
  std::size_t m_inputIndex;

public:
  User(Operation* op, std::size_t inputIndex);

  User(const User&) = default;
  User& operator=(const User&) = default;

  User(User&& other)
    : m_op(std::exchange(other.m_op, nullptr))
    , m_inputIndex(std::exchange(other.m_inputIndex, -1)) {}

  User& operator=(User&& other) {
    m_op = std::exchange(other.m_op, nullptr);
    m_inputIndex = std::exchange(other.m_inputIndex, -1);
    return *this;
  }

  virtual ~User() = default;

  const Operation* getUserOp() const {
    return m_op;
  }

  Operation* getUserOp() {
    return m_op;
  }

  std::size_t getInputIndex() const {
    return m_inputIndex;
  }

  bool compare(const User& other) const {
    return (m_op == other.m_op) && (m_inputIndex == other.m_inputIndex);
  }

  friend bool operator==(const User& lhs, const User& rhs) {
    return lhs.compare(rhs);
  }
};

} // namespace iris

#endif // INCLUDE_OPS_GENERIC_USER_HPP
