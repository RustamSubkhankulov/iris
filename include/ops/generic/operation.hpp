#ifndef INCLUDE_OPS_GENERIC_OPERATION_HPP
#define INCLUDE_OPS_GENERIC_OPERATION_HPP

#include <initializer_list>
#include <iostream>
#include <list>
#include <utility>
#include <vector>

#include <data_types.hpp>
#include <exception.hpp>
#include <utils.hpp>

#include <ops/common.hpp>
#include <ops/generic/input.hpp>
#include <ops/generic/user.hpp>

namespace iris {

class User;
class BasicBlock;

// Operation base class
class Operation : private detail::ListNode {
private:
  // Operation code - unique for each operation
  // (type of operation, not an instance)
  const opcode_t m_opcode = nullopcode;

  // List of operation's users
  std::list<User> m_users;

  BasicBlock* m_ParentBlock = nullptr;
  friend class BasicBlock;

protected:
  // Resulting data type of the operation
  // DataType::NONE if operation has no result
  const DataType m_dataType = DataType::NONE;

private:
  // Vector of operation's inputs
  std::vector<Input> m_inputs;

  // Number of inputs of the operation
  const std::size_t m_inputsNumber = 0LLU;

protected:
  // Idenditier of the operation
  op_id_t m_ID = 0U;

public:
  // Default constructor - constructs an empty op.
  // Opcode 'nullopcode' is reserved to represent an empty op.
  Operation() = default;

  Operation(opcode_t opcode, DataType dataType, InputList il = {})
    : m_opcode(opcode)
    , m_dataType(dataType)
    , m_inputs(il)
    , m_inputsNumber(m_inputs.size()) {

    addAsUserToInputs();
  }

  // Operations cannot be copied or moved
  Operation(const Operation& that) = delete;

  // Make a copy of the operation with the same inputs
  // and replace all uses of it with newly constructed operation
  Operation(Operation&& other)
    : m_opcode(other.m_opcode)
    , m_ParentBlock(other.m_ParentBlock)
    , m_dataType(other.m_dataType)
    , m_inputs(other.m_inputs) // Do not move, just copy
    , m_inputsNumber(other.m_inputsNumber) {

    // TODO: what to do with ID at this point?
    addAsUserToInputs();
    replaceAllUsesOf(std::move(other));
  }

  // Assignment is prohibited, since it will implicitly
  // override previous state of the operation.
  // Operation must be removed explicitly first.
  Operation& operator=(const Operation&) = delete;
  Operation& operator=(Operation&& other) = delete;

  virtual ~Operation() {
    removeAllUses();
    removeAsUserFromInputs();
  }

  operator bool() const {
    return (m_opcode != nullopcode);
  }

  //--- Mnemonics of the operation ---

  // Mnemonis is unique string that corresponds to each operation
  virtual std::string_view getMnemonic() const = 0;

  // Name of the dialect - group of operations that this operation belongs to
  virtual std::string_view getDialectName() const = 0;

  //--- Operation's parent basic block

  bool hasParentBasicBlock() const {
    return (m_ParentBlock != nullptr);
  }

  BasicBlock* getParentBasicBlock() {
    return m_ParentBlock;
  }

  const BasicBlock* getParentBasicBlock() const {
    return m_ParentBlock;
  }

  void setParentBasicBlock(BasicBlock* basicBlock) {
    m_ParentBlock = basicBlock;
  }

  //--- General properties of the operation ---

  virtual bool isTerminator() const = 0;

  DataType getDataType() const {
    return m_dataType;
  }
  bool hasResult() const {
    return (getDataType() != DataType::NONE);
  }

  //--- Operation's inputs ---

  std::size_t getInputsNum() const {
    return m_inputsNumber;
  }
  bool hasInputs() const {
    return (getInputsNum() != 0);
  }

  const std::vector<Input>& getInputs() const {
    return m_inputs;
  }

  const Input& getInput(std::size_t index) const {
    return m_inputs[index];
  }

  const Input& getInputAt(std::size_t index) const {
    return m_inputs.at(index);
  }

  //--- Verification ---

  virtual bool verify(std::string& msg) const;

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

  //--- Operation type identification ---

  bool isa(opcode_t opcode) const {
    return m_opcode == opcode;
  }

  bool isa(const Operation& other) const {
    return isa(other.m_opcode);
  }

  //--- Misc ---

  void setID(op_id_t id) {
    m_ID = id;
  }

  op_id_t getID() const {
    return m_ID;
  }

  void print(std::ostream& os) const;

private:
  void replaceAllUsesOf(Operation&& other) noexcept;
  void removeAllUses() noexcept;

  bool addUser(const User& user) {
    if (!isUserUniqueWith(user, m_users.begin(), m_users.end())) {
      // Provided user is already in the users list
      return false;
    }
    m_users.push_back(user);
    return true;
  }

  bool addUser(User&& user) {
    if (!isUserUniqueWith(user, m_users.begin(), m_users.end())) {
      // Provided user is already in the users list
      return false;
    }
    m_users.push_back(std::move(user));
    return true;
  }

  template <typename IterType>
  void removeUser(IterType iter) {
    m_users.erase(iter);
  }

  void removeUser(std::size_t pos) {
    using iterator_t = decltype(m_users)::iterator;
    using diff_t = typename std::iterator_traits<iterator_t>::difference_type;
    m_users.erase(std::next(m_users.begin(), static_cast<diff_t>(pos)));
  }

  void removeUser(const User& user) {
    m_users.remove(user);
  }

  // void preProcessNewInput(std::size_t inputIdx, const Input& newInput);
  // void postProcessNewInput(std::size_t inputIdx, Input& newInput);

  void addAsUserToInputs();
  void addAsUserToInput(std::size_t inputIdx, Input& input);

  void removeAsUserFromInputs();
  void removeAsUserFromInput(std::size_t inputIdx);

  template <typename UserIt>
  bool isUserUniqueWith(const User& user, UserIt usersBegin, UserIt usersEnd);

  template <typename UserIt>
  bool areUsersUnique(UserIt usersBegin, UserIt usersEnd);

protected:
  virtual void printID(std::ostream& os) const {
    os << ((hasResult()) ? "v" : "n") << m_ID;
  }

  virtual void printSpecifics([[maybe_unused]] std::ostream& os) const {
    return;
  }
};

inline std::ostream& operator<<(std::ostream& os, const Operation& operation) {
  operation.print(os);
  return os;
}

} // namespace iris

#endif // INCLUDE_OPS_GENERIC_OPERATION_HPP
