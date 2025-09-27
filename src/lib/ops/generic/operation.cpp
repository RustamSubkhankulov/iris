#include <ops/generic/operation.hpp>

namespace iris {

void Operation::replaceUsesOf(Operation&& other) noexcept {
  // Replace 'other' operation with this operation in every user's input
  for (auto& user : other.m_users) {
    auto* userOp = user.getUserOp();
    auto& input = userOp->m_inputs.at(user.getInputIndex());
    input.setDefiningOp(this);
  }

  m_users = std::move(other.m_users);
}

void Operation::addAsUserToInputs() {
  for (std::size_t inputIdx = 0; inputIdx < m_inputsNumber; ++inputIdx) {
    auto& input = m_inputs.at(inputIdx);

    if (input.isEmpty()) {
      continue;
    }

    auto* defOp = input.getDefiningOp();
    defOp->addUser(User{this, inputIdx});
  }
}

void Operation::print(std::ostream& os) const {
  if (hasResult()) {
    printID(os);
    os << "." << m_dataType << " ";
  }
  os << getDialectName() << "." << getMnemonic() << " ";

  if (hasInputs()) {
    os << "(";
    for (std::size_t inputIndex = 0; inputIndex < m_inputsNumber;
         ++inputIndex) {
      const Input& input = getInputAt(inputIndex);

      if (input.isEmpty()) {
        os << "none";
      } else {
        input.getDefiningOp()->printID(os);
        os << " : " << input.getDataType();
      }

      if (inputIndex != m_inputsNumber - 1) {
        os << ", ";
      }
    }
    os << ") ";
  }

  std::size_t userIndex = 0;
  std::size_t usersNumber = getUsersNum();

  if (usersNumber) {
    os << "-> (";
    for (const User& user : m_users) {
      user.getUserOp()->printID(os);

      if (userIndex++ != usersNumber - 1) {
        os << ",";
      }
    }
    os << ")";
  }
}

} // namespace iris