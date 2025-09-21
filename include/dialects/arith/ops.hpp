#ifndef INCLUDE_DIALECTS_ARITH_OPS_HPP
#define INCLUDE_DIALECTS_ARITH_OPS_HPP

#include <iostream>
#include <string>

#include <common/operation.hpp>

namespace iris {
namespace builtin {

namespace detail {

template <opcode_t opcode>
class BinaryArithOp : public Operation {
private:
  DataType m_dataType;
  Input m_inputX;
  Input m_inputY;

public:
  BinaryArithOp(DataType dataType, Input inputX, Input inputY):
    Operation(opcode), m_dataType(dataType), m_inputX(inputX), m_inputY(inputY) {}

  bool isTerminator() const override { return false; }

  bool hasResult() const override { return true; }
  DataType getDataType() const override { return m_dataType; }

  bool hasVerifier() const override { return true; }
  bool verify() const override {
    verifyInput("inputX", m_inputX);
    verifyInput("inputY", m_inputY);
  }

  Input getInputX() const {
    return m_inputX;
  }

  Input getInputY() const {
    return m_inputY;
  }

 private:
  bool verifyInput(std::string_view inputName, const Input& input) {
    if (!input) {
      std::cerr << "Operation " << getMnemonic() << ": ";
      std::cerr << inputName << " is empty.\n";
    }

    auto inputDataType = input.getDefiningOp()->getDataType();
    if (inputDataType != m_dataType) {
      std::cerr << "Operation " << getMnemonic() << ": ";
      std::cerr << inputName << " has incompatible data type.\n";
    }
  } 
};

} // namespace detail

enum ArithOpcode : opcode_t {
  ADD, SUB, MUL, DIV
};

class AddOp : public detail::BinaryArithOp<ArithOpcode::ADD> {
public:
  std::string_view getMnemonic() const override {
    return "add";
  }
};

class SubOp : public detail::BinaryArithOp<ArithOpcode::SUB> {
public:
  std::string_view getMnemonic() const override {
    return "sub";
  }
};

class MulOp : public detail::BinaryArithOp<ArithOpcode::MUL> {
public:
  std::string_view getMnemonic() const override {
    return "sub";
  }
};

class DivOp : public detail::BinaryArithOp<ArithOpcode::DIV> {
public:
  std::string_view getMnemonic() const override {
    return "div";
  }
};

} // namespace builtin

} // namespace iris

#endif // INCLUDE_DIALECTS_BUILTIN_OPS_HPP
