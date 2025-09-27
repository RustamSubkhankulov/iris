#ifndef INCLUDE_OPS_GENERIC_OPERATION_HPP
#define INCLUDE_OPS_GENERIC_OPERATION_HPP

#include <initializer_list>
#include <iostream>
#include <list>
#include <utility>
#include <vector>

#include <data_types.hpp>
#include <utils.hpp>

#include <ops/common.hpp>
#include <ops/generic/input.hpp>
#include <ops/generic/user.hpp>
#include <ops/types.hpp>

namespace iris {

class User;
class BasicBlock;

// Operation base class
class Operation : public ListNode {
private:
  // Operation code - unique for each operation
  // (type of operation, not an instance)
  const opcode_t m_opcode = nullopcode;

  // List of operation's users
  std::list<User> m_users;

  BasicBlock* m_ParentBlockPtr = nullptr;
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
  // TODO make ID provided
  std::size_t m_ID = 0LLU;

public:
  // Default constructor - constructs an empty op.
  // Opcode 'nullopcode' is reserved to represent an empty op.
  Operation() = default;

  template <typename... Args>
  Operation(opcode_t opcode, DataType dataType, Args&&... args)
    : m_opcode(opcode)
    , m_dataType(dataType)
    , m_inputs(std::forward<Args>(args)...)
    , m_inputsNumber(m_inputs.size()) {

    addAsUserToInputs();
  }

  Operation(opcode_t opcode, DataType dataType, std::initializer_list<Input> il)
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
    , m_ParentBlockPtr(other.m_ParentBlockPtr)
    , m_dataType(other.m_dataType)
    , m_inputs(other.m_inputs) // Do not move, just copy
    , m_inputsNumber(other.m_inputsNumber) {

    addAsUserToInputs();
    replaceUsesOf(std::move(other));
  }

  // Assignment is prohibited, since it will implicitly
  // override previous state of the operation.
  // Operation must be removed explicitly first.
  Operation& operator=(const Operation&) = delete;
  Operation& operator=(Operation&& other) = delete;

  virtual ~Operation() = default;

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
  Input& getInput(std::size_t index) {
    return m_inputs[index];
  }

  const Input& getInputAt(std::size_t index) const {
    return m_inputs.at(index);
  }
  Input& getInputAt(std::size_t index) {
    return m_inputs.at(index);
  }

  void setInput(std::size_t index, const Input& input) {
    m_inputs[index] = input;
  }
  void setInput(std::size_t index, Input&& input) {
    m_inputs[index] = std::move(input);
  }

  void setInputAt(std::size_t index, const Input& input) {
    m_inputs.at(index) = input;
  }
  void setInputAt(std::size_t index, Input&& input) {
    m_inputs.at(index) = std::move(input);
  }

  //--- Verification ---

  virtual bool verify() const {

    bool vres = true;
    for (const auto& input : m_inputs) {
      if (input.isEmpty()) {
        std::cerr << "Empty input in the operation!" << std::endl;
        vres = false;
      }
    }
    return vres;
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
    // TODO verify
    m_users = users;
  }
  void setUsers(std::list<User>&& users) {
    // TODO verify
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
    // TODO verify
    m_users.push_back(user);
  }
  void addUser(User&& user) {
    // TODO verify
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

  void replaceUsesOf(Operation&& other) noexcept;

  void print(std::ostream& os) const;

private:
  // Add this operation as an user to every input
  void addAsUserToInputs();

protected:
  virtual void printID(std::ostream& os) const {
    os << "v" << m_ID;
  }
};

} // namespace iris

#endif // INCLUDE_OPS_GENERIC_OPERATION_HPP
