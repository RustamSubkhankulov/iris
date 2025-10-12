#ifndef INCLUDE_OPS_GENERIC_USER_HPP
#define INCLUDE_OPS_GENERIC_USER_HPP

#include <cstddef>

#include <exception.hpp>

namespace iris {

class Operation;

// 'User' is a pair {op, inputIndex}, where:
//   - 'op' - operation, that uses smth - result of the another operation
//   generally;
//   - 'inputIndex' - index of the input, which corresponds to used value
//   result.
// It is more correct to call this pair a 'use', rather than 'user', but this
// name is kept for historical reasons.
class User {
public:
  User(Operation* op, std::size_t inputIndex);

  User(const User&) = default;
  User& operator=(const User&) = default;

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

private:
  Operation* m_op;
  std::size_t m_inputIndex;
};

} // namespace iris

#endif // INCLUDE_OPS_GENERIC_USER_HPP
