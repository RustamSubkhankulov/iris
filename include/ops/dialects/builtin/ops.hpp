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

  bool isTerminator() const noexcept override {
    return false;
  }

  bool hasSideEffects() const noexcept override {
    return false;
  }

  std::string_view getDialectName() const override {
    return "builtin";
  }
};

class ParamOp final : public BuiltinOp {
public:
  explicit ParamOp(DataType dataType)
    : BuiltinOp(GlobalOpcodes::PARAM, dataType) {}

  std::string_view getMnemonic() const override {
    return "param";
  }

protected:
  void printID(std::ostream& os) const override {
    os << "a" << m_ID;
  }
};

class CopyOp final : public BuiltinOp {
public:
  explicit CopyOp(Input input)
    : BuiltinOp(GlobalOpcodes::COPY, input.getDataType(), {input}) {}

  std::string_view getMnemonic() const override {
    return "copy";
  }

  const Input& getInput() const {
    return Operation::getInput(0);
  }
};

} // namespace builtin
} // namespace iris

#endif // INCLUDE_DIALECTS_BUILIN_OPS_HPP
