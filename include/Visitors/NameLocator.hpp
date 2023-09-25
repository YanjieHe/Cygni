#ifndef CYGNI_VISITORS_NAME_LOCATOR_HPP
#define CYGNI_VISITORS_NAME_LOCATOR_HPP

#include "Visitors/Visitor.hpp"
#include "Visitors/Scope.hpp"

namespace Cygni {
namespace Visitors {

enum class LocationKind {
  Global,
  FunctionVariable,
  FunctionConstant,
  FunctionVariableCount,
  FunctionConstantCount,
  Function,
};

class NameInfo {
public:
  LocationKind kind;
  int number;

  NameInfo() = default;
  NameInfo(LocationKind kind, int number) : kind{kind}, number{number} {}
};

class NameLocator : public ExpressionVisitor<void, Scope<NameInfo> *> {
private:
  std::unordered_map<const Expression *, NameInfo> nameInfoTable;

public:
  const std::unordered_map<const Expression *, NameInfo> &NameInfoTable() {
    return nameInfoTable;
  }

  void VisitBinary(const BinaryExpression *node,
                   Scope<NameInfo> *scope) override;
  void VisitUnary(const UnaryExpression *node, Scope<NameInfo> *scope) override;
  void VisitConstant(const ConstantExpression *node,
                     Scope<NameInfo> *scope) override;
  void VisitParameter(const ParameterExpression *node,
                      Scope<NameInfo> *scope) override;
  void VisitBlock(const BlockExpression *node,
                  Scope<NameInfo> *parent) override;
  void VisitConditional(const ConditionalExpression *node,
                        Scope<NameInfo> *scope) override;
  void VisitCall(const CallExpression *node, Scope<NameInfo> *scope) override;
  void VisitLambda(const LambdaExpression *node,
                   Scope<NameInfo> *parent) override;
  void VisitLoop(const LoopExpression *node, Scope<NameInfo> *parent) override;
  void VisitDefault(const DefaultExpression *node,
                    Scope<NameInfo> *scope) override;
  void VisitVariableDeclaration(const VariableDeclarationExpression *node,
                                Scope<NameInfo> *scope) override;
};

}; /* namespace Visitors */
}; /* namespace Cygni */

#endif /* CYGNI_VISITORS_NAME_LOCATOR_HPP */