#include "Visitors/ExpressionJsonSerializer.hpp"

#include "Utility/Convert.hpp"
#include "Utility/UTF32Functions.hpp"

namespace Cygni
{
namespace Visitors
{

Json ExpressionJsonSerializer::VisitBinary(const BinaryExpression *node)
{
    Json json;

    json["NodeType"] = Utility::EnumToString(node->NodeType());
    json["Left"] = Visit(node->Left());
    json["Right"] = Visit(node->Right());
    json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());

    return json;
}

Json ExpressionJsonSerializer::VisitConstant(const ConstantExpression *node)
{
    Json value;
    switch (node->GetTypeCode())
    {
    case TypeCode::Int32: {
        value = std::get<int32_t>(node->Value());
        break;
    }
    case TypeCode::Int64: {
        value = std::get<int64_t>(node->Value());
        break;
    }
    case TypeCode::Float32: {
        value = std::get<float_t>(node->Value());
        break;
    }
    case TypeCode::Float64: {
        value = std::get<double_t>(node->Value());
        break;
    }
    case TypeCode::Boolean: {
        value = std::get<bool>(node->Value());
        break;
    }
    case TypeCode::Char: {
        value = Utility::UTF32ToUTF8(std::get<std::u32string>(node->Value()));
        break;
    }
    case TypeCode::String: {
        value = Utility::UTF32ToUTF8(std::get<std::u32string>(node->Value()));
        break;
    }
    default: {
        throw std::invalid_argument("constant expression node type not supported");
    }
    }
    Json json;
    json["NodeType"] = Utility::EnumToString(node->NodeType());
    json["Value"] = value;
    json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());

    return json;
}

Json ExpressionJsonSerializer::VisitParameter(const ParameterExpression *node)
{
    Json json;

    json["NodeType"] = Utility::EnumToString(node->NodeType());
    json["Name"] = Utility::UTF32ToUTF8(node->Name());
    json["Type"] = TypeSyntaxToString(node->GetTypeSyntax());
    json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());

    return json;
}

Json ExpressionJsonSerializer::VisitBlock(const BlockExpression *node)
{
    Json json;

    json["NodeType"] = Utility::EnumToString(node->NodeType());
    json["Expressions"] = Json::array();
    for (const auto &expr : node->Expressions())
    {
        json["Expressions"].push_back(Visit(expr));
    }
    json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());

    return json;
}

Json ExpressionJsonSerializer::VisitConditional(const ConditionalExpression *node)
{
    Json json;

    json["NodeType"] = Utility::EnumToString(node->NodeType());
    json["Test"] = Visit(node->Test());
    json["IfTrue"] = Visit(node->IfTrue());
    json["IfFalse"] = Visit(node->IfFalse());
    json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());

    return json;
}

Json ExpressionJsonSerializer::VisitUnary(const UnaryExpression *node)
{
    Json json;

    json["NodeType"] = Utility::EnumToString(node->NodeType());
    json["Operand"] = Visit(node->Operand());
    json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());

    return json;
}

Json ExpressionJsonSerializer::VisitCall(const CallExpression *node)
{
    Json json;

    json["NodeType"] = Utility::EnumToString(node->NodeType());
    json["ReturnType"] = Visit(node->Function());
    json["Arguments"] = Json::array();
    for (const auto &arg : node->Arguments())
    {
        json["Arguments"].push_back(Visit(arg));
    }
    json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());

    return json;
}

Json ExpressionJsonSerializer::VisitLambda(const LambdaExpression *node)
{
    Json json;

    json["NodeType"] = Utility::EnumToString(node->NodeType());
    json["Name"] = Utility::UTF32ToUTF8(node->Name());
    json["Parameters"] = Json::array();
    for (const auto &param : node->Parameters())
    {
        json["Parameters"].push_back(Visit(param));
    }
    json["Body"] = Visit(node->Body());
    json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());

    return json;
}

Json ExpressionJsonSerializer::VisitWhileLoop(const WhileLoopExpression *node)
{
    Json json;

    json["NodeType"] = Utility::EnumToString(node->NodeType());
    json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());

    json["Condition"] = Visit(node->Condition());
    json["Body"] = Visit(node->Body());

    return json;
}

Json ExpressionJsonSerializer::VisitDefault(const DefaultExpression *node)
{
    Json json;

    json["NodeType"] = Utility::EnumToString(node->NodeType());
    json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());
    json["Type"] = TypeSyntaxToString(node->GetTypeSyntax());

    return json;
}

Json ExpressionJsonSerializer::VisitVariableDeclaration(const VariableDeclarationExpression *node)
{
    Json json;

    json["NodeType"] = Utility::EnumToString(node->NodeType());
    json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());
    json["Name"] = Utility::UTF32ToUTF8(node->Name());
    json["Initializer"] = Visit(node->Initializer());

    return json;
}

Json ExpressionJsonSerializer::VisitNew(const NewExpression *node)
{
    Json json;

    json["NodeType"] = Utility::EnumToString(node->NodeType());
    json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());
    std::vector<Json> fieldsInitializationJson;
    for (const std::u32string &key : node->FieldsInitialization().GetAllKeys())
    {
        fieldsInitializationJson.push_back(
            FieldInitializationToJson(key, node->FieldsInitialization().GetItemByKey(key)));
    }
    json["FieldsInitialization"] = fieldsInitializationJson;
    json["Type"] = TypeSyntaxToString(node->GetTypeSyntax());

    return json;
}

Json ExpressionJsonSerializer::VisitMember(const MemberExpression *node)
{
    Json json;

    json["NodeType"] = Utility::EnumToString(node->NodeType());
    json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());

    json["Expression"] = Visit(node->GetExpression());
    json["FieldName"] = Utility::UTF32ToUTF8(node->FieldName());

    return json;
}

Json ExpressionJsonSerializer::SourceRangeToJson(const SourceRange &sourceRange)
{
    Json json;

    json["FileName"] = sourceRange.CodeFile()->FileName();
    json["StartLine"] = sourceRange.StartLine();
    json["StartColumn"] = sourceRange.StartColumn();
    json["EndLine"] = sourceRange.EndLine();
    json["EndColumn"] = sourceRange.EndColumn();

    return json;
}

Json ExpressionJsonSerializer::FieldInitializationToJson(const std::u32string &fieldName, const Expression *value)
{
    Json json;

    json["FieldName"] = Utility::UTF32ToUTF8(fieldName);
    json["FieldValue"] = Visit(value);

    return json;
}

Json ExpressionJsonSerializer::ExpressionToJson(const Expression *node)
{
    ExpressionJsonSerializer serializer;

    return serializer.Visit(node);
}

std::string ExpressionJsonSerializer::TypeSyntaxToString(const TypeSyntax *typeSyntax)
{
    if (typeSyntax == nullptr)
    {
        return "<Unknown>";
    }
    else
    {
        std::string result;
        const auto &qualifiedName = typeSyntax->QualifiedName();
        for (size_t i = 0; i < qualifiedName.size(); ++i)
        {
            if (i > 0)
            {
                result += "::";
            }
            result += Utility::UTF32ToUTF8(qualifiedName.at(i));
        }

        if (!typeSyntax->Arguments().empty())
        {
            result += "[";
            for (size_t i = 0; i < typeSyntax->Arguments().size(); ++i)
            {
                if (i > 0)
                {
                    result += ", ";
                }
                result += TypeSyntaxToString(typeSyntax->Arguments().at(i));
            }
            result += "]";
        }

        return result;
    }
}

}; /* namespace Visitors */
}; /* namespace Cygni */