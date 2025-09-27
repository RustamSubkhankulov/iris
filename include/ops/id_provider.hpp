#ifndef INCLUDE_OSP_ID_PROVIDER_HPP
#define INCLUDE_OSP_ID_PROVIDER_HPP

#include <utils.hpp>

namespace iris {

class IDProvider : public Singleton<IDProvider> {
private:
  std::size_t m_curID = 0LLU;

public:
  std::size_t obtainID() {
    return m_curID++;
  }

  void reset() {
    m_curID = 0LLU;
  }
};

} // namespace iris

#endif // INCLUDE_OSP_ID_PROVIDER_HPP
