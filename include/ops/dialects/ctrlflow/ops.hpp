#ifndef INCLUDE_DIALECTS_CTRLFLOW_OPS_HPP
#define INCLUDE_DIALECTS_CTRLFLOW_OPS_HPP

#include <string_view>
#include <utility>
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

class ReturnOp : public CtrFlowOp {
public:
  ReturnOp(Input input)
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

class JumpOp : public CtrFlowOp {
public:
  enum class Pred : uint8_t {
    NONE, // Unconditional
    EQ,   // Equal
    NEQ,  // Not equal
    A,    // Above
    B,    // Below
    AE,   // Above or greater
    BE,   // Below or greater
  };

private:
  bb_id_t m_targetBbID;
  Pred m_pred;

public:
  JumpOp(bb_id_t targetBbID, Pred pred = Pred::NONE)
    : CtrFlowOp(GlobalOpcodes::JUMP, DataType::NONE)
    , m_targetBbID(targetBbID)
    , m_pred(pred) {}

  std::string_view getMnemonic() const override {
    // clang-format off
    switch (m_pred) {
      case Pred::NONE: return "jmp";     break;
      case Pred::EQ:   return "jmp.eq";  break;
      case Pred::NEQ:  return "jmp.neq"; break;
      case Pred::A:    return "jmp.a";   break;
      case Pred::B:    return "jmp.b";   break;
      case Pred::AE:   return "jmp.ae";  break;
      case Pred::BE:   return "jmp.be";  break;
    }
    std::unreachable();
    // clang-format on
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
};

class CallOp : public CtrFlowOp {
private:
  std::vector<Input> m_inputs;
  std::string m_funcName;

public:
  template <typename... Args>
  CallOp(std::string_view funcName, DataType dataType, Args&&... args)
    : CtrFlowOp(GlobalOpcodes::CALL, dataType, args...)
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
};

class PhiOp : public CtrFlowOp {
public:
  PhiOp(Input inputX, Input inputY)
    : CtrFlowOp(GlobalOpcodes::PHI, inputX.getDefiningOp()->getDataType(),
                {inputX, inputY}) {}

  std::string_view getMnemonic() const override {
    return "phi";
  }

  bool isTerminator() const override {
    return false;
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
