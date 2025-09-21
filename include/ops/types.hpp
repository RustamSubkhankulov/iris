#ifndef INCLUDE_COMMON_TYPES_HPP
#define INCLUDE_COMMON_TYPES_HPP

namespace iris {

// Type for representing operation's code
using opcode_t = uint8_t;

// Reserved operation code for 'empty' op.
constexpr opcode_t nullopcode = 0U;

} // namespace iris

#endif // INCLUDE_COMMON_TYPES_HPP
