#include <climits>

#include <opt/lib/arith/constfold.hpp>

#include <attributes.hpp>
#include <ops/dialects/arith/ops.hpp>

namespace iris {
namespace opt {
namespace arith {

namespace {

using iris::BoolConstAttribute;
using iris::ConstAttribute;
using iris::DataType;
using iris::FloatConstAttribute;
using iris::SIntegerConstAttribute;
using iris::UIntegerConstAttribute;

using iris::arith::ConstantOp;

// === Helpers ===============================================================

template <typename F>
std::unique_ptr<ConstAttribute>
foldNumericBinary(DataType dataType, const ConstAttribute& lhs,
                  const ConstAttribute& rhs, F&& func) {
  switch (dataType) {
  case DataType::FLOAT: {
    auto a = static_cast<const FloatConstAttribute&>(lhs).getData();
    auto b = static_cast<const FloatConstAttribute&>(rhs).getData();
    return makeConstAttribute(func(a, b));
  }
  case DataType::SINT: {
    auto a = static_cast<const SIntegerConstAttribute&>(lhs).getData();
    auto b = static_cast<const SIntegerConstAttribute&>(rhs).getData();
    return makeConstAttribute(func(a, b));
  }
  case DataType::UINT: {
    auto a = static_cast<const UIntegerConstAttribute&>(lhs).getData();
    auto b = static_cast<const UIntegerConstAttribute&>(rhs).getData();
    return makeConstAttribute(func(a, b));
  }
  default:
    return nullptr;
  }
}

template <typename F>
std::unique_ptr<ConstAttribute>
foldIntegerBinary(DataType dataType, const ConstAttribute& lhs,
                  const ConstAttribute& rhs, F&& func) {
  switch (dataType) {
  case DataType::SINT: {
    auto a = static_cast<const SIntegerConstAttribute&>(lhs).getData();
    auto b = static_cast<const SIntegerConstAttribute&>(rhs).getData();
    return makeConstAttribute(func(a, b));
  }
  case DataType::UINT: {
    auto a = static_cast<const UIntegerConstAttribute&>(lhs).getData();
    auto b = static_cast<const UIntegerConstAttribute&>(rhs).getData();
    return makeConstAttribute(func(a, b));
  }
  default:
    return nullptr;
  }
}

template <typename F>
std::unique_ptr<ConstAttribute>
foldComparison(DataType dataType, const ConstAttribute& lhs,
               const ConstAttribute& rhs, F&& cmp) {
  bool result = false;
  bool failed = false;

  switch (dataType) {
  case DataType::FLOAT: {
    auto a = static_cast<const FloatConstAttribute&>(lhs).getData();
    auto b = static_cast<const FloatConstAttribute&>(rhs).getData();
    result = cmp(a, b);
    break;
  }
  case DataType::SINT: {
    auto a = static_cast<const SIntegerConstAttribute&>(lhs).getData();
    auto b = static_cast<const SIntegerConstAttribute&>(rhs).getData();
    result = cmp(a, b);
    break;
  }
  case DataType::UINT: {
    auto a = static_cast<const UIntegerConstAttribute&>(lhs).getData();
    auto b = static_cast<const UIntegerConstAttribute&>(rhs).getData();
    result = cmp(a, b);
    break;
  }
  case DataType::BOOL: {
    auto a = static_cast<const BoolConstAttribute&>(lhs).getData();
    auto b = static_cast<const BoolConstAttribute&>(rhs).getData();
    result = cmp(a, b);
    break;
  }
  default:
    failed = true;
    break;
  }

  if (failed) {
    return nullptr;
  }

  return makeConstAttribute(result);
}

// === Generic CRTP bases =====================================================

template <typename ConcretePattern, typename ArithBinaryOp>
class ArithBinaryOpConstFoldPattern : public Pattern {
public:
  bool matchAndRewrite(Operation& op,
                       PatternRewriter& rewriter) const override {
    auto* arithOp = dynamic_cast<ArithBinaryOp*>(&op);
    if (!arithOp) {
      return false;
    }

    const auto& inX = arithOp->getInputX();
    const auto& inY = arithOp->getInputY();

    auto* defX = inX.getDefiningOp();
    auto* defY = inY.getDefiningOp();
    if (!defX || !defY) {
      return false;
    }

    auto* cX = dynamic_cast<const ConstantOp*>(defX);
    auto* cY = dynamic_cast<const ConstantOp*>(defY);
    if (!cX || !cY) {
      return false;
    }

    auto newAttr = static_cast<const ConcretePattern*>(this)->fold(
      *arithOp, cX->getAttr(), cY->getAttr());
    if (!newAttr) {
      return false;
    }

    rewriter.replaceOpWith(op,
                           std::make_unique<ConstantOp>(std::move(newAttr)));
    return true;
  }
};

template <typename ConcretePattern, typename ArithUnaryOp>
class ArithUnaryOpConstFoldPattern : public Pattern {
public:
  bool matchAndRewrite(Operation& op,
                       PatternRewriter& rewriter) const override {
    auto* arithOp = dynamic_cast<ArithUnaryOp*>(&op);
    if (!arithOp) {
      return false;
    }

    const auto& in = arithOp->getInput();
    auto* def = in.getDefiningOp();
    if (!def) {
      return false;
    }

    auto* c = dynamic_cast<const ConstantOp*>(def);
    if (!c) {
      return false;
    }

    auto newAttr =
      static_cast<const ConcretePattern*>(this)->fold(*arithOp, c->getAttr());
    if (!newAttr) {
      return false;
    }

    rewriter.replaceOpWith(op,
                           std::make_unique<ConstantOp>(std::move(newAttr)));
    return true;
  }
};

// === Concrete constant-folding patterns =====================================

// add(const, const)
class AddOpConstFoldPattern final
  : public ArithBinaryOpConstFoldPattern<AddOpConstFoldPattern,
                                         iris::arith::AddOp> {
public:
  std::unique_ptr<ConstAttribute> fold(const iris::arith::AddOp& op,
                                       const ConstAttribute& lhs,
                                       const ConstAttribute& rhs) const {
    auto dataType = op.getInputX().getDataType();
    if (!isInteger(dataType) && !isFloatingPoint(dataType)) {
      return nullptr;
    }

    return foldNumericBinary(dataType, lhs, rhs,
                             [](auto a, auto b) { return a + b; });
  }
};

// sub(const, const)
class SubOpConstFoldPattern final
  : public ArithBinaryOpConstFoldPattern<SubOpConstFoldPattern,
                                         iris::arith::SubOp> {
public:
  std::unique_ptr<ConstAttribute> fold(const iris::arith::SubOp& op,
                                       const ConstAttribute& lhs,
                                       const ConstAttribute& rhs) const {
    auto dataType = op.getInputX().getDataType();
    if (!isInteger(dataType) && !isFloatingPoint(dataType)) {
      return nullptr;
    }

    return foldNumericBinary(dataType, lhs, rhs,
                             [](auto a, auto b) { return a - b; });
  }
};

// mul(const, const)
class MulOpConstFoldPattern final
  : public ArithBinaryOpConstFoldPattern<MulOpConstFoldPattern,
                                         iris::arith::MulOp> {
public:
  std::unique_ptr<ConstAttribute> fold(const iris::arith::MulOp& op,
                                       const ConstAttribute& lhs,
                                       const ConstAttribute& rhs) const {
    auto dataType = op.getInputX().getDataType();
    if (!isInteger(dataType) && !isFloatingPoint(dataType)) {
      return nullptr;
    }

    return foldNumericBinary(dataType, lhs, rhs,
                             [](auto a, auto b) { return a * b; });
  }
};

// div(const, const)
class DivOpConstFoldPattern final
  : public ArithBinaryOpConstFoldPattern<DivOpConstFoldPattern,
                                         iris::arith::DivOp> {
public:
  std::unique_ptr<ConstAttribute> fold(const iris::arith::DivOp& op,
                                       const ConstAttribute& lhs,
                                       const ConstAttribute& rhs) const {
    auto dataType = op.getInputX().getDataType();

    if (isInteger(dataType)) {
      switch (dataType) {
      case DataType::SINT: {
        auto a = static_cast<const SIntegerConstAttribute&>(lhs).getData();
        auto b = static_cast<const SIntegerConstAttribute&>(rhs).getData();
        if (b == 0) {
          return nullptr; // do not fold division by zero
        }
        return makeConstAttribute(
          static_cast<SIntegerConstAttribute::StorageType>(a / b));
      }
      case DataType::UINT: {
        auto a = static_cast<const UIntegerConstAttribute&>(lhs).getData();
        auto b = static_cast<const UIntegerConstAttribute&>(rhs).getData();
        if (b == 0) {
          return nullptr;
        }
        return makeConstAttribute(
          static_cast<UIntegerConstAttribute::StorageType>(a / b));
      }
      default:
        break;
      }
      return nullptr;
    }

    if (isFloatingPoint(dataType)) {
      // For floating point, rely on IEEE semantics (Inf/NaN)
      return foldNumericBinary(dataType, lhs, rhs,
                               [](auto a, auto b) { return a / b; });
    }

    return nullptr;
  }
};

// and(const, const)
class AndOpConstFoldPattern final
  : public ArithBinaryOpConstFoldPattern<AndOpConstFoldPattern,
                                         iris::arith::AndOp> {
public:
  std::unique_ptr<ConstAttribute> fold(const iris::arith::AndOp& op,
                                       const ConstAttribute& lhs,
                                       const ConstAttribute& rhs) const {
    auto dataType = op.getInputX().getDataType();
    if (!isInteger(dataType)) {
      return nullptr;
    }

    return foldIntegerBinary(dataType, lhs, rhs,
                             [](auto a, auto b) { return a & b; });
  }
};

// or(const, const)
class OrOpConstFoldPattern final
  : public ArithBinaryOpConstFoldPattern<OrOpConstFoldPattern,
                                         iris::arith::OrOp> {
public:
  std::unique_ptr<ConstAttribute> fold(const iris::arith::OrOp& op,
                                       const ConstAttribute& lhs,
                                       const ConstAttribute& rhs) const {
    auto dataType = op.getInputX().getDataType();
    if (!isInteger(dataType)) {
      return nullptr;
    }

    return foldIntegerBinary(dataType, lhs, rhs,
                             [](auto a, auto b) { return a | b; });
  }
};

// xor(const, const)
class XorOpConstFoldPattern final
  : public ArithBinaryOpConstFoldPattern<XorOpConstFoldPattern,
                                         iris::arith::XorOp> {
public:
  std::unique_ptr<ConstAttribute> fold(const iris::arith::XorOp& op,
                                       const ConstAttribute& lhs,
                                       const ConstAttribute& rhs) const {
    auto dataType = op.getInputX().getDataType();
    if (!isInteger(dataType)) {
      return nullptr;
    }

    return foldIntegerBinary(dataType, lhs, rhs,
                             [](auto a, auto b) { return a ^ b; });
  }
};

// sal(const, const)
class SalOpConstFoldPattern final
  : public ArithBinaryOpConstFoldPattern<SalOpConstFoldPattern,
                                         iris::arith::SalOp> {
public:
  std::unique_ptr<ConstAttribute> fold(const iris::arith::SalOp& op,
                                       const ConstAttribute& lhs,
                                       const ConstAttribute& rhs) const {
    auto dataType = op.getInputX().getDataType();
    if (dataType != DataType::SINT) {
      return nullptr;
    }

    auto value = static_cast<const SIntegerConstAttribute&>(lhs).getData();
    auto shift = static_cast<const SIntegerConstAttribute&>(rhs).getData();

    int bitWidth = sizeof(SIntegerConstAttribute::StorageType) * CHAR_BIT;
    if (shift < 0 || shift >= bitWidth) {
      return nullptr;
    }

    SIntegerConstAttribute::StorageType result = value << shift;
    return makeConstAttribute(result);
  }
};

// sar(const, const)
class SarOpConstFoldPattern final
  : public ArithBinaryOpConstFoldPattern<SarOpConstFoldPattern,
                                         iris::arith::SarOp> {
public:
  std::unique_ptr<ConstAttribute> fold(const iris::arith::SarOp& op,
                                       const ConstAttribute& lhs,
                                       const ConstAttribute& rhs) const {
    auto dataType = op.getInputX().getDataType();
    if (dataType != DataType::SINT) {
      return nullptr;
    }

    auto value = static_cast<const SIntegerConstAttribute&>(lhs).getData();
    auto shift = static_cast<const SIntegerConstAttribute&>(rhs).getData();

    int bitWidth = sizeof(SIntegerConstAttribute::StorageType) * CHAR_BIT;
    if (shift < 0 || shift >= bitWidth) {
      return nullptr;
    }

    SIntegerConstAttribute::StorageType result = value >> shift;
    return makeConstAttribute(result);
  }
};

// shl(const, const)
class ShlOpConstFoldPattern final
  : public ArithBinaryOpConstFoldPattern<ShlOpConstFoldPattern,
                                         iris::arith::ShlOp> {
public:
  std::unique_ptr<ConstAttribute> fold(const iris::arith::ShlOp& op,
                                       const ConstAttribute& lhs,
                                       const ConstAttribute& rhs) const {
    auto dataType = op.getInputX().getDataType();
    if (dataType != DataType::UINT) {
      return nullptr;
    }

    auto value = static_cast<const UIntegerConstAttribute&>(lhs).getData();
    auto shift = static_cast<const UIntegerConstAttribute&>(rhs).getData();

    auto bitWidth = sizeof(UIntegerConstAttribute::StorageType) * CHAR_BIT;
    if (shift >= bitWidth) {
      return nullptr;
    }

    UIntegerConstAttribute::StorageType result = value << shift;
    return makeConstAttribute(result);
  }
};

// shr(const, const)
class ShrOpConstFoldPattern final
  : public ArithBinaryOpConstFoldPattern<ShrOpConstFoldPattern,
                                         iris::arith::ShrOp> {
public:
  std::unique_ptr<ConstAttribute> fold(const iris::arith::ShrOp& op,
                                       const ConstAttribute& lhs,
                                       const ConstAttribute& rhs) const {
    auto dataType = op.getInputX().getDataType();
    if (dataType != DataType::UINT) {
      return nullptr;
    }

    auto value = static_cast<const UIntegerConstAttribute&>(lhs).getData();
    auto shift = static_cast<const UIntegerConstAttribute&>(rhs).getData();

    auto bitWidth = sizeof(UIntegerConstAttribute::StorageType) * CHAR_BIT;
    if (shift >= bitWidth) {
      return nullptr;
    }

    UIntegerConstAttribute::StorageType result = value >> shift;
    return makeConstAttribute(result);
  }
};

// not(const)
class NotOpConstFoldPattern final
  : public ArithUnaryOpConstFoldPattern<NotOpConstFoldPattern,
                                        iris::arith::NotOp> {
public:
  std::unique_ptr<ConstAttribute> fold(const iris::arith::NotOp& op,
                                       const ConstAttribute& operand) const {
    auto dataType = op.getInput().getDataType();
    if (!isInteger(dataType)) {
      return nullptr;
    }

    switch (dataType) {
    case DataType::SINT: {
      auto v = static_cast<const SIntegerConstAttribute&>(operand).getData();
      SIntegerConstAttribute::StorageType result = ~v;
      return makeConstAttribute(result);
    }
    case DataType::UINT: {
      auto v = static_cast<const UIntegerConstAttribute&>(operand).getData();
      UIntegerConstAttribute::StorageType result = ~v;
      return makeConstAttribute(result);
    }
    default:
      return nullptr;
    }
  }
};

// cmp(const, const)
class CompareOpConstFoldPattern final
  : public ArithBinaryOpConstFoldPattern<CompareOpConstFoldPattern,
                                         iris::arith::CompareOp> {
public:
  std::unique_ptr<ConstAttribute> fold(const iris::arith::CompareOp& op,
                                       const ConstAttribute& lhs,
                                       const ConstAttribute& rhs) const {
    auto operandDT = op.getInputX().getDataType();
    auto pred = op.getPred();

    auto cmp = [pred](auto a, auto b) -> bool {
      using Pred = iris::arith::CompareOp::Pred;
      switch (pred) {
      case Pred::EQ:
        return a == b;
      case Pred::NEQ:
        return a != b;
      case Pred::A:
        return a > b;
      case Pred::B:
        return a < b;
      case Pred::AE:
        return a >= b;
      case Pred::BE:
        return a <= b;
      default:
        return false;
      }
    };

    return foldComparison(operandDT, lhs, rhs, cmp);
  }
};

} // namespace

ArithConstFoldPass::ArithConstFoldPass() {
  addPattern<AddOpConstFoldPattern>();
  addPattern<SubOpConstFoldPattern>();
  addPattern<MulOpConstFoldPattern>();
  addPattern<DivOpConstFoldPattern>();

  addPattern<AndOpConstFoldPattern>();
  addPattern<OrOpConstFoldPattern>();
  addPattern<XorOpConstFoldPattern>();

  addPattern<SalOpConstFoldPattern>();
  addPattern<SarOpConstFoldPattern>();
  addPattern<ShlOpConstFoldPattern>();
  addPattern<ShrOpConstFoldPattern>();

  addPattern<NotOpConstFoldPattern>();
  addPattern<CompareOpConstFoldPattern>();
}

} // namespace arith
} // namespace opt
} // namespace iris
