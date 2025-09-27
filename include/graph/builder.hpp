#ifndef INCLUDE_OPS_OPBUILDER
#define INCLUDE_OPS_OPBUILDER

#include <graph/basic_block.hpp>
#include <graph/region.hpp>

namespace iris {

class Builder {
private:
  BasicBlock* m_currBasicBlock = nullptr;

public:
  Builder() = default;
};

} // namespace iris

#endif // INCLUDE_OPS_OPBUILDER
