#include <ops/generic/operation.hpp>
#include <ops/generic/user.hpp>

namespace iris {

User::User(Operation* op, std::size_t inputIndex)
  : m_op(op)
  , m_inputIndex(inputIndex) {
  if (m_op == nullptr) {
    throw IrisException("Invalid pointer to using operation!");
  }

  if (inputIndex >= m_op->getInputsNum()) {
    throw IrisException("Invalid operation's input index!");
  }
}

} // namespace iris