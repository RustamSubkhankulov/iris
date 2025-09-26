#ifndef INCLUDE_DIALECTS_ARITH_OPS_HPP
#define INCLUDE_DIALECTS_ARITH_OPS_HPP

#include <iostream>

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

  bool verify() const override {
    if (!ArithOp::verify()) {
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
};

// Inputs does not have to be the same data type as operation's data type
using HeterogenArithOp = BinaryArithOp;

class CmpOp : public HeterogenArithOp {
public:
  CmpOp(Input inputX, Input inputY)
    : BinaryArithOp(GlobalOpcodes::CMP, DataType::BOOL, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "cmp";
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
  ConstAttribute m_attr;

public:
  ConstantOp(ConstAttribute attr)
    : ArithOp(GlobalOpcodes::CONST, attr.getDataType())
    , m_attr(attr) {}

  std::string_view getMnemonic() const override {
    return "const";
  }
};

class CastOp : public ArithOp {
public:
  CastOp(DataType dataType, Input input)
    : ArithOp(GlobalOpcodes::CAST, dataType, {input}) {}

  std::string_view getMnemonic() const override {
    return "cast";
  }

  Input& getInput() {
    return Operation::getInput(0);
  }

  const Input& getInput() const {
    return Operation::getInput(0);
  }
};

} // namespace arith
} // namespace iris

#endif // INCLUDE_DIALECTS_ARITH_OPS_HPP
