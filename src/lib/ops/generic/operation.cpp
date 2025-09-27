#include <ops/generic/operation.hpp>

namespace iris {

void Operation::replaceAllUsesOf(Operation&& other) noexcept {
  // Replace 'other' operation with this operation in every user's input
  m_users = std::move(other.m_users);
  for (auto& user : m_users) {
    auto* userOp = user.getUserOp();
    userOp->setInputAt(user.getInputIndex(), Input{this});
  }
}

void Operation::addAsUserToInputs() {
  for (std::size_t inputIdx = 0; inputIdx < m_inputsNumber; ++inputIdx) {
    addAsUserToInput(inputIdx);
  }
}

void Operation::addAsUserToInput(std::size_t inputIdx) {
  auto& input = m_inputs.at(inputIdx);

  if (!input.isEmpty()) {
    auto* defOp = input.getDefiningOp();
    auto res = defOp->addUser(User{this, inputIdx});

    if (!res) {
      std::clog << "Warning: operation was not added to the users list "
                << "since it is already present in it.\n";
    }
  }
}

template <typename UserIt>
bool Operation::isUserUniqueWith(const User& user, UserIt usersBegin,
                                 UserIt usersEnd) {
  for (auto userIt = usersBegin; userIt != usersEnd; ++userIt) {
    if (*userIt == user) {
      return false;
    }
  }
  return true;
}

template <typename UserIt>
bool Operation::areUsersUnique(UserIt usersBegin, UserIt usersEnd) {
  for (auto firstIt = usersBegin; firstIt != usersEnd; ++firstIt) {
    if (!isUserUniqueWith(*firstIt, std::next(firstIt), usersEnd)) {
      return false;
    }
  }
  return true;
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