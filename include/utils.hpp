#ifndef INCLUDE_UTILS_HPP
#define INCLUDE_UTILS_HPP

#include <cassert>
#include <concepts>
#include <limits>
#include <stdexcept>

#include <exception.hpp>

namespace iris::detail {

template <std::unsigned_integral IdType>
class IDProvider final {
public:
  IdType obtainID() {
    if (m_curID == std::numeric_limits<IdType>::max()) {
      throw std::overflow_error("ID Provider has overflown!");
    }
    return m_curID++;
  }

  IdType getLastID() const noexcept {
    return m_curID;
  }

  void reset() noexcept {
    m_curID = 0;
  }

private:
  IdType m_curID = 0;
};

} // namespace iris::detail

#endif // INCLUDE_UTILS_HPP
