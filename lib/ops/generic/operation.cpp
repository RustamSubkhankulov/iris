#include <sstream>

#include <ops/generic/operation.hpp>

namespace iris {

void Operation::clearAllUses() noexcept {
  // Nulify all inputs for every user of this operation
  for (auto& user : m_users) {
    auto* userOp = user.getUserOp();
    auto& input = userOp->m_inputs.at(user.getInputIndex());
    input.clear();
  }
  m_users.clear();
}

void Operation::replaceAllUsesOf(Operation& other) noexcept {
  if (this == &other) {
    return;
  }

  // Replace 'other' operation with this operation in every user's input
  for (auto& user : other.m_users) {
    auto* userOp = user.getUserOp();
    auto& input = userOp->m_inputs.at(user.getInputIndex());
    input.setDefiningOp(this);
  }

  std::copy(other.m_users.begin(), other.m_users.end(),
            std::back_inserter(m_users));
  other.m_users.clear();
}

void Operation::replaceAllUsesWith(Operation& that) noexcept {
  that.replaceAllUsesOf(*this);
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
    throw IrisException("Operation is already in the users list!");
  }
}

void Operation::removeAsUserFromInputs() noexcept {
  for (std::size_t inputIdx = 0; inputIdx < m_inputsNumber; ++inputIdx) {
    removeAsUserFromInput(inputIdx);
  }
}

void Operation::removeAsUserFromInput(std::size_t inputIdx) noexcept {
  auto& input = m_inputs.at(inputIdx);

  if (!input.isEmpty()) {
    auto* defOp = input.getDefiningOp();
    defOp->removeUser(User{this, inputIdx});
  }
}

void Operation::clearInputs() noexcept {
  for (auto& input : m_inputs) {
    input.clear();
  }
}

template <std::input_iterator UserIt>
bool Operation::isUserUniqueWith(const User& user, UserIt usersBegin,
                                 UserIt usersEnd) noexcept {
  for (auto userIt = usersBegin; userIt != usersEnd; ++userIt) {
    if (*userIt == user) {
      return false;
    }
  }
  return true;
}

template <std::input_iterator UserIt>
bool Operation::areUsersUnique(UserIt usersBegin, UserIt usersEnd) noexcept {
  for (auto firstIt = usersBegin; firstIt != usersEnd; ++firstIt) {
    if (!isUserUniqueWith(*firstIt, std::next(firstIt), usersEnd)) {
      return false;
    }
  }
  return true;
}

bool Operation::verify(std::string& msg) const noexcept {
  // Verify that all of the inputs are non-empty.
  for (std::size_t inputIdx = 0; inputIdx < m_inputsNumber; ++inputIdx) {
    const auto& input = m_inputs[inputIdx];
    if (input.isEmpty()) {
      std::stringstream ss;
      ss << getDialectName() << "." << getMnemonic() << "'s input #" << inputIdx
         << " is empty!";
      msg = ss.str();
      return false;
    }
  }
  return true;
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