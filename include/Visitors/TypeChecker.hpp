#ifndef CYGNI_VISITORS_TYPE_CHECKER_HPP
#define CYGNI_VISITORS_TYPE_CHECKER_HPP

#include "Visitors/Visitor.hpp"
#include "Visitors/Scope.hpp"
#include "Expressions/Type.hpp"
#include "Expressions/Namespace.hpp"
#include <stack>

namespace Cygni {
namespace Visitors {

class TypeChecker
    : public ExpressionVisitor<const Type *, Scope<const Type *> *> {
private:
  std::unordered_map<const Expression *, const Type *> nodeTypes;
  TypeFactory Types;
  NamespaceFactory &namespaceFactory;
  std::stack<Namespace *> namespaceStack;

public:
  TypeChecker(NamespaceFactory &namespaceFactory);

  const Type *VisitBinary(const BinaryExpression *node,
                          Scope<const Type *> *scope) override;
  const Type *VisitConstant(const ConstantExpression *node,
                            Scope<const Type *> *scope) override;
  const Type *VisitParameter(const ParameterExpression *node,
                             Scope<const Type *> *scope) override;
  const Type *VisitBlock(const BlockExpression *node,
                         Scope<const Type *> *parent) override;
  const Type *VisitConditional(const ConditionalExpression *node,
                               Scope<const Type *> *scope) override;
  const Type *VisitUnary(const UnaryExpression *node,
                         Scope<const Type *> *scope) override;
  const Type *VisitCall(const CallExpression *node,
                        Scope<const Type *> *scope) override;
  const Type *VisitLambda(const LambdaExpression *node,
                          Scope<const Type *> *parent) override;
  const Type *VisitLoop(const LoopExpression *node,
                        Scope<const Type *> *parent) override;
  const Type *VisitDefault(const DefaultExpression *node,
                           Scope<const Type *> *scope) override;
  const Type *
  VisitVariableDeclaration(const VariableDeclarationExpression *node,
                           Scope<const Type *> *scope) override;

  const Type *GetType(const Expression *node);

  void CheckNamespace(Scope<const Type *> *parent);

private:
  const Type *Register(const Expression *node, const Type *type);
  bool CheckFunctionType(const Type* declaration, const Type* actual);
};

}; /* namespace Visitors */
}; /* namespace Cygni */

#endif /* CYGNI_VISITORS_TYPE_CHECKER_HPP */