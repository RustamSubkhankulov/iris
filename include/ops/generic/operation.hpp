#ifndef INCLUDE_OPS_GENERIC_OPERATION_HPP
#define INCLUDE_OPS_GENERIC_OPERATION_HPP

#include <cassert>
#include <cstdint>
#include <list>
#include <ostream>
#include <string>
#include <utility>

#include <data_types.hpp>
#include <ops/common.hpp>
#include <ops/generic/input.hpp>
#include <ops/generic/user.hpp>
#include <ops/types.hpp>
#include <utils.hpp>

namespace iris {

class BasicBlock;

// Operation base class
class Operation : public ListNode {
private:
  // Operation code - unique for each operation
  // (type of operation, not an instance)
  opcode_t m_opcode = nullopcode;

  std::list<User> m_users;

  BasicBlock* m_ParentBlockPtr = nullptr;
  friend class BasicBlock;

protected:
  // Resulting data type of the operation
  // DataType::NONE if operation has no result
  DataType m_dataType = DataType::NONE;

  // Number of inputs of the operation
  std::size_t m_inputsNumber = 0LLU;

  // Idenditier of the operation
  std::size_t m_ID = 0LLU;

public:
  // Default constructor - constructs an empty op.
  // Opcode 'nullopcode' is reserved to represent an empty op.
  Operation() = default;

  Operation(opcode_t opcode, DataType dataType, std::size_t inputsNumber)
    : m_opcode(opcode)
    , m_dataType(dataType)
    , m_inputsNumber(inputsNumber) {}

  // TODO: implement
  Operation(const Operation& that) = delete;
  Operation(Operation&& other) = delete;

  // Assignment is prohibited, since it will implicitly
  // override previous state of the operation.
  // Operation must be removed / replaced explicitly.
  Operation& operator=(const Operation&) = delete;
  Operation& operator=(Operation&& other) = delete;

  virtual ~Operation() = default;

  operator bool() {
    return (m_opcode != nullopcode);
  }

  //--- Mnemonics of the operation ---

  // Mnemonis is unique string that corresponds to each operation
  virtual std::string_view getMnemonic() const = 0;

  // Name of the dialect - group of operations that this operation belongs to
  virtual std::string_view getDialectName() const = 0;

  //--- Operation's parent basic block

  bool hasParentBasicBlock() const {
    return (m_ParentBlockPtr != nullptr);
  }

  BasicBlock* getParentBasicBlock() {
    return m_ParentBlockPtr;
  }

  const BasicBlock* getParentBasicBlock() const {
    return m_ParentBlockPtr;
  }

  //--- General properties of the operation ---

  virtual bool isTerminator() const = 0;

  DataType getDataType() const {
    return m_dataType;
  }
  bool hasResult() const {
    return (getDataType() != DataType::NONE);
  }

  std::size_t getInputsNum() const {
    return m_inputsNumber;
  }
  bool hasInputs() const {
    return (getInputsNum() != 0);
  }

  // Returns nullptr if specified index does not correspond
  // to an operation's input
  virtual Input* getInputAt(std::size_t inputIndex) = 0;
  virtual const Input* getInputAt(std::size_t inputIndex) const = 0;

  virtual bool verify() const {
    // TODO: check that all users are unique
    // TODO: check that all user indexes are valid
    return true;
  }

  //--- Operation result's users ---

  const std::list<User>& getUsers() const {
    return m_users;
  }

  std::size_t getUsersNum() const {
    return m_users.size();
  }

  bool hasUsers() const {
    return (getUsersNum() != 0);
  }

  void setUsers(const std::list<User>& users) {
    m_users = users;
  }
  void setUsers(std::list<User>&& users) {
    m_users = std::move(users);
  }

  template <typename IterType>
  void removeUser(IterType iter) {
    m_users.erase(iter);
  }
  void removeUser(std::size_t pos) {
    m_users.erase(std::next(m_users.begin(), pos));
  }

  void addUser(const User& user) {
    m_users.push_back(user);
  }
  void addUser(User&& user) {
    m_users.push_back(std::move(user));
  }

  //--- Operation type identification ---

  bool isa(opcode_t opcode) const {
    return m_opcode == opcode;
  }

  bool isa(const Operation& other) const {
    return isa(other.m_opcode);
  }

  void setID(std::size_t id) {
    m_ID = id;
  }

  void print(std::ostream& os) const {
    if (hasResult()) {
      printID(os);
      os << "." << m_dataType << " ";
    }
    os << getDialectName() << "." << getMnemonic() << " ";

    if (hasInputs()) {
      os << "(";
      for (std::size_t inputIndex = 0; inputIndex < m_inputsNumber;
           ++inputIndex) {
        const Input* input = getInputAt(inputIndex);
        input->getDefiningOp()->printID(os);

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

protected:
  virtual void printID(std::ostream& os) const {
    os << "v" << m_ID;
  }
};

} // namespace iris

#endif // INCLUDE_OPS_GENERIC_OPERATION_HPP
