#ifndef INCLUDE_EXCEPTION_HPP
#define INCLUDE_EXCEPTION_HPP

#include <stdexcept>

namespace iris {

class IrisException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

} // namespace iris

#endif // INCLUDE_EXCEPTION_HPP