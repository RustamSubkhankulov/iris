#ifndef INCLUDE_COMMON_OPERATION_HPP
#define INCLUDE_COMMON_OPERATION_HPP

#include <cstdint>
#include <list>
#include <string_view>
#include <utility>
#include <span>

#include <ops/types.hpp>
#include <ops/generic/utils.hpp>
#include <ops/generic/data_types.hpp>
#include <ops/generic/user.hpp>
#include <ops/generic/input.hpp>

namespace iris {

class BasicBlock;

// Operation base class
class Operation : public ListNode {
private:
  // Operation code - unique for each operation
  // (type of operation, not an instance)
  opcode_t m_opcode;

  std::list<User> m_users;

  BasicBlock* m_ParentBlockPtr = nullptr;
  friend class BasicBlock;

public:
  // Default constructor - constructs an empty op.
  // Opcode 'nullopcode' is reserved to represent an empty op.
  Operation():
    m_opcode(nullopcode) {}

  explicit Operation(opcode_t opcode):
    m_opcode(opcode) {}

  Operation(const Operation&) = delete;
  Operation& operator=(const Operation&) = delete;

  Operation(Operation&& other):
    m_opcode(std::exchange(other.m_opcode, nullopcode)),
    m_users(std::move(other.m_users)),
    m_ParentBlockPtr(std::exchange(other.m_ParentBlockPtr, nullptr)) {}

  Operation& operator=(Operation&& other) {
    m_opcode = std::exchange(other.m_opcode, nullopcode);
    m_users = std::move(other.m_users);
    m_ParentBlockPtr = std::exchange(other.m_ParentBlockPtr, nullptr);
    return *this;
  }

  virtual ~Operation() = default;

  // Mnemonis is unique string that corresponds to each operation
  virtual std::string_view getMnemonic() const = 0;
  
  virtual std::string_view getDialectName() const = 0;

  virtual bool isTerminator() const = 0;

  const std::list<User>& getUsers() const { return m_users; }
  std::list<User>& getUsers() { return m_users; }

  bool hasParentBasicBlock() const {
    return (m_ParentBlockPtr != nullptr);
  }

  BasicBlock* getParentBasicBlock() const {
    return m_ParentBlockPtr;
  }

  void addUser(User user) {
    m_users.push_back(user);
  }

  virtual bool hasResult() const { return false; }
  virtual DataType getDataType() const { return DataType::NONE; }

  virtual bool hasVerifier() const { return false; }
  virtual bool verify() const { return true; }

  bool isa(opcode_t opcode) const {
    return m_opcode == opcode;
  }

  bool isa(const Operation& other) const {
    return m_opcode == other.m_opcode;
  }
};

} // namespace iris

#endif // INCLUDE_COMMON_OPERATION_HPP
