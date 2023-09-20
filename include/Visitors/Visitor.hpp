#ifndef CYGNI_VISITORS_VISITOR_HPP
#define CYGNI_VISITORS_VISITOR_HPP

#include "Expressions/TreeException.hpp"
#include "Expressions/Expression.hpp"

namespace Cygni {
namespace Visitors {

using namespace Expressions;

template <typename ReturnType, typename... ArgTypes> class ExpressionVisitor {
public:
  virtual ReturnType Visit(const Expression *node, ArgTypes... arguments) {
    switch (node->NodeType()) {
    case ExpressionType::Add:
    case ExpressionType::Subtract:
    case ExpressionType::Multiply:
    case ExpressionType::Divide:
    case ExpressionType::Modulo:
    case ExpressionType::GreaterThan:
    case ExpressionType::LessThan:
    case ExpressionType::GreaterThanOrEqual:
    case ExpressionType::LessThanOrEqual:
    case ExpressionType::Equal:
    case ExpressionType::NotEqual:
    case ExpressionType::And:
    case ExpressionType::Or:
    case ExpressionType::Assign:
      return VisitBinary(static_cast<const BinaryExpression *>(node),
                         arguments...);
    case ExpressionType::Not:
    case ExpressionType::Convert:
    case ExpressionType::Halt:
      return VisitUnary(static_cast<const UnaryExpression *>(node),
                        arguments...);
    case ExpressionType::Constant:
      return VisitConstant(static_cast<const ConstantExpression *>(node),
                           arguments...);
    case ExpressionType::Parameter:
      return VisitParameter(static_cast<const ParameterExpression *>(node),
                            arguments...);
    case ExpressionType::Block:
      return VisitBlock(static_cast<const BlockExpression *>(node),
                        arguments...);
    case ExpressionType::Conditional:
      return VisitConditional(static_cast<const ConditionalExpression *>(node),
                              arguments...);
    case ExpressionType::Call:
      return VisitCall(static_cast<const CallExpression *>(node), arguments...);
    case ExpressionType::Lambda:
      return VisitLambda(static_cast<const LambdaExpression *>(node),
                         arguments...);
    case ExpressionType::Loop:
      return VisitLoop(static_cast<const LoopExpression *>(node), arguments...);
    case ExpressionType::Default:
      return VisitDefault(static_cast<const DefaultExpression *>(node),
                          arguments...);
    case ExpressionType::VariableDeclaration:
      return VisitVariableDeclaration(
          static_cast<const VariableDeclarationExpression *>(node),
          arguments...);
    default:
      throw TreeException(__FILE__, __LINE__,
                          "The node type is not supported by the visitor.",
                          node, nullptr);
    }
  }
  virtual ReturnType VisitBinary(const BinaryExpression *node,
                                 ArgTypes... arguments) = 0;
  virtual ReturnType VisitUnary(const UnaryExpression *node,
                                ArgTypes... arguments) = 0;
  virtual ReturnType VisitConstant(const ConstantExpression *node,
                                   ArgTypes... arguments) = 0;
  virtual ReturnType VisitParameter(const ParameterExpression *node,
                                    ArgTypes... arguments) = 0;
  virtual ReturnType VisitBlock(const BlockExpression *node,
                                ArgTypes... arguments) = 0;
  virtual ReturnType VisitConditional(const ConditionalExpression *node,
                                      ArgTypes... arguments) = 0;
  virtual ReturnType VisitCall(const CallExpression *node,
                               ArgTypes... arguments) = 0;
  virtual ReturnType VisitLambda(const LambdaExpression *node,
                                 ArgTypes... arguments) = 0;
  virtual ReturnType VisitLoop(const LoopExpression *node,
                               ArgTypes... arguments) = 0;
  virtual ReturnType VisitDefault(const DefaultExpression *node,
                                  ArgTypes... arguments) = 0;
  virtual ReturnType
  VisitVariableDeclaration(const VariableDeclarationExpression *node,
                           ArgTypes... arguments) = 0;
};

}; /* namespace Visitors */
}; /* namespace Cygni */

#endif /* CYGNI_VISITORS_VISITOR_HPP */