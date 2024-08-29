#include "Utility/StringUtils.hpp"

namespace Cygni
{
namespace Utility
{

std::u32string StringUtils::Join(std::u32string separator, const std::vector<std::u32string> &items)
{
    if (items.empty())
    {
        return U"";
    }
    else
    {
        size_t count = 0;
        for (const std::u32string &item : items)
        {
            count += item.size();
        }
        count += separator.size() * (items.size() - 1);
        std::u32string result;
        result.reserve(count);
        result += items.front();
        for (size_t i = 1; i < items.size(); i++)
        {
            result += separator;
            result += items.at(i);
        }

        return result;
    }
}

}; /* namespace Utility */
}; /* namespace Cygni */