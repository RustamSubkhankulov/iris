#include <limits>
#include <sstream>

#include <opt/lib/arith/peephole.hpp>

#include <attributes.hpp>
#include <ops/dialects/arith/ops.hpp>

namespace iris {
namespace opt {
namespace arith {

namespace {

using iris::ConstAttribute;
using iris::DataType;
using iris::FloatConstAttribute;
using iris::SIntegerConstAttribute;
using iris::UIntegerConstAttribute;

using iris::arith::ConstantOp;

// === Helpers ===============================================================

inline ConstantOp* getConstFromInput(Input& input) {
  auto* def = input.getDefiningOp();
  return dynamic_cast<ConstantOp*>(def);
}

inline bool isZeroAttribute(const ConstAttribute& attr) {
  switch (attr.getDataType()) {
  case DataType::FLOAT:
    return (static_cast<const FloatConstAttribute&>(attr).getData() == 0.0);
  case DataType::SINT:
    return (static_cast<const SIntegerConstAttribute&>(attr).getData() == 0);
  case DataType::UINT:
    return (static_cast<const UIntegerConstAttribute&>(attr).getData() == 0);
  default:
    return false;
  }
}

inline bool isOneAttribute(const ConstAttribute& attr) {
  switch (attr.getDataType()) {
  case DataType::FLOAT:
    return (static_cast<const FloatConstAttribute&>(attr).getData() == 1.0);
  case DataType::SINT:
    return (static_cast<const SIntegerConstAttribute&>(attr).getData() == 1);
  case DataType::UINT:
    return (static_cast<const UIntegerConstAttribute&>(attr).getData() == 1);
  default:
    return false;
  }
}

inline bool isAllOnesBitwiseAttribute(const ConstAttribute& attr) {
  switch (attr.getDataType()) {
  case DataType::SINT: {
    auto v = static_cast<const SIntegerConstAttribute&>(attr).getData();
    return (v == static_cast<SIntegerConstAttribute::StorageType>(-1));
  }
  case DataType::UINT: {
    auto v = static_cast<const UIntegerConstAttribute&>(attr).getData();
    return (v ==
            std::numeric_limits<UIntegerConstAttribute::StorageType>::max());
  }
  default:
    return false;
  }
}

inline std::unique_ptr<ConstAttribute> makeZeroConstAttribute(DataType dt) {
  switch (dt) {
  case DataType::FLOAT:
    return makeConstAttribute(0.0);
  case DataType::SINT:
    return makeConstAttribute(
      static_cast<SIntegerConstAttribute::StorageType>(0));
  case DataType::UINT:
    return makeConstAttribute(
      static_cast<UIntegerConstAttribute::StorageType>(0));
  case DataType::BOOL:
    return makeConstAttribute(false);
  default:
    break;
  }

  std::stringstream msg;
  msg << "Unexpected DataType: " << dt;
  throw IrisException(msg.str());
}

// === Generic CRTP base ======================================================

template <typename ConcretePattern, typename ArithBinaryOp>
class ArithBinaryPeepholePattern : public Pattern {
public:
  bool matchAndRewrite(Operation& op,
                       PatternRewriter& rewriter) const override {
    auto* arithOp = dynamic_cast<ArithBinaryOp*>(&op);
    if (!arithOp) {
      return false;
    }
    return static_cast<const ConcretePattern*>(this)->rewrite(*arithOp,
                                                              rewriter);
  }
};

// === Concrete peephole patterns =============================================

// --- add patterns -----------------------------------------------------------

// add(x, 0) -> x
class AddZeroRhsPattern final
  : public ArithBinaryPeepholePattern<AddZeroRhsPattern, iris::arith::AddOp> {
public:
  bool rewrite(iris::arith::AddOp& op, PatternRewriter& rewriter) const {
    auto* cR = getConstFromInput(op.getInputY());
    if (!cR || !isZeroAttribute(cR->getAttr())) {
      return false;
    }

    auto* xDef = op.getInputX().getDefiningOp();
    if (!xDef) {
      return false;
    }

    op.replaceAllUsesWith(*xDef);
    rewriter.eraseOp(op);
    return true;
  }
};

// add(0, x) -> x
class AddZeroLhsPattern final
  : public ArithBinaryPeepholePattern<AddZeroLhsPattern, iris::arith::AddOp> {
public:
  bool rewrite(iris::arith::AddOp& op, PatternRewriter& rewriter) const {
    auto* cL = getConstFromInput(op.getInputX());
    if (!cL || !isZeroAttribute(cL->getAttr())) {
      return false;
    }

    auto* yDef = op.getInputY().getDefiningOp();
    if (!yDef) {
      return false;
    }

    op.replaceAllUsesWith(*yDef);
    rewriter.eraseOp(op);
    return true;
  }
};

// (x + C1) + C2  ->  x + (C1 + C2)
// (C1 + x) + C2  ->  x + (C1 + C2)
// C2 + (x + C1)  ->  x + (C1 + C2)
// C2 + (C1 + x)  ->  x + (C1 + C2)
class AddRotateConstantsPattern final
  : public ArithBinaryPeepholePattern<AddRotateConstantsPattern,
                                      iris::arith::AddOp> {
public:
  bool rewrite(iris::arith::AddOp& op,
               [[maybe_unused]] PatternRewriter& /*rewriter*/) const {
    // Only for integer types (avoid FP reassociation issues).
    if (!isInteger(op.getDataType())) {
      return false;
    }

    return tryRotate(op, /*innerOnLHS=*/true) ||
           tryRotate(op, /*innerOnLHS=*/false);
  }

private:
  static bool tryRotate(iris::arith::AddOp& outer, bool innerOnLHS) {
    Input& innerInput = innerOnLHS ? outer.getInputX() : outer.getInputY();
    Input& otherInput = innerOnLHS ? outer.getInputY() : outer.getInputX();

    auto* innerDef = innerInput.getDefiningOp();
    auto* innerAdd = dynamic_cast<iris::arith::AddOp*>(innerDef);
    if (!innerAdd) {
      return false;
    }

    auto* outerConst = getConstFromInput(otherInput);
    if (!outerConst) {
      return false;
    }

    if (innerAdd->getUsersNum() != 1) {
      return false;
    }

    const auto& users = innerAdd->getUsers();
    const auto& soleUser = users.front();
    if (soleUser.getUserOp() != &outer) {
      return false;
    }

    Input& inX = innerAdd->getInputX();
    Input& inY = innerAdd->getInputY();

    auto* innerConstX = getConstFromInput(inX);
    auto* innerConstY = getConstFromInput(inY);

    // Either both are const or both are non-const
    if (!!innerConstX == !!innerConstY) {
      return false;
    }

    auto* innerConst = innerConstX ? innerConstX : innerConstY;
    Operation* varOp = innerConstX ? inY.getDefiningOp() : inX.getDefiningOp();
    if (!varOp) {
      return false;
    }

    // Perform rotation:
    //   innerAdd := add(innerConst, outerConst)
    //   outer    := add(varOp, innerAdd)
    innerAdd->setInput(0, innerConst);
    innerAdd->setInput(1, outerConst);

    outer.setInput(0, varOp);
    outer.setInput(1, innerAdd);

    return true;
  }
};

// --- sub patterns -----------------------------------------------------------

// sub(x, 0) -> x
class SubZeroRhsPattern final
  : public ArithBinaryPeepholePattern<SubZeroRhsPattern, iris::arith::SubOp> {
public:
  bool rewrite(iris::arith::SubOp& op, PatternRewriter& rewriter) const {
    auto* cR = getConstFromInput(op.getInputY());
    if (!cR || !isZeroAttribute(cR->getAttr())) {
      return false;
    }

    auto* xDef = op.getInputX().getDefiningOp();
    if (!xDef) {
      return false;
    }

    op.replaceAllUsesWith(*xDef);
    rewriter.eraseOp(op);
    return true;
  }
};

// sub(x, x) -> 0
class SubSelfToZeroPattern final
  : public ArithBinaryPeepholePattern<SubSelfToZeroPattern,
                                      iris::arith::SubOp> {
public:
  bool rewrite(iris::arith::SubOp& op, PatternRewriter& rewriter) const {
    auto* lhs = op.getInputX().getDefiningOp();
    auto* rhs = op.getInputY().getDefiningOp();
    if (!lhs || lhs != rhs) {
      return false;
    }

    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    auto zeroAttr = makeZeroConstAttribute(dt);
    auto newConst =
      std::make_unique<iris::arith::ConstantOp>(std::move(zeroAttr));

    rewriter.replaceOpWith(op, std::move(newConst));
    return true;
  }
};

// --- mul patterns -----------------------------------------------------------

// mul(x, 1) -> x
class MulOneRhsPattern final
  : public ArithBinaryPeepholePattern<MulOneRhsPattern, iris::arith::MulOp> {
public:
  bool rewrite(iris::arith::MulOp& op, PatternRewriter& rewriter) const {
    auto* cR = getConstFromInput(op.getInputY());
    if (!cR || !isOneAttribute(cR->getAttr())) {
      return false;
    }

    auto* xDef = op.getInputX().getDefiningOp();
    if (!xDef) {
      return false;
    }

    op.replaceAllUsesWith(*xDef);
    rewriter.eraseOp(op);
    return true;
  }
};

// mul(1, x) -> x
class MulOneLhsPattern final
  : public ArithBinaryPeepholePattern<MulOneLhsPattern, iris::arith::MulOp> {
public:
  bool rewrite(iris::arith::MulOp& op, PatternRewriter& rewriter) const {
    auto* cL = getConstFromInput(op.getInputX());
    if (!cL || !isOneAttribute(cL->getAttr())) {
      return false;
    }

    auto* yDef = op.getInputY().getDefiningOp();
    if (!yDef) {
      return false;
    }

    op.replaceAllUsesWith(*yDef);
    rewriter.eraseOp(op);
    return true;
  }
};

// mul(x, 0) -> 0
class MulZeroRhsPattern final
  : public ArithBinaryPeepholePattern<MulZeroRhsPattern, iris::arith::MulOp> {
public:
  bool rewrite(iris::arith::MulOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    auto* cR = getConstFromInput(op.getInputY());
    if (!cR || !isZeroAttribute(cR->getAttr())) {
      return false;
    }

    auto zeroAttr = makeZeroConstAttribute(dt);
    auto newConst =
      std::make_unique<iris::arith::ConstantOp>(std::move(zeroAttr));
    rewriter.replaceOpWith(op, std::move(newConst));
    return true;
  }
};

// mul(0, x) -> 0
class MulZeroLhsPattern final
  : public ArithBinaryPeepholePattern<MulZeroLhsPattern, iris::arith::MulOp> {
public:
  bool rewrite(iris::arith::MulOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    auto* cL = getConstFromInput(op.getInputX());
    if (!cL || !isZeroAttribute(cL->getAttr())) {
      return false;
    }

    auto zeroAttr = makeZeroConstAttribute(dt);
    auto newConst =
      std::make_unique<iris::arith::ConstantOp>(std::move(zeroAttr));
    rewriter.replaceOpWith(op, std::move(newConst));
    return true;
  }
};

// (C1 * x) * C2  ->  x * (C1 * C2)
// (x * C1) * C2  ->  x * (C1 * C2)
// C2 * (C1 * x)  ->  x * (C1 * C2)
// C2 * (x * C1)  ->  x * (C1 * C2)
class MulRotateConstantsPattern final
  : public ArithBinaryPeepholePattern<MulRotateConstantsPattern,
                                      iris::arith::MulOp> {
public:
  bool rewrite(iris::arith::MulOp& op,
               [[maybe_unused]] PatternRewriter& /*rewriter*/) const {
    // Only for integer types (avoid FP reassociation issues).
    if (!isInteger(op.getDataType())) {
      return false;
    }

    return tryRotate(op, /*innerOnLHS=*/true) ||
           tryRotate(op, /*innerOnLHS=*/false);
  }

private:
  static bool tryRotate(iris::arith::MulOp& outer, bool innerOnLHS) {
    Input& innerInput = innerOnLHS ? outer.getInputX() : outer.getInputY();
    Input& otherInput = innerOnLHS ? outer.getInputY() : outer.getInputX();

    auto* innerDef = innerInput.getDefiningOp();
    auto* innerMul = dynamic_cast<iris::arith::MulOp*>(innerDef);
    if (!innerMul) {
      return false;
    }

    auto* outerConst = getConstFromInput(otherInput);
    if (!outerConst) {
      return false;
    }

    if (innerMul->getUsersNum() != 1) {
      return false;
    }

    const auto& users = innerMul->getUsers();
    const auto& soleUser = users.front();
    if (soleUser.getUserOp() != &outer) {
      return false;
    }

    Input& inX = innerMul->getInputX();
    Input& inY = innerMul->getInputY();

    auto* innerConstX = getConstFromInput(inX);
    auto* innerConstY = getConstFromInput(inY);

    // Either both are const or both are non-const
    if (!!innerConstX == !!innerConstY) {
      return false;
    }

    auto* innerConst = innerConstX ? innerConstX : innerConstY;
    Operation* varOp = innerConstX ? inY.getDefiningOp() : inX.getDefiningOp();
    if (!varOp) {
      return false;
    }

    // Perform rotation:
    //   innerMul := mul(innerConst, outerConst)
    //   outer    := mul(varOp, innerMul)
    innerMul->setInput(0, Input(innerConst));
    innerMul->setInput(1, Input(outerConst));

    outer.setInput(0, Input(varOp));
    outer.setInput(1, Input(innerMul));

    return true;
  }
};

// --- div patterns -----------------------------------------------------------

// div(x, 1) -> x
class DivOneRhsPattern final
  : public ArithBinaryPeepholePattern<DivOneRhsPattern, iris::arith::DivOp> {
public:
  bool rewrite(iris::arith::DivOp& op, PatternRewriter& rewriter) const {
    auto* cR = getConstFromInput(op.getInputY());
    if (!cR || !isOneAttribute(cR->getAttr())) {
      return false;
    }

    auto* xDef = op.getInputX().getDefiningOp();
    if (!xDef) {
      return false;
    }

    op.replaceAllUsesWith(*xDef);
    rewriter.eraseOp(op);
    return true;
  }
};

// --- and patterns -----------------------------------------------------------

// and(x, 0) -> 0
class AndZeroRhsPattern final
  : public ArithBinaryPeepholePattern<AndZeroRhsPattern, iris::arith::AndOp> {
public:
  bool rewrite(iris::arith::AndOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    auto* cR = getConstFromInput(op.getInputY());
    if (!cR || !isZeroAttribute(cR->getAttr())) {
      return false;
    }

    auto zeroAttr = makeZeroConstAttribute(dt);
    auto newConst =
      std::make_unique<iris::arith::ConstantOp>(std::move(zeroAttr));
    rewriter.replaceOpWith(op, std::move(newConst));
    return true;
  }
};

// and(0, x) -> 0
class AndZeroLhsPattern final
  : public ArithBinaryPeepholePattern<AndZeroLhsPattern, iris::arith::AndOp> {
public:
  bool rewrite(iris::arith::AndOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    auto* cL = getConstFromInput(op.getInputX());
    if (!cL || !isZeroAttribute(cL->getAttr())) {
      return false;
    }

    auto zeroAttr = makeZeroConstAttribute(dt);
    auto newConst =
      std::make_unique<iris::arith::ConstantOp>(std::move(zeroAttr));
    rewriter.replaceOpWith(op, std::move(newConst));
    return true;
  }
};

// and(x, 1..1) -> x
class AndAllOnesRhsPattern final
  : public ArithBinaryPeepholePattern<AndAllOnesRhsPattern,
                                      iris::arith::AndOp> {
public:
  bool rewrite(iris::arith::AndOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    auto* cR = getConstFromInput(op.getInputY());
    if (!cR || !isAllOnesBitwiseAttribute(cR->getAttr())) {
      return false;
    }

    auto* xDef = op.getInputX().getDefiningOp();
    if (!xDef) {
      return false;
    }

    op.replaceAllUsesWith(*xDef);
    rewriter.eraseOp(op);
    return true;
  }
};

// and(1..1, x) -> x
class AndAllOnesLhsPattern final
  : public ArithBinaryPeepholePattern<AndAllOnesLhsPattern,
                                      iris::arith::AndOp> {
public:
  bool rewrite(iris::arith::AndOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    auto* cL = getConstFromInput(op.getInputX());
    if (!cL || !isAllOnesBitwiseAttribute(cL->getAttr())) {
      return false;
    }

    auto* yDef = op.getInputY().getDefiningOp();
    if (!yDef) {
      return false;
    }

    op.replaceAllUsesWith(*yDef);
    rewriter.eraseOp(op);
    return true;
  }
};

// and(x, x) -> x
class AndSelfPattern final
  : public ArithBinaryPeepholePattern<AndSelfPattern, iris::arith::AndOp> {
public:
  bool rewrite(iris::arith::AndOp& op, PatternRewriter& rewriter) const {
    auto* lhs = op.getInputX().getDefiningOp();
    auto* rhs = op.getInputY().getDefiningOp();
    if (!lhs || lhs != rhs) {
      return false;
    }

    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    op.replaceAllUsesWith(*lhs);
    rewriter.eraseOp(op);
    return true;
  }
};

// --- or patterns ------------------------------------------------------------

// or(x, 0) -> x
class OrZeroRhsPattern final
  : public ArithBinaryPeepholePattern<OrZeroRhsPattern, iris::arith::OrOp> {
public:
  bool rewrite(iris::arith::OrOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    auto* cR = getConstFromInput(op.getInputY());
    if (!cR || !isZeroAttribute(cR->getAttr())) {
      return false;
    }

    auto* xDef = op.getInputX().getDefiningOp();
    if (!xDef) {
      return false;
    }

    op.replaceAllUsesWith(*xDef);
    rewriter.eraseOp(op);
    return true;
  }
};

class OrZeroLhsPattern final
  : public ArithBinaryPeepholePattern<OrZeroLhsPattern, iris::arith::OrOp> {
public:
  bool rewrite(iris::arith::OrOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    auto* cL = getConstFromInput(op.getInputX());
    if (!cL || !isZeroAttribute(cL->getAttr())) {
      return false;
    }

    auto* yDef = op.getInputY().getDefiningOp();
    if (!yDef) {
      return false;
    }

    op.replaceAllUsesWith(*yDef);
    rewriter.eraseOp(op);
    return true;
  }
};

// or(x, 1..1) -> 1..1
class OrAllOnesRhsPattern final
  : public ArithBinaryPeepholePattern<OrAllOnesRhsPattern, iris::arith::OrOp> {
public:
  bool rewrite(iris::arith::OrOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    auto* cR = getConstFromInput(op.getInputY());
    if (!cR || !isAllOnesBitwiseAttribute(cR->getAttr())) {
      return false;
    }

    op.replaceAllUsesWith(*cR);
    rewriter.eraseOp(op);
    return true;
  }
};

// or(1..1, x) -> 1..1
class OrAllOnesLhsPattern final
  : public ArithBinaryPeepholePattern<OrAllOnesLhsPattern, iris::arith::OrOp> {
public:
  bool rewrite(iris::arith::OrOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    auto* cL = getConstFromInput(op.getInputX());
    if (!cL || !isAllOnesBitwiseAttribute(cL->getAttr())) {
      return false;
    }

    op.replaceAllUsesWith(*cL);
    rewriter.eraseOp(op);
    return true;
  }
};

// or(x, x) -> x
class OrSelfPattern final
  : public ArithBinaryPeepholePattern<OrSelfPattern, iris::arith::OrOp> {
public:
  bool rewrite(iris::arith::OrOp& op, PatternRewriter& rewriter) const {
    auto* lhs = op.getInputX().getDefiningOp();
    auto* rhs = op.getInputY().getDefiningOp();
    if (!lhs || lhs != rhs) {
      return false;
    }

    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    op.replaceAllUsesWith(*lhs);
    rewriter.eraseOp(op);
    return true;
  }
};

// --- xor patterns -----------------------------------------------------------

// xor(x, 0) -> x
class XorZeroRhsPattern final
  : public ArithBinaryPeepholePattern<XorZeroRhsPattern, iris::arith::XorOp> {
public:
  bool rewrite(iris::arith::XorOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    auto* cR = getConstFromInput(op.getInputY());
    if (!cR || !isZeroAttribute(cR->getAttr())) {
      return false;
    }

    auto* xDef = op.getInputX().getDefiningOp();
    if (!xDef) {
      return false;
    }

    op.replaceAllUsesWith(*xDef);
    rewriter.eraseOp(op);
    return true;
  }
};

// xor(0, x) -> x
class XorZeroLhsPattern final
  : public ArithBinaryPeepholePattern<XorZeroLhsPattern, iris::arith::XorOp> {
public:
  bool rewrite(iris::arith::XorOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    auto* cL = getConstFromInput(op.getInputX());
    if (!cL || !isZeroAttribute(cL->getAttr())) {
      return false;
    }

    auto* yDef = op.getInputY().getDefiningOp();
    if (!yDef) {
      return false;
    }

    op.replaceAllUsesWith(*yDef);
    rewriter.eraseOp(op);
    return true;
  }
};

// xor(x, x) -> 0
class XorSelfToZeroPattern final
  : public ArithBinaryPeepholePattern<XorSelfToZeroPattern,
                                      iris::arith::XorOp> {
public:
  bool rewrite(iris::arith::XorOp& op, PatternRewriter& rewriter) const {
    auto* lhs = op.getInputX().getDefiningOp();
    auto* rhs = op.getInputY().getDefiningOp();
    if (!lhs || lhs != rhs) {
      return false;
    }

    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    auto zeroAttr = makeZeroConstAttribute(dt);
    auto newConst =
      std::make_unique<iris::arith::ConstantOp>(std::move(zeroAttr));
    rewriter.replaceOpWith(op, std::move(newConst));
    return true;
  }
};

// xor(x, 1..1) -> not(x)
class XorAllOnesRhsToNotPattern final
  : public ArithBinaryPeepholePattern<XorAllOnesRhsToNotPattern,
                                      iris::arith::XorOp> {
public:
  bool rewrite(iris::arith::XorOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    auto* cR = getConstFromInput(op.getInputY());
    if (!cR || !isAllOnesBitwiseAttribute(cR->getAttr())) {
      return false;
    }

    auto xInput = op.getInputX();
    rewriter.replaceOpWithNew<iris::arith::NotOp>(op, xInput);
    return true;
  }
};

// xor(1..1, x) -> not(x)
class XorAllOnesLhsToNotPattern final
  : public ArithBinaryPeepholePattern<XorAllOnesLhsToNotPattern,
                                      iris::arith::XorOp> {
public:
  bool rewrite(iris::arith::XorOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isInteger(dt)) {
      return false;
    }

    auto* cL = getConstFromInput(op.getInputX());
    if (!cL || !isAllOnesBitwiseAttribute(cL->getAttr())) {
      return false;
    }

    auto yInput = op.getInputY();
    rewriter.replaceOpWithNew<iris::arith::NotOp>(op, yInput);
    return true;
  }
};

// --- sal patterns -----------------------------------------------------------

// sal(x, 0) -> x
class SalZeroShiftPattern final
  : public ArithBinaryPeepholePattern<SalZeroShiftPattern, iris::arith::SalOp> {
public:
  bool rewrite(iris::arith::SalOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isSInteger(dt)) {
      return false;
    }

    auto* cR = getConstFromInput(op.getInputY());
    if (!cR || !isZeroAttribute(cR->getAttr())) {
      return false;
    }

    auto* xDef = op.getInputX().getDefiningOp();
    if (!xDef) {
      return false;
    }

    op.replaceAllUsesWith(*xDef);
    rewriter.eraseOp(op);
    return true;
  }
};

// sal(0, x) -> 0
class SalZeroArgPattern final
  : public ArithBinaryPeepholePattern<SalZeroArgPattern, iris::arith::SalOp> {
public:
  bool rewrite(iris::arith::SalOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isSInteger(dt)) {
      return false;
    }

    auto* cL = getConstFromInput(op.getInputX());
    if (!cL || !isZeroAttribute(cL->getAttr())) {
      return false;
    }

    auto zeroAttr = makeZeroConstAttribute(dt);
    auto newConst =
      std::make_unique<iris::arith::ConstantOp>(std::move(zeroAttr));
    rewriter.replaceOpWith(op, std::move(newConst));
    return true;
  }
};

// --- shl patterns -----------------------------------------------------------

// shl(x, 0) -> x
class ShlZeroShiftPattern final
  : public ArithBinaryPeepholePattern<ShlZeroShiftPattern, iris::arith::ShlOp> {
public:
  bool rewrite(iris::arith::ShlOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isUInteger(dt)) {
      return false;
    }

    auto* cR = getConstFromInput(op.getInputY());
    if (!cR || !isZeroAttribute(cR->getAttr())) {
      return false;
    }

    auto* xDef = op.getInputX().getDefiningOp();
    if (!xDef) {
      return false;
    }

    op.replaceAllUsesWith(*xDef);
    rewriter.eraseOp(op);
    return true;
  }
};

// shl(0, x) -> 0
class ShlZeroArgPattern final
  : public ArithBinaryPeepholePattern<ShlZeroArgPattern, iris::arith::ShlOp> {
public:
  bool rewrite(iris::arith::ShlOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isUInteger(dt)) {
      return false;
    }

    auto* cL = getConstFromInput(op.getInputX());
    if (!cL || !isZeroAttribute(cL->getAttr())) {
      return false;
    }

    auto zeroAttr = makeZeroConstAttribute(dt);
    auto newConst =
      std::make_unique<iris::arith::ConstantOp>(std::move(zeroAttr));
    rewriter.replaceOpWith(op, std::move(newConst));
    return true;
  }
};

// --- sar patterns -----------------------------------------------------------

// sar(x, 0) -> x
class SarZeroShiftPattern final
  : public ArithBinaryPeepholePattern<SarZeroShiftPattern, iris::arith::SarOp> {
public:
  bool rewrite(iris::arith::SarOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isSInteger(dt)) {
      return false;
    }

    auto* cR = getConstFromInput(op.getInputY());
    if (!cR || !isZeroAttribute(cR->getAttr())) {
      return false;
    }

    auto* xDef = op.getInputX().getDefiningOp();
    if (!xDef) {
      return false;
    }

    op.replaceAllUsesWith(*xDef);
    rewriter.eraseOp(op);
    return true;
  }
};

// sar(0, x) -> 0
class SarZeroArgPattern final
  : public ArithBinaryPeepholePattern<SarZeroArgPattern, iris::arith::SarOp> {
public:
  bool rewrite(iris::arith::SarOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isSInteger(dt)) {
      return false;
    }

    auto* cL = getConstFromInput(op.getInputX());
    if (!cL || !isZeroAttribute(cL->getAttr())) {
      return false;
    }

    auto zeroAttr = makeZeroConstAttribute(dt);
    auto newConst =
      std::make_unique<iris::arith::ConstantOp>(std::move(zeroAttr));
    rewriter.replaceOpWith(op, std::move(newConst));
    return true;
  }
};

// --- shr patterns -----------------------------------------------------------

// shr(x, 0) -> x
class ShrZeroShiftPattern final
  : public ArithBinaryPeepholePattern<ShrZeroShiftPattern, iris::arith::ShrOp> {
public:
  bool rewrite(iris::arith::ShrOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isUInteger(dt)) {
      return false;
    }

    auto* cR = getConstFromInput(op.getInputY());
    if (!cR || !isZeroAttribute(cR->getAttr())) {
      return false;
    }

    auto* xDef = op.getInputX().getDefiningOp();
    if (!xDef) {
      return false;
    }

    op.replaceAllUsesWith(*xDef);
    rewriter.eraseOp(op);
    return true;
  }
};

// shr(0, x) -> 0
class ShrZeroArgPattern final
  : public ArithBinaryPeepholePattern<ShrZeroArgPattern, iris::arith::ShrOp> {
public:
  bool rewrite(iris::arith::ShrOp& op, PatternRewriter& rewriter) const {
    auto dt = op.getDataType();
    if (!isUInteger(dt)) {
      return false;
    }

    auto* cL = getConstFromInput(op.getInputX());
    if (!cL || !isZeroAttribute(cL->getAttr())) {
      return false;
    }

    auto zeroAttr = makeZeroConstAttribute(dt);
    auto newConst =
      std::make_unique<iris::arith::ConstantOp>(std::move(zeroAttr));
    rewriter.replaceOpWith(op, std::move(newConst));
    return true;
  }
};

// --- not patterns -----------------------------------------------------------

// not(not(x)) -> x
class NotDoubleNegationPattern final : public Pattern {
public:
  bool matchAndRewrite(Operation& op,
                       PatternRewriter& rewriter) const override {
    auto* outerNot = dynamic_cast<iris::arith::NotOp*>(&op);
    if (!outerNot) {
      return false;
    }

    auto* innerOp = outerNot->getInput().getDefiningOp();
    auto* innerNot = dynamic_cast<iris::arith::NotOp*>(innerOp);
    if (!innerNot) {
      return false;
    }

    auto* xDef = innerNot->getInput().getDefiningOp();
    if (!xDef) {
      return false;
    }

    outerNot->replaceAllUsesWith(*xDef);
    rewriter.eraseOp(op);

    if (!innerNot->hasUsers()) {
      rewriter.eraseOp(*innerNot);
    }

    return true;
  }
};

} // namespace

ArithPeepHolePass::ArithPeepHolePass() {
  // add
  addPattern<AddZeroRhsPattern>();
  addPattern<AddZeroLhsPattern>();
  addPattern<AddRotateConstantsPattern>();

  // sub
  addPattern<SubZeroRhsPattern>();
  addPattern<SubSelfToZeroPattern>();

  // mul
  addPattern<MulOneRhsPattern>();
  addPattern<MulOneLhsPattern>();
  addPattern<MulZeroRhsPattern>();
  addPattern<MulZeroLhsPattern>();
  addPattern<MulRotateConstantsPattern>();

  // div
  addPattern<DivOneRhsPattern>();

  // and
  addPattern<AndZeroRhsPattern>();
  addPattern<AndZeroLhsPattern>();
  addPattern<AndAllOnesRhsPattern>();
  addPattern<AndAllOnesLhsPattern>();
  addPattern<AndSelfPattern>();

  // or
  addPattern<OrZeroRhsPattern>();
  addPattern<OrZeroLhsPattern>();
  addPattern<OrAllOnesRhsPattern>();
  addPattern<OrAllOnesLhsPattern>();
  addPattern<OrSelfPattern>();

  // xor
  addPattern<XorZeroRhsPattern>();
  addPattern<XorZeroLhsPattern>();
  addPattern<XorSelfToZeroPattern>();
  addPattern<XorAllOnesRhsToNotPattern>();
  addPattern<XorAllOnesLhsToNotPattern>();

  // sal
  addPattern<SalZeroShiftPattern>();
  addPattern<SalZeroArgPattern>();

  // sar
  addPattern<SarZeroShiftPattern>();
  addPattern<SarZeroArgPattern>();

  // shl
  addPattern<ShlZeroShiftPattern>();
  addPattern<ShlZeroArgPattern>();

  // shr
  addPattern<ShrZeroShiftPattern>();
  addPattern<ShrZeroArgPattern>();

  // not
  addPattern<NotDoubleNegationPattern>();
}

} // namespace arith
} // namespace opt
} // namespace iris
