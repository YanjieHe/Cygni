#ifndef CYGNI_UTILITY_CONVERT_HPP
#define CYGNI_UTILITY_CONVERT_HPP

#include <string>

namespace Cygni {
namespace Utility {

class Convert {
 public:
  static std::u32string ToString(const int32_t& i);
  static std::u32string ToString(const char32_t& c);
  static std::u32string ToString(const std::u32string& s);
  static std::u32string ToString(const std::string& s);
};

};  /* namespace Utility */
};  /* namespace Cygni */

#endif /* CYGNI_UTILITY_CONVERT_HPP */