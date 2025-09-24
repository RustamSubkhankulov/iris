#ifndef INCLUDE_DIALECTS_BUILIN_OPS_HPP
#define INCLUDE_DIALECTS_BUILIN_OPS_HPP

#include <ops/dialects/opcodes.hpp>
#include <ops/generic/operation.hpp>

namespace iris {
namespace builtin {

class BuiltinOp : public Operation {
public:
  using Operation::Operation;

  bool isTerminator() const override {
    return false;
  }

  std::string_view getDialectName() const override {
    return "builtin";
  }
};

class ParamOp : public BuiltinOp {
public:
  ParamOp(DataType dataType)
    : BuiltinOp(GlobalOpcodes::PARAM, dataType, 0LLU) {}

  std::string_view getMnemonic() const override {
    return "param";
  }

  Input* getInputAt([[maybe_unused]] std::size_t inputIndex) override {
    return nullptr;
  }
  const Input*
  getInputAt([[maybe_unused]] std::size_t inputIndex) const override {
    return nullptr;
  }
};

class CopyOp : public BuiltinOp {
private:
  Input m_input;

public:
  CopyOp(Input input)
    : BuiltinOp(GlobalOpcodes::COPY, input.getDefiningOp()->getDataType(), 1LLU)
    , m_input(input) {}

  std::string_view getMnemonic() const override {
    return "copy";
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

} // namespace builtin
} // namespace iris

#endif // INCLUDE_DIALECTS_BUILIN_OPS_HPP
