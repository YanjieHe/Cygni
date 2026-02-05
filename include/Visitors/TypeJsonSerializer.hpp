#ifndef CYGNI_VISITORS_TYPE_JSON_SERIALIZER_HPP
#define CYGNI_VISITORS_TYPE_JSON_SERIALIZER_HPP

#include "Expressions/Type.hpp"
#include <nlohmann/json.hpp>

using Json = nlohmann::json;

namespace Cygni
{
namespace Visitors
{

class TypeJsonSerializer
{
  public:
    static Json ToJson(const Expressions::Type *type);
};

}; /* namespace Visitors */
}; /* namespace Cygni */

#endif /* CYGNI_VISITORS_TYPE_JSON_SERIALIZER_HPP */
