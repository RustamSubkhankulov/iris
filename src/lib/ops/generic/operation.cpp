#include <ops/generic/operation.hpp>

namespace iris {

void Operation::replaceAllUsesOf(Operation&& other) noexcept {
  // Replace 'other' operation with this operation in every user's input
  m_users = std::move(other.m_users);
  for (auto& user : m_users) {
    auto* userOp = user.getUserOp();
    userOp->m_inputs.at(user.getInputIndex()) = Input{this};
  }
}

void Operation::removeAllUses() noexcept {
  // Nulify all inputs for every user of this operation
  for (auto& user : m_users) {
    auto* userOp = user.getUserOp();
    userOp->m_inputs.at(user.getInputIndex()) = Input{};
  }
  m_users.clear();
}

void Operation::addAsUserToInputs() {
  for (std::size_t inputIdx = 0; inputIdx < m_inputsNumber; ++inputIdx) {
    auto& input = m_inputs[inputIdx];
    if (!input.isEmpty()) {
      addAsUserToInput(inputIdx, input);
    }
  }
}

void Operation::addAsUserToInput(std::size_t inputIdx, Input& input) {
  auto* defOp = input.getDefiningOp();
  auto res = defOp->addUser(User{this, inputIdx});

  if (!res) {
    std::clog << "Warning: operation was not added to the users list "
              << "since it is already present in it.\n";
  }
}

void Operation::removeAsUserFromInputs() {
  for (std::size_t inputIdx = 0; inputIdx < m_inputsNumber; ++inputIdx) {
    removeAsUserFromInput(inputIdx);
  }
}

void Operation::removeAsUserFromInput(std::size_t inputIdx) {
  auto& input = m_inputs.at(inputIdx);

  if (!input.isEmpty()) {
    auto* defOp = input.getDefiningOp();
    defOp->removeUser(User{this, inputIdx});
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

bool Operation::verify(std::string& msg) const {
  // Verify that all of the inputs are non-empty.
  bool vres = true;
  for (const auto& input : m_inputs) {
    if (input.isEmpty()) {
      msg = "Empty input in the operation!";
      vres = false;
    }
  }
  return vres;
}

void Operation::print(std::ostream& os) const {
  printID(os);
  if (hasResult()) {
    os << "." << m_dataType;
  }
  os << " ";

  os << getDialectName() << "." << getMnemonic() << " ";
  printSpecifics(os);

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