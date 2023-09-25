#include "Visitors/NameLocator.hpp"

namespace Cygni {
namespace Visitors {

void NameLocator::VisitBinary(const BinaryExpression *node,
                              Scope<NameInfo> *scope) {
  Visit(node->Left(), scope);
  Visit(node->Right(), scope);
}
void NameLocator::VisitUnary(const UnaryExpression *node,
                             Scope<NameInfo> *scope) {
  Visit(node->Operand(), scope);
}
void NameLocator::VisitConstant(const ConstantExpression *node,
                                Scope<NameInfo> *scope) {
  /* TODO: support duplicated constants. */
  NameInfo nameInfo(LocationKind::FunctionConstant,
                    scope->Get(U"$LOCATION_CONSTANT_COUNT").number);
  nameInfoTable.insert({static_cast<const Expression *>(node), nameInfo});
  scope->Get(U"$LOCATION_CONSTANT_COUNT").number++;
}
void NameLocator::VisitParameter(const ParameterExpression *node,
                                 Scope<NameInfo> *scope) {
  NameInfo nameInfo = scope->Get(node->Name());
  nameInfoTable.insert({static_cast<const Expression *>(node), nameInfo});
}
void NameLocator::VisitBlock(const BlockExpression *node,
                             Scope<NameInfo> *parent) {
  Scope<NameInfo> scope(parent);
  for (const auto &exp : node->Expressions()) {
    Visit(exp, &scope);
  }
}
void NameLocator::VisitConditional(const ConditionalExpression *node,
                                   Scope<NameInfo> *scope) {
  Visit(node->Test(), scope);
  Visit(node->IfTrue(), scope);
  Visit(node->IfFalse(), scope);
}
void NameLocator::VisitCall(const CallExpression *node,
                            Scope<NameInfo> *scope) {
  Visit(node->Function(), scope);
  for (const auto &arg : node->Arguments()) {
    Visit(arg, scope);
  }
}
void NameLocator::VisitLambda(const LambdaExpression *node,
                              Scope<NameInfo> *parent) {
  Scope<NameInfo> scope(parent);
  scope.Declare(U"$LOCAL_VARIABLE_COUNT",
                NameInfo(LocationKind::FunctionVariableCount, 0));
  scope.Declare(U"$LOCAL_CONSTANT_COUNT",
                NameInfo(LocationKind::FunctionConstantCount, 0));
  for (const auto &parameter : node->Parameters()) {
    scope.Declare(parameter->Name(),
                  NameInfo(LocationKind::FunctionVariable,
                           scope.Get(U"$LOCAL_VARIABLE_COUNT").number));
    scope.Get(U"$LOCAL_VARIABLE_COUNT").number++;
  }
  Visit(node->Body(), &scope);
}
void NameLocator::VisitLoop(const LoopExpression *node,
                            Scope<NameInfo> *parent) {
  Scope<NameInfo> scope(parent);
  Visit(node->Initializer(), &scope);
  Visit(node->Condition(), &scope);
  Visit(node->Body(), &scope);
}
void NameLocator::VisitDefault(const DefaultExpression *node,
                               Scope<NameInfo> *scope) {}
void NameLocator::VisitVariableDeclaration(
    const VariableDeclarationExpression *node, Scope<NameInfo> *scope) {
  scope->Declare(node->Name(),
                 NameInfo(LocationKind::FunctionVariable,
                          scope->Get(U"$LOCAL_VARIABLE_COUNT").number));
  scope->Get(U"$LOCAL_VARIABLE_COUNT").number++;
  Visit(node->Initializer(), scope);
}
}; /* namespace Visitors */
}; /* namespace Cygni */
