#ifndef CYGNI_VISITORS_SCOPE_EXCEPTION_HPP
#define CYGNI_VISITORS_SCOPE_EXCEPTION_HPP

#include "Utility/Exception.hpp"

namespace Cygni {
namespace Visitors {

class ScopeException : public Utility::Exception {
private:
  std::u32string name;

public:
  ScopeException(std::string source, int line, const std::string &message,
                 const std::exception *innerException,
                 const std::u32string name)
      : Exception(source, line, message, innerException), name{name} {}

  const std::u32string &Name() const { return name; }
};

}; /* namespace Visitors */
}; /* namespace Cygni */

#endif /* CYGNI_VISITORS_SCOPE_EXCEPTION_HPP */