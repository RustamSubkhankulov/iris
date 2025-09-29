#ifndef INCLUDE_DIALECTS_ARITH_OPS_HPP
#define INCLUDE_DIALECTS_ARITH_OPS_HPP

#include <iostream>
#include <sstream>

#include <attributes.hpp>
#include <ops/dialects/opcodes.hpp>
#include <ops/generic/input.hpp>
#include <ops/generic/operation.hpp>
#include <ops/generic/user.hpp>

namespace iris {
namespace arith {

class ArithOp : public Operation {
public:
  using Operation::Operation;

  bool isTerminator() const override {
    return false;
  }

  std::string_view getDialectName() const override {
    return "arith";
  }

  using Operation::verify;
};

class BinaryArithOp : public ArithOp {
public:
  BinaryArithOp(opcode_t opcode, DataType dataType, Input inputX, Input inputY)
    : ArithOp(opcode, dataType, {inputX, inputY}) {}

  bool verify(std::string& msg) const override {
    if (!ArithOp::verify(msg)) {
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
    auto inputXDTy = getInputX().getDefiningOp()->getDataType();
    auto inputYDTy = getInputY().getDefiningOp()->getDataType();
    if (inputXDTy != inputYDTy) {
      std::stringstream ss;
      ss << "Operation " << getMnemonic()
         << ": inputs must have same data types.";
      msg = ss.str();
      return false;
    }
    return true;
  }
};

// Inputs does not have to be the same data type as operation's data type
using HeterogenArithOp = BinaryArithOp;

class CmpOp : public HeterogenArithOp {
public:
  enum class Pred : uint8_t {
    EQ,  // Equal
    NEQ, // Not equal
    A,   // Above
    B,   // Below
    AE,  // Above or greater
    BE,  // Below or greater
  };

private:
  Pred m_pred;

public:
  CmpOp(Input inputX, Input inputY, Pred pred)
    : BinaryArithOp(GlobalOpcodes::CMP, DataType::BOOL, inputX, inputY)
    , m_pred(pred) {}

  std::string_view getMnemonic() const override {
    // clang-format off
    switch (m_pred) {
      case Pred::EQ:   return "cmp.eq";  break;
      case Pred::NEQ:  return "cmp.neq"; break;
      case Pred::A:    return "cmp.a";   break;
      case Pred::B:    return "cmp.b";   break;
      case Pred::AE:   return "cmp.ae";  break;
      case Pred::BE:   return "cmp.be";  break;
      default: std::unreachable();
    }
    std::unreachable();
    // clang-format on
  }

  Pred getPred() const {
    return m_pred;
  }
};

// Operation's result datat type is the same as data type of the inputs
class HomogenBinaryArithOp : public BinaryArithOp {
public:
  HomogenBinaryArithOp(opcode_t opcode, Input inputX, Input inputY)
    : BinaryArithOp(opcode, inputX.getDefiningOp()->getDataType(), inputX,
                    inputY) {}
};

class AddOp final : public HomogenBinaryArithOp {
public:
  AddOp(Input inputX, Input inputY)
    : HomogenBinaryArithOp(GlobalOpcodes::ADD, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "add";
  }
};

class SubOp final : public HomogenBinaryArithOp {
public:
  SubOp(Input inputX, Input inputY)
    : HomogenBinaryArithOp(GlobalOpcodes::SUB, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "sub";
  }
};

class MulOp final : public HomogenBinaryArithOp {
public:
  MulOp(Input inputX, Input inputY)
    : HomogenBinaryArithOp(GlobalOpcodes::MUL, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "mul";
  }
};

class DivOp final : public HomogenBinaryArithOp {
public:
  DivOp(Input inputX, Input inputY)
    : HomogenBinaryArithOp(GlobalOpcodes::DIV, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "div";
  }
};

class ConstantOp final : public ArithOp {
private:
  std::unique_ptr<ConstAttribute> m_attr;

public:
  ConstantOp(std::unique_ptr<ConstAttribute>&& attr)
    : ArithOp(GlobalOpcodes::CONST, attr->getDataType())
    , m_attr(std::move(attr)) {}

  std::string_view getMnemonic() const override {
    return "const";
  }

protected:
  void printID(std::ostream& os) const override {
    os << "c" << m_ID;
  }

  void printSpecifics(std::ostream& os) const override {
    os << "(";
    m_attr->print(os);
    os << ") ";
  }
};

class CastOp : public ArithOp {
public:
  CastOp(DataType dataType, Input input)
    : ArithOp(GlobalOpcodes::CAST, dataType, {input}) {}

  std::string_view getMnemonic() const override {
    return "cast";
  }

  const Input& getInput() const {
    return Operation::getInput(0);
  }
};

} // namespace arith
} // namespace iris

#endif // INCLUDE_DIALECTS_ARITH_OPS_HPP
