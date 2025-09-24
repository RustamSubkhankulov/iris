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

  std::string_view getMnemonic() const override {
    return "call";
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

class PhiOp : public CtrFlowOp {
protected:
  Input m_inputX;
  Input m_inputY;

public:
  PhiOp(Input inputX, Input inputY)
    : CtrFlowOp(GlobalOpcodes::PHI, inputX.getDefiningOp()->getDataType(), 2LLU)
    , m_inputX(inputX)
    , m_inputY(inputY) {}

  std::string_view getMnemonic() const override {
    return "phi";
  }

  const Input* getInputAt(std::size_t index) const override {
    if (index == 0LLU) {
      return &m_inputX;
    } else if (index == 1LLU) {
      return &m_inputY;
    }
    return nullptr;
  }

  Input* getInputAt(std::size_t index) override {
    if (index == 0LLU) {
      return &m_inputX;
    } else if (index == 1LLU) {
      return &m_inputY;
    }
    return nullptr;
  }

  bool verify() const override {
    if (!CtrFlowOp::verify()) {
      return false;
    }

    // Inputs have the same data types.
    if (verifyInputsDTySame()) {
      return false;
    }

    return true;
  }

  const Input& getInputX() const {
    return m_inputX;
  }

  const Input& getInputY() const {
    return m_inputY;
  }

  Input& getInputX() {
    return m_inputX;
  }

  Input& getInputY() {
    return m_inputY;
  }

private:
  bool verifyInputsDTySame() const {
    auto inputXDTy = m_inputX.getDefiningOp()->getDataType();
    auto inputYDTy = m_inputY.getDefiningOp()->getDataType();
    if (inputXDTy != inputYDTy) {
      std::cerr << "Operation " << getMnemonic() << ": ";
      std::cerr << "inputs have different data types.\n";
      return false;
    }
    return true;
  }

protected:
  void printID(std::ostream& os) const override {
    os << "v" << m_ID << "p";
  }
};

} // namespace ctrlflow
} // namespace iris

#endif // INCLUDE_DIALECTS_CTRLFLOW_OPS_HPP
