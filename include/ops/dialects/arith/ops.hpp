#ifndef INCLUDE_DIALECTS_ARITH_OPS_HPP
#define INCLUDE_DIALECTS_ARITH_OPS_HPP

#include <iostream>

#include <attributes.hpp>
#include <ops/dialects/opcodes.hpp>
#include <ops/generic/operation.hpp>

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
protected:
  Input m_inputX;
  Input m_inputY;

public:
  BinaryArithOp(opcode_t opcode, DataType dataType, Input inputX, Input inputY)
    : ArithOp(opcode, dataType, 2LLU)
    , m_inputX(inputX)
    , m_inputY(inputY) {}

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

class HomogenBinaryArithOp : public BinaryArithOp {
public:
  HomogenBinaryArithOp(opcode_t opcode, DataType dataType, Input inputX,
                       Input inputY)
    : BinaryArithOp(opcode, dataType, inputX, inputY) {}

  bool verify() const override {
    if (!BinaryArithOp::verify()) {
      return false;
    }
    // Check that inputs have the same data type as operation's data type.
    bool verX = verifyInputDTy("inputX", m_inputX);
    bool verY = verifyInputDTy("inputY", m_inputY);
    if (!verX || !verY) {
      return false;
    }
    return true;
  }

private:
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
};

class AddOp final : public HomogenBinaryArithOp {
public:
  AddOp(DataType dataType, Input inputX, Input inputY)
    : HomogenBinaryArithOp(GlobalOpcodes::ADD, dataType, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "add";
  }
};

class SubOp final : public HomogenBinaryArithOp {
public:
  SubOp(DataType dataType, Input inputX, Input inputY)
    : HomogenBinaryArithOp(GlobalOpcodes::SUB, dataType, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "sub";
  }
};

class MulOp final : public HomogenBinaryArithOp {
public:
  MulOp(DataType dataType, Input inputX, Input inputY)
    : HomogenBinaryArithOp(GlobalOpcodes::MUL, dataType, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "mul";
  }
};

class DivOp final : public HomogenBinaryArithOp {
public:
  DivOp(DataType dataType, Input inputX, Input inputY)
    : HomogenBinaryArithOp(GlobalOpcodes::DIV, dataType, inputX, inputY) {}

  std::string_view getMnemonic() const override {
    return "div";
  }
};

class ConstantOp final : public ArithOp {
private:
  ConstAttribute m_attr;

public:
  ConstantOp(DataType dataType, ConstAttribute attr)
    : ArithOp(GlobalOpcodes::CONST, dataType, 0LLU)
    , m_attr(attr) {}

  std::string_view getMnemonic() const override {
    return "const";
  }

  Input* getInputAt([[maybe_unused]] std::size_t inputIndex) override {
    return nullptr;
  }
  const Input*
  getInputAt([[maybe_unused]] std::size_t inputIndex) const override {
    return nullptr;
  }

  bool verify() const override {
    if (!ArithOp::verify()) {
      return false;
    }

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
  CastOp(DataType dataType, Input input)
    : ArithOp(GlobalOpcodes::CAST, dataType, 1LLU)
    , m_input(input) {}

  std::string_view getMnemonic() const override {
    return "cast";
  }

  Input* getInputAt(std::size_t inputIndex) override {
    return (inputIndex == 0) ? &m_input : nullptr;
  }

  const Input* getInputAt(std::size_t inputIndex) const override {
    return (inputIndex == 0) ? &m_input : nullptr;
  }

  Input& getInput() {
    return m_input;
  }

  const Input& getInput() const {
    return m_input;
  }
};

} // namespace arith
} // namespace iris

#endif // INCLUDE_DIALECTS_ARITH_OPS_HPP
