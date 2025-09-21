#ifndef INC_COMMON_GENERIC_OP_HPP
#define INC_COMMON_GENERIC_OP_HPP

#include <cstdint>
#include <forward_list>
#include <string_view>

#include "utils.hpp"
#include "data_types.hpp"

namespace iris {

class BasicBlock;

using opcode_t = uint8_t;
constexpr opcode_t nullopcode = 0U;

// Operation base class
class Operation : public ListNode {
private:
  // Operation code - unique for each operation
  // (type of operation, not an instance)
  opcode_t m_opcode;

  // Operation's inputs - operations that produces its operands
  std::forward_list<Operation*> m_inputs;

  // Parent basic block, where operation 
  // is belongs to.
  BasicBlock* m_ParentBlockPtr = nullptr;
  friend class BasicBlock;

public:
  // Default constructor - constructs an empty op.
  // Opcode 'nullopcode' is reserved to represent an empty op.
  Operation():
    m_opcode(nullopcode) {}

  // Operation with no inputs
  explicit Operation(opcode_t opcode):
    m_opcode(opcode) {}

  explicit Operation(opcode_t opcode, std::forward_list<Operation*>&& inputs):
    m_opcode(opcode), m_inputs(std::move(inputs)) {}

  explicit Operation(opcode_t opcode, const std::forward_list<Operation*>& inputs):
    m_opcode(opcode), m_inputs(inputs) {}

  Operation(const Operation&) = delete;
  Operation& operator=(const Operation&) = delete;

  Operation(Operation&&) = default;
  Operation& operator=(Operation&&) = default;

  virtual ~Operation() = default;

  // Mnemonis is unique string that corresponds to each operation
  virtual std::string_view getMnemonic() const = 0;

  virtual bool hasResult() const = 0;
  virtual DataType getResultDataType() const = 0;

  virtual bool isTerminator() const = 0;

  const std::forward_list<Operation*>& getInputs() const {
    return m_inputs;
  }

  bool isa(opcode_t opcode) const {
    return m_opcode == opcode;
  }

  bool isa(const Operation& other) const {
    return m_opcode == other.m_opcode;
  }
};

} // namespace iris

#endif // INC_COMMON_GENERIC_OP_HPP
