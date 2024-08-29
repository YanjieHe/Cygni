#ifndef CYGNI_VISITORS_EXPRESSION_JSON_SERIALIZER_HPP
#define CYGNI_VISITORS_EXPRESSION_JSON_SERIALIZER_HPP

#include "Visitors/Visitor.hpp"
#include <nlohmann/json.hpp>

namespace Cygni
{
namespace Visitors
{

using namespace Expressions;
using Json = nlohmann::json;

class ExpressionJsonSerializer : public ExpressionVisitor<Json>
{
  public:
    Json VisitBinary(const BinaryExpression *node) override;
    Json VisitConstant(const ConstantExpression *node) override;
    Json VisitParameter(const ParameterExpression *node) override;
    Json VisitBlock(const BlockExpression *node) override;
    Json VisitConditional(const ConditionalExpression *node) override;
    Json VisitUnary(const UnaryExpression *node) override;
    Json VisitCall(const CallExpression *node) override;
    Json VisitLambda(const LambdaExpression *node) override;
    Json VisitWhileLoop(const WhileLoopExpression *node) override;
    Json VisitDefault(const DefaultExpression *node) override;
    Json VisitVariableDeclaration(const VariableDeclarationExpression *node) override;
    Json VisitNew(const NewExpression *node) override;
    Json VisitMember(const MemberExpression* node) override;

    static Json SourceRangeToJson(const SourceRange &sourceRange);
    Json FieldInitializationToJson(const std::u32string &fieldName, const Expression *value);
    static Json ExpressionToJson(const Expression* node);
};

}; /* namespace Visitors */
}; /* namespace Cygni */

#endif /* CYGNI_VISITORS_EXPRESSION_JSON_SERIALIZER_HPP */