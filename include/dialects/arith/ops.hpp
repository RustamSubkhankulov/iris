#ifndef INCLUDE_DIALECTS_ARITH_OPS_HPP
#define INCLUDE_DIALECTS_ARITH_OPS_HPP

#include <iostream>
#include <string>

#include <common/operation.hpp>
#include <common/attributes.hpp>

#include <dialects/opcodes.hpp>

namespace iris {
namespace arith {

class ArithOp : public Operation {
protected:
  DataType m_dataType;

public:
  ArithOp(opcode_t opcode, DataType dataType):
    Operation(opcode), m_dataType(dataType) {}

  bool isTerminator() const override { return false; }
  
  std::string_view getDialectName() const override {
    return "arith";
  }

  bool hasResult() const override { return true; }
  DataType getDataType() const override { return m_dataType; }
};

class BinaryArithOp : public ArithOp {
protected:
  Input m_inputX;
  Input m_inputY;

public:
  BinaryArithOp(opcode_t opcode, DataType dataType, Input inputX, Input inputY):
    ArithOp(opcode, dataType), m_inputX(inputX), m_inputY(inputY) {}

  bool hasVerifier() const override { return true; }
  bool verify() const override {
    // Inputs are present
    if (!verifyInputNonEmpty("inputX", m_inputX) || !verifyInputNonEmpty("inputY", m_inputY)) {
      return false;
    }

    // Inputs have the same data types.
    if (verifyInputsDTySame()) {
      return false;
    }

    return true;
  }

  Input getInputX() const {
    return m_inputX;
  }

  Input getInputY() const {
    return m_inputY;
  }

 protected:
  bool verifyInputNonEmpty(std::string_view inputName, const Input& input) const {
    if (!input) {
      std::cerr << "Operation " << getMnemonic() << ": ";
      std::cerr << inputName << " is empty.\n";
      return false;
    }
    return true;
  }

  bool verifyInputDTy(std::string_view inputName, const Input& input) const {
    assert(!input);
    auto inputDataType = input.getDefiningOp()->getDataType();
    if (inputDataType != m_dataType) {
      std::cerr << "Operation " << getMnemonic() << ": ";
      std::cerr << inputName << " has incompatible data type.\n";
      return false;
    }
    return true;
  }

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
};

class AddOp : public BinaryArithOp {
public:
  AddOp(DataType dataType, Input inputX, Input inputY):
    BinaryArithOp(GlobalOpcodes::ADD, dataType, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "add";
  }

  bool verify() const override {
    if (!BinaryArithOp::verify()) {
      return false;
    }

    if (!verifyInputDTy("inputX", m_inputX) || !verifyInputDTy("inputY", m_inputY)) {
      return false;
    }

    return true;
  }
};

class SubOp : public BinaryArithOp {
public:
  SubOp(DataType dataType, Input inputX, Input inputY):
    BinaryArithOp(GlobalOpcodes::SUB, dataType, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "sub";
  }

  bool verify() const override {
    if (!BinaryArithOp::verify()) {
      return false;
    }

    if (!verifyInputDTy("inputX", m_inputX) || !verifyInputDTy("inputY", m_inputY)) {
      return false;
    }

    return true;
  }
};

class MulOp : public BinaryArithOp {
public:
  MulOp(DataType dataType, Input inputX, Input inputY):
    BinaryArithOp(GlobalOpcodes::MUL, dataType, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "mul";
  }

  bool verify() const override {
    if (!BinaryArithOp::verify()) {
      return false;
    }

    if (!verifyInputDTy("inputX", m_inputX) || !verifyInputDTy("inputY", m_inputY)) {
      return false;
    }

    return true;
  }
};

class DivOp : public BinaryArithOp {
public:
  DivOp(DataType dataType, Input inputX, Input inputY):
    BinaryArithOp(GlobalOpcodes::DIV, dataType, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "div";
  }

  bool verify() const override {
    if (!BinaryArithOp::verify()) {
      return false;
    }

    if (!verifyInputDTy("inputX", m_inputX) || !verifyInputDTy("inputY", m_inputY)) {
      return false;
    }

    return true;
  }
};

class ConstantOp : public ArithOp {
private:
  ConstAttribute m_attr;
public:
  ConstantOp(DataType dataType, ConstAttribute attr):
    ArithOp(GlobalOpcodes::CONST, dataType), m_attr(attr) {}

  std::string_view getMnemonic() const override {
    return "const";
  }

  bool hasVerifier() const override { return true; }
  bool verify() const override {
    auto attrDataType = m_attr.getDataType();
    if (attrDataType != m_dataType) {
      std::cerr << "Operation " << getMnemonic() << ": ";
      std::cerr << "attribute has incompatible data type.\n";
      return false;
    }
    return true;
  }
};

class CastOp : public ArithOp {
private:
  Input m_input;
public:
  CastOp(DataType dataType, Input input):
    ArithOp(GlobalOpcodes::CAST, dataType), m_input(input) {}

  std::string_view getMnemonic() const override {
    return "cast";
  }
};

class CmpOp : public BinaryArithOp {
public:
  CmpOp(Input inputX, Input inputY):
    BinaryArithOp(GlobalOpcodes::CMP, DataType::BOOL, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "cmp";
  }
};

} // namespace arith

} // namespace iris

#endif // INCLUDE_DIALECTS_BUILTIN_OPS_HPP
