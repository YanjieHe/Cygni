#ifndef CYGNI_VISITORS_NAME_LOCATOR_HPP
#define CYGNI_VISITORS_NAME_LOCATOR_HPP

#include "Visitors/Visitor.hpp"
#include "Visitors/Scope.hpp"
#include "Expressions/Namespace.hpp"
#include "Utility/HashPair.hpp"
#include <stack>

namespace Cygni {
namespace Visitors {

enum class LocationKind {
  GlobalVariable,
  FunctionVariable,
  FunctionConstant,
  FunctionVariableCount,
  FunctionConstantCount,
  Function,
  NativeFunction,
  Global_Variable_Count,
  Global_Function_Count,
  Global_Native_Function_Count
};

class NameInfo {
private:
  LocationKind kind;
  int number;

public:
  NameInfo() = default;
  NameInfo(LocationKind kind, int number) : kind{kind}, number{number} {}

  LocationKind Kind() const { return kind; }
  int Number() const { return number; }
  int &Number() { return number; }
};

class NameLocator : public ExpressionVisitor<void, Scope<NameInfo> *> {
private:
  std::unordered_map<std::pair<const Expression *, LocationKind>, NameInfo,
                     Utility::HashPair> nameInfoTable;
  NamespaceFactory &namespaceFactory;
  std::stack<Namespace *> namespaceStack;

public:
  NameLocator(NamespaceFactory &namespaceFactory);
  const std::unordered_map<std::pair<const Expression *, LocationKind>,
                           NameInfo, Utility::HashPair> &
  NameInfoTable() {
    return nameInfoTable;
  }
  const NameInfo &GetNameInfo(const Expression *node,
                              LocationKind locationKind) const {
    return nameInfoTable.at({node, locationKind});
  }
  bool ExistsNameInfo(const Expression *node, LocationKind locationKind) const {
    return nameInfoTable.count({node, locationKind}) > 0;
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
  void CheckNamespace(Scope<NameInfo> *parent);

private:
  void Register(const Expression *node, const NameInfo &nameInfo);
};

const std::u32string LOCAL_CONSTANT_COUNT = U"$LOCAL_CONSTANT_COUNT";
const std::u32string LOCAL_VARIABLE_COUNT = U"$LOCAL_VARIABLE_COUNT";
const std::u32string GLOBAL_NATIVE_FUNCTION_COUNT =
    U"$GLOBAL_NATIVE_FUNCTION_COUNT";
const std::u32string GLOBAL_FUNCTION_COUNT = U"$GLOBAL_FUNCTION_COUNT";
const std::u32string GLOBAL_VARIABLE_COUNT = U"$GLOBAL_VARIABLE_COUNT";
}; /* namespace Visitors */
}; /* namespace Cygni */

#endif /* CYGNI_VISITORS_NAME_LOCATOR_HPP */