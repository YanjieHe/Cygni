#include "Visitors/NameLocator.hpp"

namespace Cygni {
namespace Visitors {

NameLocator::NameLocator(NamespaceFactory &namespaceFactory)
    : namespaceFactory{namespaceFactory} {
  namespaceStack.push(namespaceFactory.GetRoot());
}
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
                    scope->Get(LOCAL_CONSTANT_COUNT).Number());
  Register(node, nameInfo);
  scope->Get(LOCAL_CONSTANT_COUNT).Number()++;
}
void NameLocator::VisitParameter(const ParameterExpression *node,
                                 Scope<NameInfo> *scope) {
  if (node->Prefix().empty()) {
    NameInfo nameInfo = scope->Get(node->Name());
    Register(node, nameInfo);
  } else {
    Namespace *ns =
        namespaceFactory.Search(namespaceFactory.GetRoot(), node->Prefix());
    if (ns) {
      if (ns->GlobalVariables().ContainsKey(node->Name())) {
        const NameInfo &nameInfo =
            GetNameInfo(ns->GlobalVariables().GetItemByKey(node->Name()),
                        LocationKind::GlobalVariable);

        return Register(node, nameInfo);
      } else if (ns->Functions().ContainsKey(node->Name())) {
        const NameInfo &nameInfo = GetNameInfo(
            ns->Functions().GetItemByKey(node->Name()), LocationKind::Function);

        return Register(node, nameInfo);
      } else {
        throw TreeException(
            __FILE__, __LINE__,
            Utility::UTF32ToUTF8(U"'" + node->Name() + U"' not defined."), node,
            nullptr);
      }
    }
  }
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
  scope.Declare(LOCAL_VARIABLE_COUNT,
                NameInfo(LocationKind::FunctionVariableCount, 0));
  scope.Declare(LOCAL_CONSTANT_COUNT,
                NameInfo(LocationKind::FunctionConstantCount, 0));
  for (const auto &parameter : node->Parameters()) {
    scope.Declare(parameter->Name(),
                  NameInfo(LocationKind::FunctionVariable,
                           scope.Get(LOCAL_VARIABLE_COUNT).Number()));
    scope.Get(LOCAL_VARIABLE_COUNT).Number()++;
  }
  Visit(node->Body(), &scope);

  Register(node, NameInfo(LocationKind::FunctionVariableCount,
                          scope.Get(LOCAL_VARIABLE_COUNT).Number()));
  Register(node, NameInfo(LocationKind::FunctionConstantCount,
                          scope.Get(LOCAL_CONSTANT_COUNT).Number()));
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
  NameInfo nameInfo(LocationKind::FunctionVariable,
                    scope->Get(LOCAL_VARIABLE_COUNT).Number());
  scope->Declare(node->Name(), nameInfo);
  Register(node, nameInfo);
  scope->Get(LOCAL_VARIABLE_COUNT).Number()++;
  Visit(node->Initializer(), scope);
}
void NameLocator::CheckNamespace(Scope<NameInfo> *parent) {
  Namespace *top = namespaceStack.top();
  Scope<NameInfo> *scope(parent);

  for (const auto &varDecl : top->GlobalVariables().GetAllItems()) {
    NameInfo nameInfo(LocationKind::GlobalVariable,
                      scope->Get(GLOBAL_VARIABLE_COUNT).Number());
    scope->Declare(varDecl->Name(), nameInfo);
    Register(varDecl, nameInfo);
    scope->Get(GLOBAL_VARIABLE_COUNT).Number()++;
  }
  for (const auto &funcDecl : top->Functions().GetAllItems()) {
    if (funcDecl->IsNativeFunction()) {
      NameInfo nameInfo(LocationKind::NativeFunction,
                        scope->Get(GLOBAL_NATIVE_FUNCTION_COUNT).Number());
      scope->Declare(funcDecl->Name(), nameInfo);
      Register(funcDecl, nameInfo);
      scope->Get(GLOBAL_NATIVE_FUNCTION_COUNT).Number()++;

    } else {
      NameInfo nameInfo(LocationKind::Function,
                        scope->Get(GLOBAL_FUNCTION_COUNT).Number());
      scope->Declare(funcDecl->Name(), nameInfo);
      Register(funcDecl, nameInfo);
      scope->Get(GLOBAL_FUNCTION_COUNT).Number()++;
    }
  }

  for (const auto &varDecl : top->GlobalVariables().GetAllItems()) {
    VisitVariableDeclaration(varDecl, scope);
  }
  for (const auto &funcDecl : top->Functions().GetAllItems()) {
    VisitLambda(funcDecl, scope);
  }

  for (const auto &current : top->Children().GetAllItems()) {
    namespaceStack.push(current);
    CheckNamespace(scope);
    namespaceStack.pop();
  }
}

void NameLocator::Register(const Expression *node, const NameInfo &nameInfo) {
  nameInfoTable.insert({{node, nameInfo.Kind()}, nameInfo});
}

}; /* namespace Visitors */
}; /* namespace Cygni */
