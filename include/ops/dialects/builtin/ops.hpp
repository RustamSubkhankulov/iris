#ifndef INCLUDE_DIALECTS_BUILIN_OPS_HPP
#define INCLUDE_DIALECTS_BUILIN_OPS_HPP

#include <ops/dialects/opcodes.hpp>
#include <ops/generic/input.hpp>
#include <ops/generic/operation.hpp>
#include <ops/generic/user.hpp>

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
    : BuiltinOp(GlobalOpcodes::PARAM, dataType) {}

  std::string_view getMnemonic() const override {
    return "param";
  }
};

class CopyOp : public BuiltinOp {
public:
  CopyOp(Input input)
    : BuiltinOp(GlobalOpcodes::COPY, input.getDefiningOp()->getDataType(),
                {input}) {}

  std::string_view getMnemonic() const override {
    return "copy";
  }

  Input& getInput() {
    return Operation::getInput(0);
  }

  const Input& getInput() const {
    return Operation::getInput(1);
  }
};

} // namespace builtin
} // namespace iris

#endif // INCLUDE_DIALECTS_BUILIN_OPS_HPP
