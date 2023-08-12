#ifndef CYGNI_UTILITY_UTF32_FUNCTIONS_HPP
#define CYGNI_UTILITY_UTF32_FUNCTIONS_HPP

#include <string>

namespace Cygni {
namespace Utility {

inline static bool IsDigit(char32_t c) { return (c >= U'0' && c <= U'9'); }

inline static bool IsLetter(char32_t c) {
  return (c >= U'a' && c <= U'z') || (c >= U'A' && c <= U'Z');
}

inline static bool IsWhiteSpace(char32_t c) {
  return c == U' ' || c == U'\t' || c == U'\v' || c == U'\r' || c == U'\n';
}

int HexToInt(std::u32string hex);
std::string UTF32ToUTF8(const std::u32string &utf32);
std::u32string UTF8ToUTF32(const std::string &utf8);

}; /* namespace Utility */
}; /* namespace Cygni */

#endif /* CYGNI_UTILITY_UTF32_FUNCTIONS_HPP */