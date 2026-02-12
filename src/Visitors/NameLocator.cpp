#include "Visitors/NameLocator.hpp"
#include "Utility/StringUtils.hpp"
#include <spdlog/spdlog.h>

namespace Cygni
{
namespace Visitors
{

NameLocator::NameLocator(NamespaceFactory &namespaceFactory) : namespaceFactory{namespaceFactory}
{
    namespaceStack.push(namespaceFactory.GetRoot());
}
void NameLocator::VisitBinary(const BinaryExpression *node, Scope<NameInfo> *scope)
{
    Visit(node->Left(), scope);
    Visit(node->Right(), scope);
}
void NameLocator::VisitUnary(const UnaryExpression *node, Scope<NameInfo> *scope)
{
    Visit(node->Operand(), scope);
}
void NameLocator::VisitConstant(const ConstantExpression *node, Scope<NameInfo> *scope)
{
    /* TODO: support duplicated constants. */
    NameInfo nameInfo(LocationKind::FunctionConstant, scope->Get(LOCAL_CONSTANT_COUNT).Number());
    Register(node, nameInfo);
    scope->Get(LOCAL_CONSTANT_COUNT).Number()++;
}
void NameLocator::VisitParameter(const ParameterExpression *node, Scope<NameInfo> *scope)
{
    if (node->QualifiedName().size() == 1)
    {
        NameInfo nameInfo = scope->Get(node->Name());
        Register(node, nameInfo);
    }
    else
    {
        VariableDeclarationExpression *varDecl =
            namespaceFactory.SearchGlobalVariable(namespaceFactory.GetRoot(), node->QualifiedName());
        if (varDecl != nullptr)
        {
            const NameInfo &nameInfo = GetNameInfo(varDecl, LocationKind::GlobalVariable);

            return Register(node, nameInfo);
        }
        LambdaExpression *funcDecl = namespaceFactory.SearchFunction(namespaceFactory.GetRoot(), node->QualifiedName());
        if (funcDecl != nullptr)
        {
            if (ExistsNameInfo(funcDecl, LocationKind::Function))
            {
                const NameInfo &nameInfo = GetNameInfo(funcDecl, LocationKind::Function);

                Register(node, nameInfo);
            }
            else if (ExistsNameInfo(funcDecl, LocationKind::NativeFunction))
            {
                const NameInfo &nameInfo = GetNameInfo(funcDecl, LocationKind::NativeFunction);

                Register(node, nameInfo);
            }
            else
            {
                spdlog::error("'{}' is not defined.",
                              Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", node->QualifiedName())));

                throw TreeException(__FILE__, __LINE__,
                                    Utility::UTF32ToUTF8(U"'" +
                                                         Utility::StringUtils::Join(U"::", node->QualifiedName()) +
                                                         U"' is not defined."),
                                    node, nullptr);
            }
        }
        else
        {
            spdlog::error("'{}' is not defined.",
                          Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", node->QualifiedName())));

            throw TreeException(__FILE__, __LINE__,
                                Utility::UTF32ToUTF8(U"'" + Utility::StringUtils::Join(U"::", node->QualifiedName()) +
                                                     U"' is not defined."),
                                node, nullptr);
        }
    }
}
void NameLocator::VisitBlock(const BlockExpression *node, Scope<NameInfo> *parent)
{
    Scope<NameInfo> scope(parent);
    for (const auto &exp : node->Expressions())
    {
        Visit(exp, &scope);
    }
}
void NameLocator::VisitConditional(const ConditionalExpression *node, Scope<NameInfo> *scope)
{
    Visit(node->Test(), scope);
    Visit(node->IfTrue(), scope);
    Visit(node->IfFalse(), scope);
}
void NameLocator::VisitCall(const CallExpression *node, Scope<NameInfo> *scope)
{
    Visit(node->Function(), scope);
    for (const auto &arg : node->Arguments())
    {
        Visit(arg, scope);
    }
}
void NameLocator::VisitLambda(const LambdaExpression *node, Scope<NameInfo> *parent)
{
    Scope<NameInfo> scope(parent);
    scope.Declare(LOCAL_VARIABLE_COUNT, NameInfo(LocationKind::FunctionVariableCount, 0));
    scope.Declare(LOCAL_CONSTANT_COUNT, NameInfo(LocationKind::FunctionConstantCount, 0));
    scope.Declare(FUNCTION_PARAMETER_COUNT, NameInfo(LocationKind::FunctionParameterCount, node->Parameters().size()));
    for (const auto &parameter : node->Parameters())
    {
        scope.Declare(parameter->Name(),
                      NameInfo(LocationKind::FunctionVariable, scope.Get(LOCAL_VARIABLE_COUNT).Number()));
        scope.Get(LOCAL_VARIABLE_COUNT).Number()++;
    }
    Visit(node->Body(), &scope);

    Register(node, NameInfo(LocationKind::FunctionVariableCount, scope.Get(LOCAL_VARIABLE_COUNT).Number()));
    Register(node, NameInfo(LocationKind::FunctionConstantCount, scope.Get(LOCAL_CONSTANT_COUNT).Number()));
}
void NameLocator::VisitWhileLoop(const WhileLoopExpression *node, Scope<NameInfo> *parent)
{
    Scope<NameInfo> scope(parent);
    Visit(node->Condition(), &scope);
    Visit(node->Body(), &scope);
}
void NameLocator::VisitDefault(const DefaultExpression *node, Scope<NameInfo> *scope)
{
}
void NameLocator::VisitVariableDeclaration(const VariableDeclarationExpression *node, Scope<NameInfo> *scope)
{
    NameInfo nameInfo(LocationKind::FunctionVariable, scope->Get(LOCAL_VARIABLE_COUNT).Number());
    scope->Declare(node->Name(), nameInfo);
    Register(node, nameInfo);
    scope->Get(LOCAL_VARIABLE_COUNT).Number()++;
    Visit(node->Initializer(), scope);
}
void NameLocator::CheckNamespace(Scope<NameInfo> *parent)
{
    Namespace *top = namespaceStack.top();
    Scope<NameInfo> *scope(parent);

    for (const auto &funcDecl : top->Functions().GetAllItems())
    {
        VisitLambda(funcDecl, scope);
    }

    for (const auto &structDecl : top->Structures().GetAllItems())
    {
        for (const auto &method : structDecl->Methods().GetAllItems())
        {
            VisitMethod(method, scope);
        }
    }

    for (const auto &current : top->Children().GetAllItems())
    {
        namespaceStack.push(current);
        CheckNamespace(scope);
        namespaceStack.pop();
    }
}
void NameLocator::RegisterGlobalVariable(const VariableDeclarationExpression *node, Scope<NameInfo> *scope)
{
    NameInfo nameInfo(LocationKind::GlobalVariable, scope->Get(GLOBAL_VARIABLE_COUNT).Number());
    scope->Declare(node->Name(), nameInfo);
    Register(node, nameInfo);
    scope->Get(GLOBAL_VARIABLE_COUNT).Number()++;
}
void NameLocator::RegisterFunction(const LambdaExpression *node, Scope<NameInfo> *scope)
{
    if (node->IsNativeFunction())
    {
        NameInfo nameInfo(LocationKind::NativeFunction, scope->Get(GLOBAL_NATIVE_FUNCTION_COUNT).Number());
        scope->Declare(node->Name(), nameInfo);
        Register(node, nameInfo);
        scope->Get(GLOBAL_NATIVE_FUNCTION_COUNT).Number()++;
    }
    else
    {
        NameInfo nameInfo(LocationKind::Function, scope->Get(GLOBAL_FUNCTION_COUNT).Number());
        scope->Declare(node->Name(), nameInfo);
        Register(node, nameInfo);
        scope->Get(GLOBAL_FUNCTION_COUNT).Number()++;
    }
}
void NameLocator::RegisterStructure(const StructureExpression *node, Scope<NameInfo> *parent)
{
    NameInfo &nameInfo = parent->Get(GLOBAL_STRUCTURE_COUNT);
    Register(node, NameInfo(LocationKind::Structure, nameInfo.Number()));
    nameInfo.Number()++;

    Scope<NameInfo> scope(parent);
    for (const auto &method : node->Methods().GetAllItems())
    {
        RegisterFunction(method, &scope);
    }
}
void NameLocator::RegisterInterface(const InterfaceExpression *node, Scope<NameInfo> *parent)
{
    NameInfo &nameInfo = parent->Get(GLOBAL_INTERFACE_COUNT);
    Register(node, NameInfo(LocationKind::Interface, nameInfo.Number()));
    nameInfo.Number()++;
}
void NameLocator::RegisterAllInfo(Scope<NameInfo> *scope)
{
    std::stack<Namespace *> stack;
    stack.push(namespaceFactory.GetRoot());
    while (!stack.empty())
    {
        Namespace *top = stack.top();
        stack.pop();
        for (Namespace *child : top->Children().GetAllItems())
        {
            stack.push(child);
        }
        for (VariableDeclarationExpression *varDecl : top->GlobalVariables().GetAllItems())
        {
            RegisterGlobalVariable(varDecl, scope);
        }
        for (LambdaExpression *funcDecl : top->Functions().GetAllItems())
        {
            RegisterFunction(funcDecl, scope);
        }
        for (StructureExpression *structureDefinition : top->Structures().GetAllItems())
        {
            RegisterStructure(structureDefinition, scope);
        }
        for (InterfaceExpression *interfaceDefinition : top->Interfaces().GetAllItems())
        {
            RegisterInterface(interfaceDefinition, scope);
        }
    }
}
void NameLocator::InitializeSymbolCounters(Scope<NameInfo> *scope)
{
    scope->Declare(GLOBAL_VARIABLE_COUNT, NameInfo(LocationKind::GlobalVariableCount, 0));
    scope->Declare(GLOBAL_FUNCTION_COUNT, NameInfo(LocationKind::GlobalFunctionCount, 0));
    scope->Declare(GLOBAL_NATIVE_FUNCTION_COUNT, NameInfo(LocationKind::GlobalNativeFunctionCount, 0));
    scope->Declare(GLOBAL_STRUCTURE_COUNT, NameInfo(LocationKind::GlobalStructureCount, 0));
    scope->Declare(GLOBAL_INTERFACE_COUNT, NameInfo(LocationKind::GlobalInterfaceCount, 0));
}
void NameLocator::VisitNew(const NewExpression *node, Scope<NameInfo> *scope)
{
    const TypeSyntax *typeSyntax = node->GetTypeSyntax();
    if (typeSyntax == nullptr)
    {
        spdlog::error("The new expression is missing a type annotation.");
        throw TreeException(__FILE__, __LINE__, "The new expression should initialize a structure.", node, nullptr);
    }

    Namespace *top = namespaceStack.top();
    StructureExpression *structureDefinition = namespaceFactory.SearchStructure(top, typeSyntax->QualifiedName());
    if (structureDefinition != nullptr)
    {
        const NameInfo &nameInfo = GetNameInfo(structureDefinition, LocationKind::Structure);
        Register(node, NameInfo(LocationKind::Structure, nameInfo.Number()));
        for (Expression *value : node->FieldsInitialization().GetAllItems())
        {
            Visit(value, scope);
        }
    }
    else
    {
        spdlog::error("The structure being initialized is not defined.");

        throw TreeException(__FILE__, __LINE__, "The structure being initialized is not defined.", node, nullptr);
    }
}

void NameLocator::VisitMember(const MemberExpression *node, Scope<NameInfo> *scope)
{
    Visit(node->GetExpression(), scope);
}

void NameLocator::Register(const Expression *node, const NameInfo &nameInfo)
{
    nameInfoTable.insert({{node, nameInfo.Kind()}, nameInfo});
}

void NameLocator::VisitMethod(const LambdaExpression *node, Scope<NameInfo> *parent)
{
    Scope<NameInfo> scope(parent);
    scope.Declare(LOCAL_VARIABLE_COUNT, NameInfo(LocationKind::FunctionVariableCount, 1)); /* 1 for 'this' */
    scope.Declare(U"this", NameInfo(LocationKind::FunctionVariable, scope.Get(LOCAL_VARIABLE_COUNT).Number()));
    scope.Declare(LOCAL_CONSTANT_COUNT, NameInfo(LocationKind::FunctionConstantCount, 0));
    scope.Declare(FUNCTION_PARAMETER_COUNT,
                  NameInfo(LocationKind::FunctionParameterCount, node->Parameters().size() + 1)); /* +1 for 'this' */

    for (const auto &parameter : node->Parameters())
    {
        scope.Declare(parameter->Name(),
                      NameInfo(LocationKind::FunctionVariable, scope.Get(LOCAL_VARIABLE_COUNT).Number()));
        scope.Get(LOCAL_VARIABLE_COUNT).Number()++;
    }
    Visit(node->Body(), &scope);

    Register(node, NameInfo(LocationKind::FunctionVariableCount, scope.Get(LOCAL_VARIABLE_COUNT).Number()));
    Register(node, NameInfo(LocationKind::FunctionConstantCount, scope.Get(LOCAL_CONSTANT_COUNT).Number()));
}

}; /* namespace Visitors */
}; /* namespace Cygni */
