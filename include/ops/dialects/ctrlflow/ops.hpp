#ifndef INCLUDE_DIALECTS_CTRLFLOW_OPS_HPP
#define INCLUDE_DIALECTS_CTRLFLOW_OPS_HPP

#include <vector>

#include <exception.hpp>
#include <ops/dialects/opcodes.hpp>
#include <ops/generic/input.hpp>
#include <ops/generic/operation.hpp>
#include <ops/generic/user.hpp>

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
    : CtrFlowOp(GlobalOpcodes::RETURN, DataType::NONE) {}

  std::string_view getMnemonic() const override {
    return "return";
  }
};

class JumpOp : public CtrFlowOp {
private:
  BasicBlock* m_targetBasicBlock;

public:
  JumpOp(BasicBlock* targetBasicBlock)
    : CtrFlowOp(GlobalOpcodes::JUMP, DataType::NONE)
    , m_targetBasicBlock(targetBasicBlock) {

    if (m_targetBasicBlock == nullptr) {
      throw IrisException("Invalid pointer to target basic block!");
    }
  }

  std::string_view getMnemonic() const override {
    return "jump";
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
  template <typename... Args>
  CallOp(BasicBlock* targetBasicBlock, DataType dataType, Args&&... args)
    : CtrFlowOp(GlobalOpcodes::CALL, dataType, args...)
    , m_targetBasicBlock(targetBasicBlock) {

    if (m_targetBasicBlock == nullptr) {
      throw IrisException("Invalid pointer to target basic block!");
    }
  }

  std::string_view getMnemonic() const override {
    return "call";
  }

  const BasicBlock* getTargetBasicBlock() const {
    return m_targetBasicBlock;
  }

  BasicBlock* getTargetBasicBlock() {
    return m_targetBasicBlock;
  }
};

class PhiOp : public CtrFlowOp {
public:
  PhiOp(Input inputX, Input inputY)
    : CtrFlowOp(GlobalOpcodes::PHI, inputX.getDefiningOp()->getDataType(),
                {inputX, inputY}) {}

  std::string_view getMnemonic() const override {
    return "phi";
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
    return getInput(0);
  }

  const Input& getInputY() const {
    return getInput(1);
  }

  Input& getInputX() {
    return getInput(0);
  }

  Input& getInputY() {
    return getInput(1);
  }

private:
  bool verifyInputsDTySame() const {
    auto inputXDTy = getInputX().getDefiningOp()->getDataType();
    auto inputYDTy = getInputY().getDefiningOp()->getDataType();
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
