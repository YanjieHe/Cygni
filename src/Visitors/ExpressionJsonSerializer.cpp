#include "Visitors/ExpressionJsonSerializer.hpp"

#include <magic_enum/magic_enum.hpp>
#include "Utility/UTF32Functions.hpp"

namespace Cygni {
namespace Visitors {

Json ExpressionJsonSerializer::VisitBinary(const BinaryExpression *node) {
  Json json;
  json["NodeType"] = magic_enum::enum_name<ExpressionType>(node->NodeType());
  json["Left"] = Visit(node->Left());
  json["Right"] = Visit(node->Right());
  json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());
  return json;
}

Json ExpressionJsonSerializer::VisitConstant(const ConstantExpression *node) {
  Json value;
  switch (node->GetTypeCode()) {
  case TypeCode::Int32: {
    value = std::any_cast<int32_t>(node->Value());
    break;
  }
  case TypeCode::Int64: {
    value = std::any_cast<int64_t>(node->Value());
    break;
  }
  case TypeCode::Float32: {
    value = std::any_cast<float_t>(node->Value());
    break;
  }
  case TypeCode::Float64: {
    value = std::any_cast<double_t>(node->Value());
    break;
  }
  case TypeCode::Boolean: {
    value = std::any_cast<bool>(node->Value());
    break;
  }
  case TypeCode::Char: {
    value = Utility::UTF32ToUTF8(std::any_cast<std::u32string>(node->Value()));
    break;
  }
  case TypeCode::String: {
    value = Utility::UTF32ToUTF8(std::any_cast<std::u32string>(node->Value()));
    break;
  }
  default: {
    throw std::invalid_argument("constant expression node type not supported");
  }
  }
  Json json;
  json["NodeType"] = magic_enum::enum_name<ExpressionType>(node->NodeType());
  json["Value"] = value;
  json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());
  return json;
}

Json ExpressionJsonSerializer::VisitParameter(const ParameterExpression *node) {
  Json json;
  json["NodeType"] = magic_enum::enum_name<ExpressionType>(node->NodeType());
  json["Name"] = Utility::UTF32ToUTF8(node->Name());
  json["Type"] =
      magic_enum::enum_name<TypeCode>(node->GetType()->GetTypeCode());
  json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());
  return json;
}

Json ExpressionJsonSerializer::VisitBlock(const BlockExpression *node) {
  Json json;
  json["NodeType"] = magic_enum::enum_name<ExpressionType>(node->NodeType());
  json["Expressions"] = Json::array();
  for (const auto &expr : node->Expressions()) {
    json["Expressions"].push_back(Visit(expr));
  }
  json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());
  return json;
}

Json ExpressionJsonSerializer::VisitConditional(
    const ConditionalExpression *node) {
  Json json;
  json["NodeType"] = magic_enum::enum_name<ExpressionType>(node->NodeType());
  json["Test"] = Visit(node->Test());
  json["IfTrue"] = Visit(node->IfTrue());
  json["IfFalse"] = Visit(node->IfFalse());
  json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());
  return json;
}

Json ExpressionJsonSerializer::VisitUnary(const UnaryExpression *node) {
  Json json;
  json["NodeType"] = magic_enum::enum_name<ExpressionType>(node->NodeType());
  json["Operand"] = Visit(node->Operand());
  json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());
  return json;
}

Json ExpressionJsonSerializer::VisitCall(const CallExpression *node) {
  Json json;
  json["NodeType"] = magic_enum::enum_name<ExpressionType>(node->NodeType());
  json["ReturnType"] = Visit(node->Function());
  json["Arguments"] = Json::array();
  for (const auto &arg : node->Arguments()) {
    json["Arguments"].push_back(Visit(arg));
  }
  json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());
  return json;
}

Json ExpressionJsonSerializer::VisitLambda(const LambdaExpression *node) {
  Json json;
  json["NodeType"] = magic_enum::enum_name<ExpressionType>(node->NodeType());
  json["Name"] = Utility::UTF32ToUTF8(node->Name());
  json["Parameters"] = Json::array();
  for (const auto &param : node->Parameters()) {
    json["Parameters"].push_back(Visit(param));
  }
  json["Body"] = Visit(node->Body());
  json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());
  return json;
}

Json ExpressionJsonSerializer::VisitLoop(const LoopExpression *node) {
  Json json;

  json["NodeType"] = magic_enum::enum_name<ExpressionType>(node->NodeType());
  json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());

  json["Initializer"] = Visit(node->Initializer());
  json["Condition"] = Visit(node->Condition());
  json["Body"] = Visit(node->Body());

  return json;
}

Json ExpressionJsonSerializer::VisitDefault(const DefaultExpression *node) {
  Json json;

  json["NodeType"] = magic_enum::enum_name<ExpressionType>(node->NodeType());
  json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());
  json["Type"] =
      magic_enum::enum_name<TypeCode>(node->GetType()->GetTypeCode());

  return json;
}

Json ExpressionJsonSerializer::VisitVariableDeclaration(
    const VariableDeclarationExpression *node) {
  Json json;

  json["NodeType"] = magic_enum::enum_name<ExpressionType>(node->NodeType());
  json["SourceRange"] = SourceRangeToJson(node->GetSourceRange());
  json["Name"] = Utility::UTF32ToUTF8(node->Name());
  json["Initializer"] = Visit(node->Initializer());

  return json;
}

Json ExpressionJsonSerializer::SourceRangeToJson(
    const SourceRange &sourceRange) {
  Json json;

  json["FileName"] = sourceRange.CodeFile()->FileName();
  json["StartLine"] = sourceRange.StartLine();
  json["StartColumn"] = sourceRange.StartColumn();
  json["EndLine"] = sourceRange.EndLine();
  json["EndColumn"] = sourceRange.EndColumn();

  return json;
}

}; /* namespace Visitors */
}; /* namespace Cygni */