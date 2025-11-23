#ifndef INCLUDE_DIALECTS_CTRLFLOW_OPS_HPP
#define INCLUDE_DIALECTS_CTRLFLOW_OPS_HPP

#include <sstream>
#include <string_view>
#include <vector>

#include <exception.hpp>
#include <ops/types.hpp>

#include <ops/dialects/opcodes.hpp>
#include <ops/generic/input.hpp>
#include <ops/generic/operation.hpp>
#include <ops/generic/user.hpp>

namespace iris {
namespace ctrlflow {

class CtrFlowOp : public Operation {
public:
  using Operation::Operation;

  std::string_view getDialectName() const override {
    return "ctrlflow";
  }

  using Operation::verify;
};

class ReturnOp final : public CtrFlowOp {
public:
  ReturnOp()
    : CtrFlowOp(GlobalOpcodes::RETURN, DataType::NONE) {}

  explicit ReturnOp(Input input)
    : CtrFlowOp(GlobalOpcodes::RETURN, DataType::NONE, {input}) {}

  std::string_view getMnemonic() const override {
    return "return";
  }

  bool isTerminator() const noexcept override {
    return true;
  }

  bool hasSideEffects() const noexcept override {
    return false;
  }
};

class JumpOp final : public CtrFlowOp {
public:
  JumpOp()
    : CtrFlowOp(GlobalOpcodes::JUMP, DataType::NONE) {}

  std::string_view getMnemonic() const override {
    return "jmp";
  }

  bool isTerminator() const noexcept override {
    return true;
  }

  bool hasSideEffects() const noexcept override {
    return false;
  }
};

class JumpcOp final : public CtrFlowOp {
public:
  explicit JumpcOp(Input input)
    : CtrFlowOp(GlobalOpcodes::JUMPC, DataType::NONE, {input}) {}

  std::string_view getMnemonic() const override {
    return "jmpc";
  }

  bool isTerminator() const noexcept override {
    return true;
  }

  bool hasSideEffects() const noexcept override {
    return false;
  }

  bool verify(std::string& msg) const noexcept override {
    if (!CtrFlowOp::verify(msg)) {
      return false;
    }

    const Input& input = Operation::getInput(0);
    if (input.getDataType() != DataType::BOOL) {
      msg = "Input's data type must be bool";
      return false;
    }

    return true;
  }

  const Input& getInput() const {
    return Operation::getInput(0);
  }

  Input& getInput() {
    return Operation::getInput(0);
  }
};

class CallOp final : public CtrFlowOp {
public:
  CallOp(std::string_view funcName, DataType dataType, InputList il = {})
    : CtrFlowOp(GlobalOpcodes::CALL, dataType, il)
    , m_funcName(funcName) {

    if (m_funcName.empty()) {
      throw IrisException("Invalid function name!");
    }
  }

  std::string_view getMnemonic() const override {
    return "call";
  }

  bool isTerminator() const noexcept override {
    return false;
  }

  bool hasSideEffects() const noexcept override {
    return true;
  }

  std::string_view getFuncName() const {
    return m_funcName;
  }

  void printSpecifics(std::ostream& os) const override {
    os << "@" << m_funcName << " ";
  }

private:
  std::vector<Input> m_inputs;
  std::string m_funcName;
};

class PhiOp final : public CtrFlowOp {
public:
  PhiOp(Input inputX, Input inputY)
    : CtrFlowOp(GlobalOpcodes::PHI, inputX.getDataType(), {inputX, inputY}) {}

  explicit PhiOp(InputList il)
    : CtrFlowOp(GlobalOpcodes::PHI, DataType::NONE, il) {

    if (getInputsNum() == 0U) {
      throw IrisException("Operation must have at least one input!");
    }

    setDataType(getInput(0U).getDataType());
  }

  std::string_view getMnemonic() const override {
    return "phi";
  }

  bool isTerminator() const noexcept override {
    return false;
  }

  bool hasSideEffects() const noexcept override {
    return false;
  }

  bool verify(std::string& msg) const noexcept override {
    if (!CtrFlowOp::verify(msg)) {
      return false;
    }

    // Inputs have the same data types.
    if (!verifyInputsDTySame(msg)) {
      return false;
    }

    return true;
  }

private:
  bool verifyInputsDTySame(std::string& msg) const {
    auto dataTy = getInput(0).getDataType();
    auto inputsNum = getInputsNum();

    for (std::size_t inIdx = 0U; inIdx < inputsNum; ++inIdx) {
      auto otherDataTy = getInput(inIdx).getDataType();
      if (dataTy != otherDataTy) {
        std::stringstream ss;
        ss << "Operation " << getMnemonic()
           << ": inputs must have same data types.";
        msg = ss.str();
        return false;
      }
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
