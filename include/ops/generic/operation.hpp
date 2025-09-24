#ifndef INCLUDE_OPS_GENERIC_OPERATION_HPP
#define INCLUDE_OPS_GENERIC_OPERATION_HPP

#include <cassert>
#include <cstdint>
#include <list>
#include <string_view>
#include <utility>

#include <ops/common.hpp>
#include <ops/generic/data_types.hpp>
#include <ops/generic/input.hpp>
#include <ops/generic/user.hpp>
#include <ops/generic/utils.hpp>
#include <ops/types.hpp>

namespace iris {

class BasicBlock;

// Operation base class
class Operation : public ListNode {
private:
  // Operation code - unique for each operation
  // (type of operation, not an instance)
  opcode_t m_opcode;

  BasicBlock* m_ParentBlockPtr = nullptr;
  friend class BasicBlock;

public:
  // Default constructor - constructs an empty op.
  // Opcode 'nullopcode' is reserved to represent an empty op.
  Operation()
    : m_opcode(nullopcode) {}

  explicit Operation(opcode_t opcode)
    : m_opcode(opcode) {}

  // Copying is prohibited, new operations must be created,
  // and old operation will be replaced with new one explicitly
  Operation(const Operation& that) = delete;

  Operation(Operation&& other)
    : m_opcode(std::exchange(other.m_opcode, nullopcode))
    , m_ParentBlockPtr(std::exchange(other.m_ParentBlockPtr, nullptr)) {}

  operator bool() {
    return (m_opcode != nullopcode);
  }

  // Assignment is prohibited, since it will implicitly
  // override previous state of the operation.
  // Operation must be removed / replaced explicitly.
  Operation& operator=(const Operation&) = delete;
  Operation& operator=(Operation&& other) = delete;

  virtual ~Operation() = default;

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

  virtual DataType getDataType() const {
    return DataType::NONE;
  }
  bool hasResult() const {
    return (getDataType() != DataType::NONE);
  }

  virtual bool hasInputs() const {
    return false;
  }
  virtual std::size_t getInputsNum() const {
    return 0LLU;
  }

  virtual const Input& getInputAt(std::size_t index) const = 0;
  virtual Input& getInputAt(std::size_t index) = 0;

  //--- Operation type identification ---

  bool isa(opcode_t opcode) const {
    return m_opcode == opcode;
  }

  bool isa(const Operation& other) const {
    return isa(other.m_opcode);
  }
};

class MaterialOperation : public Operation {
private:
  DataType m_dataType;
  std::list<User> m_users;

public:
  MaterialOperation(opcode_t opcode, DataType dataType)
    : Operation(opcode)
    , m_dataType(dataType) {}

  MaterialOperation(MaterialOperation&& that)
    : Operation(std::move(that))
    , m_dataType(that.m_dataType)
    , m_users(std::move(that.m_users)) {
    for (auto& user : m_users) {
      assert(user);

      std::size_t inputIndex = user.getInputIndex();
      Operation* userOp = user.getUserOp();

      userOp->getInputAt(inputIndex).setDefiningOp(this);
    }
  }

  DataType getDataType() const override {
    return m_dataType;
  }

  //--- Operation result's users ---

  const std::list<User>& getUsers() const {
    return m_users;
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
};

} // namespace iris

#endif // INCLUDE_OPS_GENERIC_OPERATION_HPP
