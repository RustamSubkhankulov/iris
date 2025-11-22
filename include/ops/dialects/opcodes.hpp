#ifndef INCLUDE_DIALECTS_OPCODES_HPP
#define INCLUDE_DIALECTS_OPCODES_HPP

#include <ops/common.hpp>

namespace iris {

enum GlobalOpcodes : opcode_t {
  /* Arith */
  ADD = nullopcode + 1,
  SUB,
  MUL,
  DIV,
  AND,
  OR,
  XOR,
  SAL,
  SAR,
  SHL,
  SHR,
  NOT,
  CONSTANT,
  CAST,
  COMPARE,

  /* Builtin */
  PARAM,
  COPY,

  /* Ctrlflow */
  CALL,
  JUMP,
  JUMPC,
  RETURN,
  PHI,
};

} // namespace iris

#endif // INCLUDE_DIALECTS_OPCODES_HPP
