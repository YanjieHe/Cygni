#ifndef CYGNI_UTILITY_STRING_UTILS_HPP
#define CYGNI_UTILITY_STRING_UTILS_HPP

#include <string>
#include <vector>

namespace Cygni
{
namespace Utility
{

class StringUtils
{
  public:
    static std::u32string Join(std::u32string separator, const std::vector<std::u32string> &items);
};

}; /* namespace Utility */
}; /* namespace Cygni */

#endif /* CYGNI_UTILITY_STRING_UTILS_HPP */