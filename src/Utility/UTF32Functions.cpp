#include "Utility/UTF32Functions.hpp"
#include <stdexcept>

namespace Cygni {
namespace Utility {

int HexToInt(std::u32string hex) {
  int val = 0;
  for (size_t i = 0; i < hex.size(); i++) {
    val = val * 16;
    if (hex[i] >= U'0' && hex[i] <= U'9') {
      val = val + (hex[i] - U'0');
    } else if (hex[i] >= 'a' && hex[i] <= U'f') {
      val = val + (hex[i] - U'a') + 10;
    } else if (hex[i] >= 'A' && hex[i] <= U'F') {
      val = val + (hex[i] - U'A') + 10;
    } else {
      throw std::invalid_argument("hex digit character out of range");
    }
  }
  return val;
}
std::string UTF32ToUTF8(const std::u32string &utf32) {
  std::string res;
  for (char32_t cp : utf32) {
    if (cp < 0x80) {
      res.push_back(cp & 0x7F);
    } else if (cp < 0x800) {
      res.push_back(((cp >> 6) & 0x1F) | 0xC0);
      res.push_back((cp & 0x3F) | 0x80);
    } else if (cp < 0x10000) {
      res.push_back(((cp >> 12) & 0x0F) | 0xE0);
      res.push_back(((cp >> 6) & 0x3F) | 0x80);
      res.push_back((cp & 0x3F) | 0x80);
    } else {
      res.push_back(((cp >> 18) & 0x07) | 0xF0);
      res.push_back(((cp >> 12) & 0x3F) | 0x80);
      res.push_back(((cp >> 6) & 0x3F) | 0x80);
      res.push_back((cp & 0x3F) | 0x80);
    }
  }
  return res;
}
std::u32string UTF8ToUTF32(const std::string &utf8) {
  std::u32string res;
  int i = 0;
  int n = static_cast<int>(utf8.size());
  while (i < n) {
    if ((utf8[i] & 0x80) == 0) {
      res.push_back(utf8[i]);
      i = i + 1;
    } else if ((utf8[i] & 0xE0) == 0xC0) {
      if (i + 1 < n) {
        res.push_back(((utf8[i] & 0x1F) << 6) | (utf8[i + 1] & 0x3F));
        i = i + 2;
      } else {
        throw std::invalid_argument("The sequence is truncated.");
      }
    } else if ((utf8[i] & 0xF0) == 0xE0) {
      if (i + 2 < n) {
        res.push_back(((utf8[i] & 0x0F) << 12) | ((utf8[i + 1] & 0x3F) << 6) |
                      (utf8[i + 2] & 0x3F));
        i = i + 3;
      } else {
        throw std::invalid_argument("The sequence is truncated.");
      }
    } else if ((utf8[i] & 0xF8) == 0xF0) {
      if (i + 3 < n) {
        res.push_back(((utf8[i] & 0x07) << 18) | ((utf8[i + 1] & 0x3F) << 12) |
                      ((utf8[i + 2] & 0x3F) << 6) | (utf8[i + 3] & 0x3F));
        i = i + 4;
      } else {
        throw std::invalid_argument("The sequence is truncated.");
      }
    } else {
      throw std::invalid_argument("Illegal starting byte");
    }
  }
  return res;
}
}; /* namespace Utility */
}; /* namespace Cygni */