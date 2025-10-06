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

#include <graph/basic_block.hpp>

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

  bool isTerminator() const override {
    return true;
  }

  const Input& getInput() const {
    return Operation::getInput(0);
  }
};

class JumpOp final : public CtrFlowOp {
public:
  explicit JumpOp(bb_id_t targetBbID)
    : CtrFlowOp(GlobalOpcodes::JUMP, DataType::NONE)
    , m_targetBbID(targetBbID) {}

  std::string_view getMnemonic() const override {
    return "jmp";
  }

  bool isTerminator() const override {
    return true;
  }

  bb_id_t getTargetBbID() const {
    return m_targetBbID;
  }

  void printSpecifics(std::ostream& os) const override {
    os << "^bb" << m_targetBbID << " ";
  }

private:
  bb_id_t m_targetBbID;
};

class JumpcOp final : public CtrFlowOp {
public:
  JumpcOp(bb_id_t targetBbID, Input input)
    : CtrFlowOp(GlobalOpcodes::JUMPC, DataType::NONE, {input})
    , m_targetBbID(targetBbID) {}

  std::string_view getMnemonic() const override {
    return "jmpc";
  }

  bool isTerminator() const override {
    return true;
  }

  bb_id_t getTargetBbID() const {
    return m_targetBbID;
  }

  void printSpecifics(std::ostream& os) const override {
    os << "^bb" << m_targetBbID << " ";
  }

  bool verify(std::string& msg) const override {
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
    return Operation::getInput(1);
  }

private:
  bb_id_t m_targetBbID;
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

  bool isTerminator() const override {
    return false;
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

  std::string_view getMnemonic() const override {
    return "phi";
  }

  bool isTerminator() const override {
    return false;
  }

  bool verify(std::string& msg) const override {
    if (!CtrFlowOp::verify(msg)) {
      return false;
    }

    // Inputs have the same data types.
    if (!verifyInputsDTySame(msg)) {
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

private:
  bool verifyInputsDTySame(std::string& msg) const {
    auto inputXDTy = getInputX().getDataType();
    auto inputYDTy = getInputY().getDataType();
    if (inputXDTy != inputYDTy) {
      std::stringstream ss;
      ss << "Operation " << getMnemonic()
         << ": inputs must have same data types.";
      msg = ss.str();
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
