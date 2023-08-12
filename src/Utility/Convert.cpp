#include "Utility/Convert.hpp"

#include "Utility/UTF32Functions.hpp"


namespace Cygni {
namespace Utility {

std::u32string Convert::ToString(const int32_t& i) {
  return UTF8ToUTF32(std::to_string(i));
}

std::u32string Convert::ToString(const char32_t& c) {
  return std::u32string(1, c);
}

std::u32string Convert::ToString(const std::u32string& s) { return s; }

std::u32string Convert::ToString(const std::string& s) {
  return UTF8ToUTF32(s);
}

}; /* namespace Utility */
}; /* namespace Cygni */