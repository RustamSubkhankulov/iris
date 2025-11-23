#ifndef INCLUDE_DIALECTS_ARITH_OPS_HPP
#define INCLUDE_DIALECTS_ARITH_OPS_HPP

#include <iostream>
#include <sstream>

#include <attributes.hpp>
#include <exception.hpp>

#include <ops/dialects/opcodes.hpp>
#include <ops/generic/input.hpp>
#include <ops/generic/operation.hpp>
#include <ops/generic/user.hpp>

namespace iris {
namespace arith {

class ArithOp : public Operation {
public:
  using Operation::Operation;

  bool isTerminator() const noexcept override {
    return false;
  }

  bool hasSideEffects() const noexcept override {
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

  bool verify(std::string& msg) const noexcept override {
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
};

// Inputs does not have to be the same data type as operation's data type
using HeterogenArithOp = BinaryArithOp;

class CompareOp final : public HeterogenArithOp {
public:
  enum class Pred : uint8_t {
    EQ,  // Equal
    NEQ, // Not equal
    A,   // Above
    B,   // Below
    AE,  // Above or greater
    BE,  // Below or greater
  };

  CompareOp(Input inputX, Input inputY, Pred pred)
    : BinaryArithOp(GlobalOpcodes::COMPARE, DataType::BOOL, inputX, inputY)
    , m_pred(pred) {
    // clang-format off
      switch (m_pred) {
        case Pred::EQ:   [[fallthrough]];
        case Pred::NEQ:  [[fallthrough]];
        case Pred::A:    [[fallthrough]];
        case Pred::B:    [[fallthrough]];
        case Pred::AE:   [[fallthrough]];
        case Pred::BE:   break;
        default: throw IrisException("Unexpected predicate in CompareOp operation!");
      }
    // clang-format on
  }

  std::string_view getMnemonic() const override {
    // clang-format off
    switch (m_pred) {
      case Pred::EQ:   return "cmp.eq";
      case Pred::NEQ:  return "cmp.neq";
      case Pred::A:    return "cmp.a"; 
      case Pred::B:    return "cmp.b"; 
      case Pred::AE:   return "cmp.ae"; 
      case Pred::BE:   return "cmp.be";
      default:         return "cmp.?";
    }
    // clang-format on
  }

  Pred getPred() const {
    return m_pred;
  }

private:
  Pred m_pred;
};

// Operation's result datat type is the same as data type of the inputs
class HomogenBinaryArithOp : public BinaryArithOp {
public:
  HomogenBinaryArithOp(opcode_t opcode, Input inputX, Input inputY)
    : BinaryArithOp(opcode, inputX.getDataType(), inputX, inputY) {}
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

// Base class for binary bitwise operations (AND, OR, XOR, shifts)
class BitwiseBinaryArithOp : public HomogenBinaryArithOp {
public:
  BitwiseBinaryArithOp(opcode_t opcode, Input inputX, Input inputY)
    : HomogenBinaryArithOp(opcode, inputX, inputY) {}

  bool verify(std::string& msg) const noexcept override {
    if (!HomogenBinaryArithOp::verify(msg)) {
      return false;
    }

    auto dataTy = getInputX().getDataType();
    if (!isInteger(dataTy)) {
      std::stringstream ss;
      ss << "Operation " << getMnemonic() << ": inputs must be integer types.";
      msg = ss.str();
      return false;
    }

    return true;
  }
};

class AndOp final : public BitwiseBinaryArithOp {
public:
  AndOp(Input inputX, Input inputY)
    : BitwiseBinaryArithOp(GlobalOpcodes::AND, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "and";
  }
};

class OrOp final : public BitwiseBinaryArithOp {
public:
  OrOp(Input inputX, Input inputY)
    : BitwiseBinaryArithOp(GlobalOpcodes::OR, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "or";
  }
};

class XorOp final : public BitwiseBinaryArithOp {
public:
  XorOp(Input inputX, Input inputY)
    : BitwiseBinaryArithOp(GlobalOpcodes::XOR, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "xor";
  }
};

class SalOp final : public BitwiseBinaryArithOp {
public:
  SalOp(Input inputX, Input inputY)
    : BitwiseBinaryArithOp(GlobalOpcodes::SAL, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "sal";
  }

  bool verify(std::string& msg) const noexcept override {
    if (!BitwiseBinaryArithOp::verify(msg)) {
      return false;
    }

    auto dataTy = getInputX().getDataType();
    if (!isSInteger(dataTy)) {
      std::stringstream ss;
      ss << "Operation " << getMnemonic()
         << ": inputs must be signed integer types.";
      msg = ss.str();
      return false;
    }

    return true;
  }
};

class SarOp final : public BitwiseBinaryArithOp {
public:
  SarOp(Input inputX, Input inputY)
    : BitwiseBinaryArithOp(GlobalOpcodes::SAR, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "sar";
  }

  bool verify(std::string& msg) const noexcept override {
    if (!BitwiseBinaryArithOp::verify(msg)) {
      return false;
    }

    auto dataTy = getInputX().getDataType();
    if (!isSInteger(dataTy)) {
      std::stringstream ss;
      ss << "Operation " << getMnemonic()
         << ": inputs must be signed integer types.";
      msg = ss.str();
      return false;
    }

    return true;
  }
};

class ShlOp final : public BitwiseBinaryArithOp {
public:
  ShlOp(Input inputX, Input inputY)
    : BitwiseBinaryArithOp(GlobalOpcodes::SHL, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "shl";
  }

  bool verify(std::string& msg) const noexcept override {
    if (!BitwiseBinaryArithOp::verify(msg)) {
      return false;
    }

    auto dataTy = getInputX().getDataType();
    if (!isUInteger(dataTy)) {
      std::stringstream ss;
      ss << "Operation " << getMnemonic()
         << ": inputs must be unsigned integer types.";
      msg = ss.str();
      return false;
    }

    return true;
  }
};

class ShrOp final : public BitwiseBinaryArithOp {
public:
  ShrOp(Input inputX, Input inputY)
    : BitwiseBinaryArithOp(GlobalOpcodes::SHR, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "shr";
  }

  bool verify(std::string& msg) const noexcept override {
    if (!BitwiseBinaryArithOp::verify(msg)) {
      return false;
    }

    auto dataTy = getInputX().getDataType();
    if (!isUInteger(dataTy)) {
      std::stringstream ss;
      ss << "Operation " << getMnemonic()
         << ": inputs must be unsigned integer types.";
      msg = ss.str();
      return false;
    }

    return true;
  }
};

// Unary bitwise operations
class UnaryBitwiseArithOp : public ArithOp {
public:
  explicit UnaryBitwiseArithOp(opcode_t opcode, Input input)
    : ArithOp(opcode, input.getDataType(), {input}) {}

  const Input& getInput() const {
    return Operation::getInput(0);
  }

  bool verify(std::string& msg) const noexcept override {
    if (!ArithOp::verify(msg)) {
      return false;
    }

    auto dataTy = getInput().getDataType();
    if (!isInteger(dataTy)) {
      std::stringstream ss;
      ss << "Operation " << getMnemonic() << ": input must be an integer type.";
      msg = ss.str();
      return false;
    }

    return true;
  }
};

class NotOp final : public UnaryBitwiseArithOp {
public:
  explicit NotOp(Input input)
    : UnaryBitwiseArithOp(GlobalOpcodes::NOT, input) {}

  std::string_view getMnemonic() const override {
    return "not";
  }
};

class ConstantOp final : public ArithOp {
public:
  explicit ConstantOp(std::unique_ptr<ConstAttribute> attr)
    : ArithOp(GlobalOpcodes::CONSTANT, attr->getDataType())
    , m_attr(std::move(attr)) {}

  std::string_view getMnemonic() const override {
    return "const";
  }

  const ConstAttribute& getAttr() const {
    return *m_attr;
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

private:
  std::unique_ptr<ConstAttribute> m_attr;
};

class CastOp final : public ArithOp {
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
