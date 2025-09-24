#ifndef INCLUDE_DIALECTS_OPCODES_HPP
#define INCLUDE_DIALECTS_OPCODES_HPP

#include <ops/common.hpp>

namespace iris {

enum GlobalOpcodes : opcode_t {
  ADD = nullopcode + 1,
  SUB,
  MUL,
  DIV,
  CONST,
  CAST,
  CMP,
  PARAM,
  COPY,
  CALL,
  JUMP,
  RETURN,
  PHI,
};

} // namespace iris

#endif // INCLUDE_DIALECTS_OPCODES_HPP
