#ifndef CYGNI_VISITORS_TYPE_CHECKER_HPP
#define CYGNI_VISITORS_TYPE_CHECKER_HPP

#include "Expressions/Namespace.hpp"
#include "Expressions/Type.hpp"
#include "Visitors/Scope.hpp"
#include "Visitors/Visitor.hpp"
#include <stack>

namespace Cygni
{
namespace Visitors
{

class TypeChecker : public ExpressionVisitor<const Type *, Scope<const Type *> *>
{
  private:
    std::unordered_map<const Expression *, const Type *> nodeTypes;
    std::unordered_map<const StructureExpression *, StructureType *> structureTypeCache;
    std::unordered_map<const InterfaceExpression *, InterfaceType *> interfaceTypeCache;
    TypeFactory Types;
    NamespaceFactory &namespaceFactory;
    ExpressionFactory &expressionFactory;
    std::stack<Namespace *> namespaceStack;

  public:
    TypeChecker(NamespaceFactory &namespaceFactory, ExpressionFactory &expressionFactory);

    const Type *VisitBinary(const BinaryExpression *node, Scope<const Type *> *scope) override;
    const Type *VisitConstant(const ConstantExpression *node, Scope<const Type *> *scope) override;
    const Type *VisitParameter(const ParameterExpression *node, Scope<const Type *> *scope) override;
    const Type *VisitBlock(const BlockExpression *node, Scope<const Type *> *parent) override;
    const Type *VisitConditional(const ConditionalExpression *node, Scope<const Type *> *scope) override;
    const Type *VisitUnary(const UnaryExpression *node, Scope<const Type *> *scope) override;
    const Type *VisitCall(const CallExpression *node, Scope<const Type *> *scope) override;
    const Type *VisitLambda(const LambdaExpression *node, Scope<const Type *> *parent) override;
    const Type *VisitWhileLoop(const WhileLoopExpression *node, Scope<const Type *> *parent) override;
    const Type *VisitDefault(const DefaultExpression *node, Scope<const Type *> *scope) override;
    const Type *VisitVariableDeclaration(const VariableDeclarationExpression *node,
                                         Scope<const Type *> *scope) override;
    const Type *VisitNew(const NewExpression *node, Scope<const Type *> *scope) override;
    const Type *VisitMember(const MemberExpression *node, Scope<const Type *> *scope) override;

    const Type *GetType(const Expression *node);

    void CheckNamespace(Scope<const Type *> *parent);

    void CheckGlobalVariable(const VariableDeclarationExpression *node, Scope<const Type *> *parent,
                             Namespace *current);

  private:
    const Type *Register(const Expression *node, const Type *type);
    bool CheckFunctionType(const Type *declaration, const Type *actual);
    const Type *ResolveTypeSyntax(const TypeSyntax *typeSyntax);
    StructureType *ResolveStructureDefinition(StructureExpression *structureDefinition);
    InterfaceType *ResolveInterfaceDefinition(InterfaceExpression *interfaceDefinition);
    CallableType *BuildCallableType(const LambdaExpression *node);
    const Type *CheckAssignment(const BinaryExpression *node, Scope<const Type *> *scope);
    const Type *VisitMethodCall(const CallExpression *node, Scope<const Type *> *scope);
    const Type *CheckArguments(const CallExpression *node, const Type *callableType, Scope<const Type *> *scope);
    void CheckInterfaceImplementation(const StructureExpression *node, const StructureType *structureType,
                                      const InterfaceType *interfaceType);
    std::vector<const InterfaceType *> GetAllImplementedInterfaces(const StructureType *structureType);
};

}; /* namespace Visitors */
}; /* namespace Cygni */

#endif /* CYGNI_VISITORS_TYPE_CHECKER_HPP */