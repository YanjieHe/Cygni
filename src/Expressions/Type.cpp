#include "Expressions/Type.hpp"
#include "Utility/Convert.hpp"
#include "Utility/StringUtils.hpp"
#include "Utility/UTF32Functions.hpp"
#include <stdexcept>

namespace Cygni
{
namespace Expressions
{

TypeFactory::~TypeFactory()
{
    for (auto type : types)
    {
        delete type;
    }
}

Type *TypeFactory::CreateBasicType(TypeCode typeCode)
{
    static UnknownType unknownType;
    static EmptyType emptyType;
    static Int32Type int32Type;
    static Int64Type int64Type;
    static Float32Type float32Type;
    static Float64Type float64Type;
    static BooleanType booleanType;
    static CharType charType;
    static StringType stringType;

    switch (typeCode)
    {
    case TypeCode::Unknown:
        return &unknownType;
    case TypeCode::Empty:
        return &emptyType;
    case TypeCode::Int32:
        return &int32Type;
    case TypeCode::Int64:
        return &int64Type;
    case TypeCode::Float32:
        return &float32Type;
    case TypeCode::Float64:
        return &float64Type;
    case TypeCode::Boolean:
        return &booleanType;
    case TypeCode::Char:
        return &charType;
    case TypeCode::String:
        return &stringType;
    default:
        throw std::invalid_argument("Incorrect type code for a basic type.");
    }
}

bool TypeFactory::IsBasicType(TypeCode typeCode)
{
    switch (typeCode)
    {
    case TypeCode::Unknown:
    case TypeCode::Empty:
    case TypeCode::Int32:
    case TypeCode::Int64:
    case TypeCode::Float32:
    case TypeCode::Float64:
    case TypeCode::Boolean:
    case TypeCode::Char:
    case TypeCode::String:
        return true;
    default:
        return false;
    }
}

bool TypeFactory::AreTypesEqual(const Type *a, const Type *b)
{
    if (a->GetTypeCode() == b->GetTypeCode())
    {
        if (IsBasicType(a->GetTypeCode()))
        {
            return true;
        }
        else if (a->GetTypeCode() == TypeCode::Array)
        {
            return AreTypesEqual(static_cast<const ArrayType *>(a)->ElementType(),
                                 static_cast<const ArrayType *>(b)->ElementType());
        }
        else if (a->GetTypeCode() == TypeCode::Callable)
        {
            auto callableA = static_cast<const CallableType *>(a);
            auto callableB = static_cast<const CallableType *>(b);
            return AreOrderedTypesEqual(callableA->Arguments(), callableB->Arguments()) &&
                   AreTypesEqual(callableA->GetReturnType(), callableB->GetReturnType());
        }
        else if (a->GetTypeCode() == TypeCode::Union)
        {
            auto unionA = static_cast<const UnionType *>(a);
            auto unionB = static_cast<const UnionType *>(b);
            return AreUnorderedTypesEqual(unionA->GetTypes(), unionB->GetTypes());
        }
        else if (a->GetTypeCode() == TypeCode::Structure)
        {
            const StructureType *structureTypeA = static_cast<const StructureType *>(a);
            const StructureType *structureTypeB = static_cast<const StructureType *>(b);
            return structureTypeA->QualifiedName() == structureTypeB->QualifiedName();
        }
        else
        {
            throw std::invalid_argument("not supported type");
        }
    }
    else
    {
        return false;
    }
}

ArrayType *TypeFactory::CreateArrayType(const Type *elementType)
{
    return static_cast<ArrayType *>(CreateType(new ArrayType(elementType)));
}

const Type *TypeFactory::CreateUnionType(const Type *a, const Type *b)
{
    if (AreTypesEqual(a, b))
    {
        return a;
    }
    else
    {
        if (a->GetTypeCode() == TypeCode::Union && b->GetTypeCode() != TypeCode::Union)
        {
            auto unionA = static_cast<const UnionType *>(a);
            for (auto t : unionA->GetTypes())
            {
                if (AreTypesEqual(t, b))
                {
                    return a;
                }
            }
            std::vector<const Type *> types = unionA->GetTypes();
            types.push_back(b);

            return CreateType(new UnionType(types));
        }
        else if (a->GetTypeCode() != TypeCode::Union && b->GetTypeCode() == TypeCode::Union)
        {
            return CreateUnionType(b, a);
        }
        else if (a->GetTypeCode() == TypeCode::Union && b->GetTypeCode() == TypeCode::Union)
        {
            auto unionA = static_cast<const UnionType *>(a);
            auto unionB = static_cast<const UnionType *>(b);
            std::vector<const Type *> types = unionA->GetTypes();
            for (auto bType : unionB->GetTypes())
            {
                bool found = false;
                for (auto aType : types)
                {
                    if (AreTypesEqual(aType, bType))
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    types.push_back(bType);
                }
            }

            return CreateType(new UnionType(types));
        }
        else
        {
            return CreateType(new UnionType({a, b}));
        }
    }
}

CallableType *TypeFactory::CreateCallableType(std::vector<const Type *> arguments, const Type *returnType)
{
    return static_cast<CallableType *>(CreateType(new CallableType(arguments, returnType)));
}

bool TypeFactory::AreOrderedTypesEqual(const std::vector<const Type *> &a, const std::vector<const Type *> &b)
{
    if (a.size() == b.size())
    {
        for (size_t i = 0; i < a.size(); i++)
        {
            if (!AreTypesEqual(a[i], b[i]))
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

StructureType *TypeFactory::CreateStructureType(std::vector<std::u32string> qualifiedName,
                                                Utility::OrderPreservingMap<std::u32string, const Type *> fields)
{
    return static_cast<StructureType *>(CreateType(new StructureType(qualifiedName, fields)));
}

Json TypeFactory::ToJson(const Type *type)
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

bool TypeFactory::AreUnorderedTypesEqual(const std::vector<const Type *> &a, const std::vector<const Type *> &b)
{
    if (a.size() == b.size())
    {
        for (size_t i = 0; i < a.size(); i++)
        {
            bool found = false;
            for (size_t j = 0; j < b.size(); j++)
            {
                if (AreTypesEqual(a[i], b[j]))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

Type *TypeFactory::CreateType(Type *type)
{
    types.push_back(type);
    return type;
}
}; /* namespace Expressions */
}; /* namespace Cygni */