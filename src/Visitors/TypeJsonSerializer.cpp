#include "Visitors/TypeJsonSerializer.hpp"
#include "Utility/Convert.hpp"
#include "Utility/StringUtils.hpp"
#include "Utility/UTF32Functions.hpp"

namespace Cygni
{
namespace Visitors
{

using namespace Expressions;

Json TypeJsonSerializer::ToJson(const Type *type)
{
    Json json;

    json["TypeCode"] = Utility::EnumToString(type->GetTypeCode());
    if (type->GetTypeCode() == TypeCode::Array)
    {
        const ArrayType *arrayType = static_cast<const ArrayType *>(type);
        json["ElementType"] = ToJson(arrayType->ElementType());

        return json;
    }
    else if (type->GetTypeCode() == TypeCode::Callable)
    {
        const CallableType *callableType = static_cast<const CallableType *>(type);
        std::vector<Json> arguments;
        for (auto arg : callableType->Arguments())
        {
            arguments.push_back(ToJson(arg));
        }
        json["Arguments"] = arguments;
        json["ReturnType"] = ToJson(callableType->GetReturnType());

        return json;
    }
    else if (type->GetTypeCode() == TypeCode::Union)
    {
        const UnionType *unionType = static_cast<const UnionType *>(type);
        std::vector<Json> types;
        for (auto t : unionType->GetTypes())
        {
            types.push_back(ToJson(t));
        }
        json["Types"] = types;

        return json;
    }
    else if (type->GetTypeCode() == TypeCode::Structure)
    {
        const StructureType *structureType = static_cast<const StructureType *>(type);
        std::vector<Json> fields;
        size_t size = structureType->Fields().GetAllItems().size();
        for (size_t i = 0; i < size; i++)
        {
            Json fieldJson;
            fieldJson["Name"] = structureType->Fields().GetKeyByIndex(i);
            fieldJson["Type"] = ToJson(structureType->Fields().GetItemByIndex(i));
            fields.push_back(fieldJson);
        }
        json["QualifiedName"] = Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", structureType->QualifiedName()));

        return json;
    }
    else
    {
        return json;
    }
}

}; /* namespace Visitors */
}; /* namespace Cygni */
