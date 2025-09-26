#include <ops/generic/input.hpp>
#include <ops/generic/operation.hpp>
#include <ops/generic/user.hpp>

namespace iris {

void Operation::replaceUses(Operation&& other) noexcept {
  // Replace 'other' operation with this operation in every user's input
  for (auto& user : other.m_users) {
    auto* userOp = user.getUserOp();
    auto& input = userOp->m_inputs.at(user.getInputIndex());
    input.setDefiningOp(this);
  }

  m_users = std::move(other.m_users);
  m_inputs = other.m_inputs;

  // Replace 'other' with this operation in users list of every input
  for (std::size_t inputIdx = 0; inputIdx < m_inputsNumber; ++inputIdx) {
    auto& input = m_inputs.at(inputIdx);
    auto* defOp = input.getDefiningOp();

    auto& users = defOp->m_users;
    for (auto userIt = users.begin(); userIt != users.end(); ++userIt) {
      if (userIt->getUserOp() == &other &&
          userIt->getInputIndex() == inputIdx) {
        users.erase(userIt);
        users.push_back(User{this, inputIdx});
      }
    }
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
      input.getDefiningOp()->printID(os);

      if (inputIndex != m_inputsNumber - 1) {
        os << ",";
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