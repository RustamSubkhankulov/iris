#ifndef INCLUDE_DIALECTS_CTRLFLOW_OPS_HPP
#define INCLUDE_DIALECTS_CTRLFLOW_OPS_HPP

#include <vector>

#include <ops/dialects/opcodes.hpp>
#include <ops/generic/operation.hpp>

namespace iris {
namespace ctrlflow {

class CtrFlowOp : public Operation {
public:
  using Operation::Operation;

  bool isTerminator() const override {
    return true;
  }

  std::string_view getDialectName() const override {
    return "ctrlflow";
  }

  using Operation::verify;
};

class ReturnOp : public CtrFlowOp {
public:
  ReturnOp()
    : CtrFlowOp(GlobalOpcodes::RETURN, DataType::NONE, 0LLU) {}

  std::string_view getMnemonic() const override {
    return "return";
  }

  Input* getInputAt([[maybe_unused]] std::size_t inputIndex) override {
    return nullptr;
  }
  const Input*
  getInputAt([[maybe_unused]] std::size_t inputIndex) const override {
    return nullptr;
  }
};

class JumpOp : public CtrFlowOp {
private:
  BasicBlock* m_targetBasicBlock;

public:
  JumpOp(BasicBlock* targetBasicBlock)
    : CtrFlowOp(GlobalOpcodes::JUMP, DataType::NONE, 0LLU)
    , m_targetBasicBlock(targetBasicBlock) {
    assert(m_targetBasicBlock != nullptr &&
           "Invalid pointer to target basic block");
  }

  std::string_view getMnemonic() const override {
    return "jump";
  }

  Input* getInputAt([[maybe_unused]] std::size_t inputIndex) override {
    return nullptr;
  }
  const Input*
  getInputAt([[maybe_unused]] std::size_t inputIndex) const override {
    return nullptr;
  }

  const BasicBlock* getTargetBasicBlock() const {
    return m_targetBasicBlock;
  }

  BasicBlock* getTargetBasicBlock() {
    return m_targetBasicBlock;
  }
};

class CallOp : public CtrFlowOp {
private:
  std::vector<Input> m_inputs;
  BasicBlock* m_targetBasicBlock;

public:
  CallOp(BasicBlock* targetBasicBlock, DataType dataType,
         const std::vector<Input>& inputs)
    : CtrFlowOp(GlobalOpcodes::CALL, dataType, inputs.size())
    , m_inputs(inputs)
    , m_targetBasicBlock(targetBasicBlock) {
    assert(m_targetBasicBlock != nullptr &&
           "Invalid pointer to target basic block");
  }

  CallOp(BasicBlock* targetBasicBlock, DataType dataType,
         std::vector<Input>&& inputs)
    : CtrFlowOp(GlobalOpcodes::CALL, dataType, inputs.size())
    , m_inputs(std::move(inputs))
    , m_targetBasicBlock(targetBasicBlock) {
    assert(m_targetBasicBlock != nullptr &&
           "Invalid pointer to target basic block");
  }

  CallOp(BasicBlock* targetBasicBlock, DataType dataType, Input input)
    : CtrFlowOp(GlobalOpcodes::CALL, dataType, 1LLU)
    , m_inputs({input})
    , m_targetBasicBlock(targetBasicBlock) {
    assert(m_targetBasicBlock != nullptr &&
           "Invalid pointer to target basic block");
  }

  Input* getInputAt(std::size_t inputIndex) override {
    return (inputIndex < m_inputsNumber) ? &m_inputs[inputIndex] : nullptr;
  }
  const Input* getInputAt(std::size_t inputIndex) const override {
    return (inputIndex < m_inputsNumber) ? &m_inputs[inputIndex] : nullptr;
  }

  const BasicBlock* getTargetBasicBlock() const {
    return m_targetBasicBlock;
  }

  BasicBlock* getTargetBasicBlock() {
    return m_targetBasicBlock;
  }
};

} // namespace ctrlflow
} // namespace iris

#endif // INCLUDE_DIALECTS_CTRLFLOW_OPS_HPP
