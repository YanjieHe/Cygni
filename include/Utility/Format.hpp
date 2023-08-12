#ifndef CYGNI_UTILITY_FORMAT_HPP
#define CYGNI_UTILITY_FORMAT_HPP

#include <sstream>
#include <string>

#include "Utility/Convert.hpp"


namespace Cygni {
namespace Utility {

std::u32string FormatInternal(std::u32string fmt,
                              std::basic_ostringstream<char32_t>& stream,
                              int i);

template <class T, class... Args>
std::u32string FormatInternal(std::u32string fmt,
                              std::basic_ostringstream<char32_t>& stream, int i,
                              const T& head, const Args&... rest) {
  int n = static_cast<int>(fmt.size());
  while (i < n) {
    if (fmt[i] == U'{') {
      if (i + 1 >= n) {
        throw std::invalid_argument("wrong format");
      } else {
        if (fmt[i + 1] == U'{') {
          // escape left brace
          stream.put(U'{');
          i = i + 2;
        } else if (fmt[i + 1] == U'}') {
          stream << Convert::ToString(head);
          i = i + 2;
          return FormatInternal(fmt, stream, i, rest...);
        } else {
          throw std::invalid_argument("expecting right brace for closure");
        }
      }
    } else if (fmt[i] == U'}') {
      if (i + 1 >= n) {
        throw std::invalid_argument("wrong format");
      } else {
        if (fmt[i + 1] == U'}') {
          stream.put(U'}');
          i = i + 2;
        } else {
          throw std::invalid_argument("expecting a right brace for escaping");
        }
      }
    } else {
      stream.put(fmt[i]);
      i = i + 1;
    }
  }
  return stream.str();
}

inline std::u32string FormatInternal(std::u32string fmt,
                                     std::basic_ostringstream<char32_t>& stream,
                                     int i) {
  int n = static_cast<int>(fmt.size());
  while (i < n) {
    if (fmt[i] == U'{') {
      if (i + 1 >= n) {
        throw std::invalid_argument("wrong format");
      } else {
        if (fmt[i + 1] == U'{') {
          // escape left brace
          stream.put(U'{');
          i = i + 2;
        } else if (fmt[i + 1] == U'}') {
          throw std::invalid_argument("out of arguments");
        } else {
          throw std::invalid_argument("expecting right brace for closure");
        }
      }
    } else if (fmt[i] == U'}') {
      if (i + 1 >= n) {
        throw std::invalid_argument("wrong format");
      } else {
        if (fmt[i + 1] == U'}') {
          stream.put(U'}');
          i = i + 2;
        } else {
          throw std::invalid_argument("expecting a right brace for escaping");
        }
      }
    } else {
      stream.put(fmt[i]);
      i = i + 1;
    }
  }
  return stream.str();
}

template <class... Args>
std::u32string Format(std::u32string fmt, const Args&... args) {
  std::basic_ostringstream<char32_t> stream;
  int i = 0;
  return FormatInternal(fmt, stream, i, args...);
}

}; /* namespace Utility */
}; /* namespace Cygni */

#endif /* CYGNI_UTILITY_FORMAT_HPP */