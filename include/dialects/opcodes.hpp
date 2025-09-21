#ifndef INCLUDE_DIALECTS_OPCODES_HPP
#define INCLUDE_DIALECTS_OPCODES_HPP

#include <common/types.hpp>

namespace iris {

enum GlobalOpcodes : opcode_t {
  ADD = nullopcode + 1,
  SUB,
  MUL,
  DIV,
  CONST,
  CAST,
  CMP
};

} // namespace iris

#endif // INCLUDE_DIALECTS_OPCODES_HPP
