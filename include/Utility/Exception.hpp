#ifndef CYGNI_UTILITY_EXCEPTION_HPP
#define CYGNI_UTILITY_EXCEPTION_HPP

#include <exception>
#include <string>

namespace Cygni {
namespace Utility {

class Exception : public std::exception {
private:
  std::string source;
  int line;
  std::string message;
  const std::exception *innerException;

public:
  Exception(std::string source, int line, std::string message,
            const std::exception *innerException)
      : source{source}, line{line}, message{message},
        innerException{innerException} {}

  const char *what() const noexcept override { return message.c_str(); }

  const std::string &Source() const { return source; }

  int Line() const { return line; }

  const std::string &Message() const { return message; }

  const std::exception *InnerException() const { return innerException; }
};

}; /* namespace Utility */
}; /* namespace Cygni */

#endif /* CYGNI_UTILITY_EXCEPTION_HPP */