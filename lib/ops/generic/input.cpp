#include <ops/generic/input.hpp>
#include <ops/generic/operation.hpp>

namespace iris {

DataType Input::getDataType() const {
  if (isEmpty()) {
    return DataType::NONE;
  }

  auto* defOp = getDefiningOp();
  return defOp->getDataType();
}

} // namespace iris