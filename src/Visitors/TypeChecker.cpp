#include "Visitors/TypeChecker.hpp"

#include "Utility/Convert.hpp"
#include "Utility/StringUtils.hpp"
#include "Utility/UTF32Functions.hpp"
#include <spdlog/spdlog.h>
#include <unordered_set>

namespace Cygni
{
namespace Visitors
{

TypeChecker::TypeChecker(NamespaceFactory &namespaceFactory, ExpressionFactory &expressionFactory)
    : namespaceFactory{namespaceFactory}, expressionFactory{expressionFactory}
{
    namespaceStack.push(namespaceFactory.GetRoot());
    spdlog::debug("Type checker initialized.");
}

const Type *TypeChecker::VisitBinary(const BinaryExpression *node, Scope<const Type *> *scope)
{
    spdlog::debug("{}: {}", __FUNCTION__, Utility::EnumToString(node->NodeType()));
    if (node->NodeType() == ExpressionType::Assign)
    {
        return CheckAssignment(node, scope);
    }
    else
    {
        const Type *left = Visit(node->Left(), scope);
        const Type *right = Visit(node->Right(), scope);
        switch (node->NodeType())
        {
        case ExpressionType::Add:
        case ExpressionType::Subtract:
        case ExpressionType::Multiply:
        case ExpressionType::Divide: {
            if (left->GetTypeCode() == TypeCode::Int32 && right->GetTypeCode() == TypeCode::Int32)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Int32));
            }
            else if (left->GetTypeCode() == TypeCode::Int64 && right->GetTypeCode() == TypeCode::Int64)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Int64));
            }
            else if (left->GetTypeCode() == TypeCode::Float32 && right->GetTypeCode() == TypeCode::Float32)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Float32));
            }
            else if (left->GetTypeCode() == TypeCode::Float64 && right->GetTypeCode() == TypeCode::Float64)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Float64));
            }
            else
            {
                throw TreeException(__FILE__, __LINE__,
                                    "Arithmetic operation type mismatch: cannot apply '" +
                                        Utility::EnumToString(node->NodeType()) + "' to '" +
                                        Utility::EnumToString(left->GetTypeCode()) + "' and '" +
                                        Utility::EnumToString(right->GetTypeCode()) + "'.",
                                    node, nullptr);
            }
        }
        case ExpressionType::GreaterThan:
        case ExpressionType::LessThan:
        case ExpressionType::GreaterThanOrEqual:
        case ExpressionType::LessThanOrEqual: {
            if (left->GetTypeCode() == TypeCode::Int32 && right->GetTypeCode() == TypeCode::Int32)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
            }
            else if (left->GetTypeCode() == TypeCode::Int64 && right->GetTypeCode() == TypeCode::Int64)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
            }
            else if (left->GetTypeCode() == TypeCode::Float32 && right->GetTypeCode() == TypeCode::Float32)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
            }
            else if (left->GetTypeCode() == TypeCode::Float64 && right->GetTypeCode() == TypeCode::Float64)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
            }
            else if (left->GetTypeCode() == TypeCode::Char && right->GetTypeCode() == TypeCode::Char)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
            }
            else if (left->GetTypeCode() == TypeCode::String && right->GetTypeCode() == TypeCode::String)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
            }
            else
            {
                throw TreeException(__FILE__, __LINE__,
                                    "Comparison type mismatch: cannot compare '" +
                                        Utility::EnumToString(left->GetTypeCode()) + "' with '" +
                                        Utility::EnumToString(right->GetTypeCode()) + "'.",
                                    node, nullptr);
            }
        }
        case ExpressionType::Equal:
        case ExpressionType::NotEqual: {
            if (left->GetTypeCode() == TypeCode::Int32 && right->GetTypeCode() == TypeCode::Int32)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
            }
            else if (left->GetTypeCode() == TypeCode::Int64 && right->GetTypeCode() == TypeCode::Int64)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
            }
            else if (left->GetTypeCode() == TypeCode::Float32 && right->GetTypeCode() == TypeCode::Float32)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
            }
            else if (left->GetTypeCode() == TypeCode::Float64 && right->GetTypeCode() == TypeCode::Float64)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
            }
            else if (left->GetTypeCode() == TypeCode::Boolean && right->GetTypeCode() == TypeCode::Boolean)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
            }
            else if (left->GetTypeCode() == TypeCode::Char && right->GetTypeCode() == TypeCode::Char)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
            }
            else if (left->GetTypeCode() == TypeCode::String && right->GetTypeCode() == TypeCode::String)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
            }
            else
            {
                throw TreeException(__FILE__, __LINE__,
                                    "Equality comparison type mismatch: cannot compare '" +
                                        Utility::EnumToString(left->GetTypeCode()) + "' with '" +
                                        Utility::EnumToString(right->GetTypeCode()) + "' for equality.",
                                    node, nullptr);
            }
        }
        case ExpressionType::And:
        case ExpressionType::Or: {
            if (left->GetTypeCode() == TypeCode::Boolean && right->GetTypeCode() == TypeCode::Boolean)
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
            }
            else
            {
                throw TreeException(__FILE__, __LINE__,
                                    "Logical operation '" + Utility::EnumToString(node->NodeType()) +
                                        "' requires boolean operands, but got '" +
                                        Utility::EnumToString(left->GetTypeCode()) + "' and '" +
                                        Utility::EnumToString(right->GetTypeCode()) + "'.",
                                    node, nullptr);
            }
            break;
        }
        default: {
            throw TreeException(__FILE__, __LINE__,
                                "Unsupported binary operation '" + Utility::EnumToString(node->NodeType()) + "'.", node,
                                nullptr);
        }
        }
    }
}

const Type *TypeChecker::VisitConstant(const ConstantExpression *node, Scope<const Type *> *scope)
{
    return Register(node, TypeFactory::CreateBasicType(node->GetTypeCode()));
}

const Type *TypeChecker::VisitParameter(const ParameterExpression *node, Scope<const Type *> *scope)
{
    if (node->QualifiedName().size() == 1)
    {
        if (scope->Exists(node->Name()))
        {
            const Type *type = scope->Get(node->Name());

            return Register(node, type);
        }
        else
        {
            spdlog::error("'{}' is not defined.", Utility::UTF32ToUTF8(node->Name()));

            throw TreeException(__FILE__, __LINE__, Utility::UTF32ToUTF8(U"'" + node->Name() + U"' is not defined."),
                                node, nullptr);
        }
    }
    else
    {
        Namespace *top = namespaceStack.top();
        VariableDeclarationExpression *varDecl = namespaceFactory.SearchGlobalVariable(top, node->QualifiedName());
        if (varDecl != nullptr)
        {
            /* If the variable's type was already resolved during CheckGlobalVariable
               (in its own namespace context), reuse that cached type.
               This avoids re-resolving type syntax under the wrong namespace. */
            auto it = nodeTypes.find(varDecl);
            if (it != nodeTypes.end())
            {
                return Register(node, it->second);
            }
            const Type *type = ResolveTypeSyntax(varDecl->GetTypeSyntax());

            return Register(node, type);
        }
        LambdaExpression *funcDecl = namespaceFactory.SearchFunction(top, node->QualifiedName());
        if (funcDecl != nullptr)
        {
            /* If the function's type was already resolved during CheckNamespace
               (in its own namespace context), reuse that cached type.
               This avoids re-resolving parameter types under the wrong namespace. */
            auto it = nodeTypes.find(funcDecl);
            if (it != nodeTypes.end())
            {
                return Register(node, it->second);
            }
            CallableType *callableType = BuildCallableType(funcDecl);

            return Register(node, static_cast<const Type *>(callableType));
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

const Type *TypeChecker::VisitBlock(const BlockExpression *node, Scope<const Type *> *parent)
{
    Scope<const Type *> scope(parent);
    const Type *type = TypeFactory::CreateBasicType(TypeCode::Empty);
    for (const auto &expression : node->Expressions())
    {
        type = Visit(expression, &scope);
    }
    return Register(node, type);
}

const Type *TypeChecker::VisitConditional(const ConditionalExpression *node, Scope<const Type *> *scope)
{
    const Type *test = Visit(node->Test(), scope);
    if (test->GetTypeCode() == TypeCode::Boolean)
    {
        const Type *ifTrue = Visit(node->IfTrue(), scope);
        const Type *ifFalse = Visit(node->IfFalse(), scope);
        return Register(node, Types.CreateUnionType(ifTrue, ifFalse));
    }
    else
    {
        throw TreeException(__FILE__, __LINE__,
                            "Conditional expression requires a boolean condition, but got '" +
                                Utility::EnumToString(test->GetTypeCode()) + "'.",
                            node, nullptr);
    }
}

const Type *TypeChecker::VisitUnary(const UnaryExpression *node, Scope<const Type *> *scope)
{
    const Type *operand = Visit(node->Operand(), scope);
    switch (node->NodeType())
    {
    case ExpressionType::UnaryPlus: {
        if (operand->GetTypeCode() == TypeCode::Int32 || operand->GetTypeCode() == TypeCode::Int64 ||
            operand->GetTypeCode() == TypeCode::Float32 || operand->GetTypeCode() == TypeCode::Float64)
        {
            return Register(node, operand);
        }
        else
        {
            throw TreeException(__FILE__, __LINE__,
                                "Unary plus operator requires a numeric operand, but got '" +
                                    Utility::EnumToString(operand->GetTypeCode()) + "'.",
                                node, nullptr);
        }
    }
    case ExpressionType::UnaryMinus: {
        if (operand->GetTypeCode() == TypeCode::Int32 || operand->GetTypeCode() == TypeCode::Int64 ||
            operand->GetTypeCode() == TypeCode::Float32 || operand->GetTypeCode() == TypeCode::Float64)
        {
            return Register(node, operand);
        }
        else
        {
            throw TreeException(__FILE__, __LINE__,
                                "Unary minus operator requires a numeric operand, but got '" +
                                    Utility::EnumToString(operand->GetTypeCode()) + "'.",
                                node, nullptr);
        }
    }
    case ExpressionType::Not: {
        if (operand->GetTypeCode() == TypeCode::Boolean)
        {
            return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
        }
        else
        {
            throw TreeException(__FILE__, __LINE__,
                                "Logical 'not' operator requires a boolean operand, but got '" +
                                    Utility::EnumToString(operand->GetTypeCode()) + "'.",
                                node, nullptr);
        }
    }
    case ExpressionType::Halt: {
        if (operand->GetTypeCode() == TypeCode::Int32)
        {
            return Register(node, TypeFactory::CreateBasicType(TypeCode::Empty));
        }
        else
        {
            throw TreeException(__FILE__, __LINE__,
                                "'halt' requires an Int32 exit code, but got '" +
                                    Utility::EnumToString(operand->GetTypeCode()) + "'.",
                                node, nullptr);
        }
    }
    case ExpressionType::Convert: {
        const Type *targetType = ResolveTypeSyntax(node->GetTargetTypeSyntax());
        if (TypeFactory::AreTypesEqual(operand, targetType))
        {
            return Register(node, operand);
        }
        else
        {
            if (operand->GetTypeCode() == TypeCode::Int32 || operand->GetTypeCode() == TypeCode::Int64 ||
                operand->GetTypeCode() == TypeCode::Float32 || operand->GetTypeCode() == TypeCode::Float64 ||
                operand->GetTypeCode() == TypeCode::Boolean || operand->GetTypeCode() == TypeCode::Char)
            {
                if (targetType->GetTypeCode() == TypeCode::Int32 || targetType->GetTypeCode() == TypeCode::Int64 ||
                    targetType->GetTypeCode() == TypeCode::Float32 || targetType->GetTypeCode() == TypeCode::Float64 ||
                    targetType->GetTypeCode() == TypeCode::Boolean || targetType->GetTypeCode() == TypeCode::Char)
                {
                    return Register(node, targetType);
                }
            }
            throw TreeException(__FILE__, __LINE__,
                                "Cannot convert from '" + Utility::EnumToString(operand->GetTypeCode()) + "' to '" +
                                    Utility::EnumToString(targetType->GetTypeCode()) + "'.",
                                node, nullptr);
        }
    }
    default: {
        throw TreeException(__FILE__, __LINE__,
                            "Unsupported unary operation '" + Utility::EnumToString(node->NodeType()) + "'.", node,
                            nullptr);
    }
    }
}

const Type *TypeChecker::VisitCall(const CallExpression *node, Scope<const Type *> *scope)
{
    if (node->Function()->NodeType() == ExpressionType::MemberAccess)
    {
        return VisitMethodCall(node, scope);
    }
    else
    {
        auto callableType = Visit(node->Function(), scope);
        if (callableType->GetTypeCode() == TypeCode::Callable)
        {
            return CheckArguments(node, callableType, scope);
        }
        else if (callableType->GetTypeCode() == TypeCode::Array)
        {
            auto arrayType = static_cast<const ArrayType *>(callableType);
            if (node->Arguments().size() == 1)
            {
                auto indexType = Visit(node->Arguments().front(), scope);
                if (indexType->GetTypeCode() == TypeCode::Int32)
                {
                    return Register(node, arrayType->ElementType());
                }
                else
                {
                    throw TreeException(__FILE__, __LINE__,
                                        "Array index must be of type 'Int32', but got '" +
                                            Utility::EnumToString(indexType->GetTypeCode()) + "'.",
                                        node, nullptr);
                }
            }
            else
            {
                throw TreeException(__FILE__, __LINE__,
                                    "Array access requires exactly 1 index, but got " +
                                        std::to_string(node->Arguments().size()) + ".",
                                    node, nullptr);
            }
        }
        else
        {
            throw TreeException(__FILE__, __LINE__,
                                "Expression is not callable: got '" +
                                    Utility::EnumToString(callableType->GetTypeCode()) +
                                    "' but expected a function or array.",
                                node, nullptr);
        }
    }
}

const Type *TypeChecker::VisitLambda(const LambdaExpression *node, Scope<const Type *> *parent)
{
    Scope<const Type *> scope(parent);
    std::vector<const Type *> argumentTypes;
    argumentTypes.reserve(node->Parameters().size());
    for (const auto &parameter : node->Parameters())
    {
        spdlog::debug("Type checker declares parameter \"{}\".", Utility::UTF32ToUTF8(parameter->Name()));
        const Type *parameterType = ResolveTypeSyntax(parameter->GetTypeSyntax());
        scope.Declare(parameter->Name(), parameterType);
        argumentTypes.push_back(parameterType);
    }
    const Type *returnType = Visit(node->Body(), &scope);
    return Types.CreateCallableType(argumentTypes, returnType);
}

const Type *TypeChecker::VisitWhileLoop(const WhileLoopExpression *node, Scope<const Type *> *parent)
{
    Scope<const Type *> scope(parent);
    const Type *type = Visit(node->Condition(), &scope);
    if (type->GetTypeCode() == TypeCode::Boolean)
    {
        return Visit(node->Body(), &scope);
    }
    else
    {
        throw TreeException(__FILE__, __LINE__,
                            "While loop condition must be a boolean, but got '" +
                                Utility::EnumToString(type->GetTypeCode()) + "'.",
                            node, nullptr);
    }
}

const Type *TypeChecker::VisitDefault(const DefaultExpression *node, Scope<const Type *> *scope)
{
    return Register(node, ResolveTypeSyntax(node->GetTypeSyntax()));
}

const Type *TypeChecker::VisitVariableDeclaration(const VariableDeclarationExpression *node, Scope<const Type *> *scope)
{
    const Type *initializer = Visit(node->Initializer(), scope);
    if (node->GetTypeSyntax() == nullptr)
    {
        scope->Declare(node->Name(), initializer);
        Register(node, initializer);
        spdlog::info("Inferred the type of the variable '{}' based on the assigned value.",
                     Utility::UTF32ToUTF8(node->Name()));
    }
    else
    {
        const Type *leftType = ResolveTypeSyntax(node->GetTypeSyntax());
        if (TypeFactory::AreTypesEqual(leftType, initializer))
        {
            scope->Declare(node->Name(), initializer);
            Register(node, initializer);
        }
        else
        {
            throw TreeException(__FILE__, __LINE__,
                                "Variable '" + Utility::UTF32ToUTF8(node->Name()) + "' declared as '" +
                                    Utility::EnumToString(leftType->GetTypeCode()) + "' but initialized with '" +
                                    Utility::EnumToString(initializer->GetTypeCode()) + "'.",
                                static_cast<const Expression *>(node), nullptr);
        }
    }

    return TypeFactory::CreateBasicType(TypeCode::Empty);
}

const Type *TypeChecker::VisitNew(const NewExpression *node, Scope<const Type *> *scope)
{
    const Type *type = ResolveTypeSyntax(node->GetTypeSyntax());
    spdlog::info("Create a new object of type '{}'", Utility::EnumToString(type->GetTypeCode()));
    Register(node, type);
    if (type->GetTypeCode() == TypeCode::Structure)
    {

        const StructureType *structureType = static_cast<const StructureType *>(type);

        auto path = structureType->QualifiedName();
        spdlog::info("Looking for structure definition of '{}'.",
                     Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", path)));
        Namespace *top = namespaceStack.top();

        StructureExpression *structureDefinition = namespaceFactory.SearchStructure(top, path);

        if (structureDefinition != nullptr)
        {
            std::unordered_set<std::u32string> fieldNameSet;
            for (const std::u32string &key : node->FieldsInitialization().GetAllKeys())
            {
                spdlog::info("Checking the type of field '{}'.", Utility::UTF32ToUTF8(key));
                if (structureDefinition->Fields().ContainsKey(key))
                {
                    if (fieldNameSet.count(key) > 0)
                    {
                        /* The field already exists. */
                        spdlog::error("The field '{}' has already been assigned a value. "
                                      "Do not assign a value to it again.",
                                      Utility::UTF32ToUTF8(key));

                        throw TreeException(__FILE__, __LINE__,
                                            "The field has already been assigned a value. "
                                            "Do not assign a value to it again.",
                                            static_cast<const Expression *>(node), nullptr);
                    }
                    else
                    {
                        /* Check if the types are equal. */
                        const Type *actualType = Visit(node->FieldsInitialization().GetItemByKey(key), scope);
                        const Type *expectedType = ResolveTypeSyntax(structureDefinition->Fields().GetItemByKey(key));
                        if (TypeFactory::AreTypesEqual(expectedType, actualType))
                        {
                            fieldNameSet.insert(key);
                        }
                        else
                        {
                            throw TreeException(__FILE__, __LINE__,
                                                "Field '" + Utility::UTF32ToUTF8(key) + "' type mismatch: expected '" +
                                                    Utility::EnumToString(expectedType->GetTypeCode()) + "' but got '" +
                                                    Utility::EnumToString(actualType->GetTypeCode()) + "'.",
                                                static_cast<const Expression *>(node), nullptr);
                        }
                    }
                }
                else
                {
                    /* The field doesn't exist. */
                    throw TreeException(__FILE__, __LINE__,
                                        "Field '" + Utility::UTF32ToUTF8(key) + "' does not exist in structure '" +
                                            Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", path)) + "'.",
                                        static_cast<const Expression *>(node), nullptr);
                }
            }
            if (fieldNameSet.size() == structureDefinition->Fields().GetAllItems().size())
            {
                return type;
            }
            else
            {
                /* Not all fields have been assigned values. */
                throw TreeException(__FILE__, __LINE__,
                                    "Not all fields have been initialized in structure '" +
                                        Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", path)) +
                                        "': " + std::to_string(fieldNameSet.size()) + " of " +
                                        std::to_string(structureDefinition->Fields().GetAllItems().size()) +
                                        " fields provided.",
                                    node, nullptr);
            }
        }
        else
        {
            throw TreeException(__FILE__, __LINE__,
                                "Structure '" + Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", path)) +
                                    "' is not defined.",
                                static_cast<const Expression *>(node), nullptr);
        }
    }
    else
    {
        throw TreeException(__FILE__, __LINE__,
                            "Cannot instantiate type '" + Utility::EnumToString(type->GetTypeCode()) +
                                "': only structures can be instantiated with 'new'.",
                            static_cast<const Expression *>(node), nullptr);
    }

    return type;
}

const Type *TypeChecker::VisitMember(const MemberExpression *node, Scope<const Type *> *scope)
{
    const Type *type = Visit(node->GetExpression(), scope);
    if (type->GetTypeCode() == TypeCode::Structure)
    {
        const StructureType *structureType = static_cast<const StructureType *>(type);
        if (structureType->Fields().ContainsKey(node->FieldName()))
        {
            return Register(node, structureType->Fields().GetItemByKey(node->FieldName()));
        }
        else if (structureType->Methods().ContainsKey(node->FieldName()))
        {
            return Register(node, structureType->Methods().GetItemByKey(node->FieldName()));
        }
        else
        {
            throw TreeException(
                __FILE__, __LINE__,
                "Field '" + Utility::UTF32ToUTF8(node->FieldName()) + "' is not defined in structure '" +
                    Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", structureType->QualifiedName())) + "'.",
                node, nullptr);
        }
    }
    else
    {
        /* TODO: string */
        spdlog::error("The type of object that the expression is trying to access is not supported.");

        throw TreeException(__FILE__, __LINE__,
                            "Cannot access field '" + Utility::UTF32ToUTF8(node->FieldName()) + "' on type '" +
                                Utility::EnumToString(type->GetTypeCode()) +
                                "'. Member access is only supported on structures.",
                            node, nullptr);
    }
}

const Type *TypeChecker::GetType(const Expression *node)
{
    return nodeTypes.at(node);
}

void TypeChecker::CheckNamespace(Scope<const Type *> *parent)
{
    Namespace *top = namespaceStack.top();
    Scope<const Type *> *scope(parent);

    /* Declare the types of global variables. */
    for (const auto &varDecl : top->GlobalVariables().GetAllItems())
    {
        scope->Declare(varDecl->Name(), ResolveTypeSyntax(varDecl->GetTypeSyntax()));
    }

    /* Declare the types of functions. */
    for (const auto &funcDecl : top->Functions().GetAllItems())
    {
        CallableType *callableType = BuildCallableType(funcDecl);
        scope->Declare(funcDecl->Name(), callableType);
    }

    for (const auto &varDecl : top->GlobalVariables().GetAllItems())
    {
        CheckGlobalVariable(varDecl, scope, top);
    }
    for (const auto &funcDecl : top->Functions().GetAllItems())
    {
        const Type *actualType = VisitLambda(funcDecl, scope);
        const Type *declarationType = scope->Get(funcDecl->Name());
        if (!CheckFunctionType(declarationType, actualType))
        {
            throw TreeException(__FILE__, __LINE__,
                                "The function's implementation "
                                "type does not match the "
                                "declared function type.",
                                funcDecl, nullptr);
        }
        else
        {
            Register(funcDecl, declarationType);
        }
    }
    for (const auto &structDecl : top->Structures().GetAllItems())
    {
        const Type *type = ResolveStructureDefinition(structDecl);
        if (type->GetTypeCode() == TypeCode::Structure)
        {
            const StructureType *structureType = static_cast<const StructureType *>(type);
            Register(structDecl, structureType);
            scope->Declare(U"this", structureType);
            for (const auto &method : structDecl->Methods().GetAllItems())
            {
                scope->Declare(method->Name(), structureType->Methods().GetItemByKey(method->Name()));
            }
            for (const auto &method : structDecl->Methods().GetAllItems())
            {
                const Type *actualType = VisitLambda(method, scope);
                const Type *declarationType = structureType->Methods().GetItemByKey(method->Name());
                if (!CheckFunctionType(declarationType, actualType))
                {
                    throw TreeException(__FILE__, __LINE__,
                                        "The method's implementation "
                                        "type does not match the "
                                        "declared method type.",
                                        method, nullptr);
                }
                else
                {
                    Register(method, declarationType);
                }
            }

            for (const InterfaceType *interfaceType : GetAllImplementedInterfaces(structureType))
            {
                CheckInterfaceImplementation(structDecl, structureType, interfaceType);
            }
        }
        else
        {
            throw TreeException(__FILE__, __LINE__,
                                "Declared type is not a structure: got '" + Utility::EnumToString(type->GetTypeCode()) +
                                    "'.",
                                structDecl, nullptr);
        }
    }
    for (const auto &interfaceDecl : top->Interfaces().GetAllItems())
    {
        const Type *type = ResolveInterfaceDefinition(interfaceDecl);
        if (type->GetTypeCode() == TypeCode::Interface)
        {
            const InterfaceType *interfaceType = static_cast<const InterfaceType *>(type);
            Register(interfaceDecl, interfaceType);
        }
        else
        {
            throw TreeException(__FILE__, __LINE__,
                                "Declared type is not an interface: got '" +
                                    Utility::EnumToString(type->GetTypeCode()) + "'.",
                                interfaceDecl, nullptr);
        }
    }

    for (const auto &current : top->Children().GetAllItems())
    {
        namespaceStack.push(current);
        CheckNamespace(scope);
        namespaceStack.pop();
    }
}

void TypeChecker::CheckGlobalVariable(const VariableDeclarationExpression *node, Scope<const Type *> *parent,
                                      Namespace *current)
{
    Scope<const Type *> *scope(parent);
    const Type *initializerType = Visit(node->Initializer(), scope);
    const Type *declaredType = ResolveTypeSyntax(node->GetTypeSyntax());
    if (TypeFactory::AreTypesEqual(declaredType, initializerType))
    {
        scope->Declare(node->Name(), initializerType);
        Register(node, initializerType);

        /* Add the initializer to the namespace. */
        std::u32string initializerName = node->Name() + U"#Initializer";
        LambdaExpression *initializer = expressionFactory.Create<LambdaExpression>(
            node->Initializer()->GetSourceRange(), initializerName, node->Initializer(),
            std::vector<ParameterExpression *>{}, nullptr, std::vector<Annotation>{});
        current->Functions().AddItem(initializerName, initializer);
        parent->Declare(initializerName, Types.CreateCallableType({}, initializerType));
    }
    else
    {
        throw TreeException(__FILE__, __LINE__,
                            "Global variable '" + Utility::UTF32ToUTF8(node->Name()) + "' declared as '" +
                                Utility::EnumToString(declaredType->GetTypeCode()) + "' but initialized with '" +
                                Utility::EnumToString(initializerType->GetTypeCode()) + "'.",
                            static_cast<const Expression *>(node), nullptr);
    }
}

const Type *TypeChecker::Register(const Expression *node, const Type *type)
{
    nodeTypes[node] = type;
    return type;
}

bool TypeChecker::CheckFunctionType(const Type *declaration, const Type *actual)
{
    if (declaration->GetTypeCode() == TypeCode::Callable && actual->GetTypeCode() == TypeCode::Callable)
    {
        const CallableType *declarationCallableType = static_cast<const CallableType *>(declaration);
        const CallableType *actualCallableType = static_cast<const CallableType *>(actual);
        if (declarationCallableType->Arguments().size() == actualCallableType->Arguments().size())
        {

            for (size_t i = 0; i < declarationCallableType->Arguments().size(); i++)
            {
                if (!TypeFactory::AreTypesEqual(declarationCallableType->Arguments().at(i),
                                                actualCallableType->Arguments().at(i)))
                {

                    return false;
                }
            }

            if (TypeFactory::AreTypesEqual(declarationCallableType->GetReturnType(),
                                           actualCallableType->GetReturnType()))
            {

                return true;
            }
            else
            {
                if (declarationCallableType->GetReturnType()->GetTypeCode() == TypeCode::Empty)
                {
                    spdlog::info("The declared function's return type is 'Void,' "
                                 "allowing any return type from the function body.");

                    return true;
                }
                else
                {

                    return false;
                }
            }
        }
        else
        {

            return false;
        }
    }
    else
    {
        if (declaration->GetTypeCode() != TypeCode::Callable)
        {
            spdlog::error("The declared function type is not callable.");
        }

        if (actual->GetTypeCode() != TypeCode::Callable)
        {
            spdlog::error("The implementation type of the function is not callable.");
        }

        return false;
    }
}

const Type *TypeChecker::ResolveTypeSyntax(const TypeSyntax *typeSyntax)
{
    if (typeSyntax == nullptr)
    {
        return TypeFactory::CreateBasicType(TypeCode::Unknown);
    }

    const auto &qualifiedName = typeSyntax->QualifiedName();
    if (qualifiedName.size() == 1)
    {
        const std::u32string &name = qualifiedName.front();
        if (name == U"Int")
        {
            return TypeFactory::CreateBasicType(TypeCode::Int32);
        }
        else if (name == U"Long")
        {
            return TypeFactory::CreateBasicType(TypeCode::Int64);
        }
        else if (name == U"Bool")
        {
            return TypeFactory::CreateBasicType(TypeCode::Boolean);
        }
        else if (name == U"Float")
        {
            return TypeFactory::CreateBasicType(TypeCode::Float32);
        }
        else if (name == U"Double")
        {
            return TypeFactory::CreateBasicType(TypeCode::Float64);
        }
        else if (name == U"Char")
        {
            return TypeFactory::CreateBasicType(TypeCode::Char);
        }
        else if (name == U"String")
        {
            return TypeFactory::CreateBasicType(TypeCode::String);
        }
        else if (name == U"Void")
        {
            return TypeFactory::CreateBasicType(TypeCode::Empty);
        }
        else if (name == U"Array")
        {
            if (typeSyntax->Arguments().size() != 1)
            {
                throw TreeException(__FILE__, __LINE__, "Array type must have exactly one argument.", nullptr, nullptr);
            }
            return Types.CreateArrayType(ResolveTypeSyntax(typeSyntax->Arguments().front()));
        }
        else if (name == U"Func")
        {
            if (typeSyntax->Arguments().empty())
            {
                throw TreeException(__FILE__, __LINE__, "Func type must specify at least a return type.", nullptr,
                                    nullptr);
            }
            std::vector<const Type *> argumentTypes;
            argumentTypes.reserve(typeSyntax->Arguments().size() - 1);
            for (size_t i = 0; i + 1 < typeSyntax->Arguments().size(); ++i)
            {
                argumentTypes.push_back(ResolveTypeSyntax(typeSyntax->Arguments().at(i)));
            }
            const Type *returnType = ResolveTypeSyntax(typeSyntax->Arguments().back());
            return Types.CreateCallableType(argumentTypes, returnType);
        }
    }

    Namespace *top = namespaceStack.top();
    StructureExpression *structureDefinition = namespaceFactory.SearchStructure(top, qualifiedName);
    spdlog::info("Looking for structure definition of '{}' in the namespace '{}'.",
                 Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", qualifiedName)),
                 Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", top->GetFullQualifiedName())));
    if (structureDefinition != nullptr)
    {
        return ResolveStructureDefinition(structureDefinition);
    }
    else
    {
        InterfaceExpression *interfaceDefinition = namespaceFactory.SearchInterface(top, qualifiedName);
        if (interfaceDefinition != nullptr)
        {
            return ResolveInterfaceDefinition(interfaceDefinition);
        }
        else
        {
            spdlog::error("The type '{}' is not defined in the namespace '{}'.",
                          Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", qualifiedName)),
                          Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", top->GetFullQualifiedName())));

            throw TreeException(
                __FILE__, __LINE__,
                "Type '" + Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", qualifiedName)) +
                    "' is not defined in the namespace '" +
                    Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", top->GetFullQualifiedName())) + "'.",
                nullptr, nullptr);
        }
    }
}

StructureType *TypeChecker::ResolveStructureDefinition(StructureExpression *structureDefinition)
{
    auto cacheIt = structureTypeCache.find(structureDefinition);
    if (cacheIt != structureTypeCache.end())
    {
        return cacheIt->second;
    }

    StructureType *structureType = Types.CreateStructureType(structureDefinition->QualifiedName(), {}, {}, {});
    structureTypeCache[structureDefinition] = structureType;

    Utility::OrderPreservingMap<std::u32string, const Type *> resolvedFields;
    for (const std::u32string &fieldName : structureDefinition->Fields().GetAllKeys())
    {
        resolvedFields.AddItem(fieldName, ResolveTypeSyntax(structureDefinition->Fields().GetItemByKey(fieldName)));
    }

    Utility::OrderPreservingMap<std::u32string, const CallableType *> resolvedMethods;
    for (const std::u32string &methodName : structureDefinition->Methods().GetAllKeys())
    {
        LambdaExpression *methodDefinition = structureDefinition->Methods().GetItemByKey(methodName);
        resolvedMethods.AddItem(methodName, BuildCallableType(methodDefinition));
    }

    std::vector<const InterfaceType *> resolvedInterfaces;
    for (const TypeSyntax *interfaceTypeSyntax : structureDefinition->Interfaces())
    {
        auto resolvedType = ResolveTypeSyntax(interfaceTypeSyntax);
        if (resolvedType->GetTypeCode() == TypeCode::Interface)
        {
            resolvedInterfaces.push_back(static_cast<const InterfaceType *>(resolvedType));
        }
        else
        {
            throw TreeException(__FILE__, __LINE__,
                                "The type specified in the 'implements' clause is not an interface: got '" +
                                    Utility::EnumToString(resolvedType->GetTypeCode()) + "'.",
                                structureDefinition, nullptr);
        }
    }

    structureType->SetFields(resolvedFields);
    structureType->SetMethods(resolvedMethods);
    structureType->SetInterfaces(resolvedInterfaces);

    return structureType;
}

InterfaceType *TypeChecker::ResolveInterfaceDefinition(InterfaceExpression *interfaceDefinition)
{
    auto cacheIt = interfaceTypeCache.find(interfaceDefinition);
    if (cacheIt != interfaceTypeCache.end())
    {
        return cacheIt->second;
    }

    InterfaceType *interfaceType = Types.CreateInterfaceType(interfaceDefinition->QualifiedName(), {}, {});
    interfaceTypeCache[interfaceDefinition] = interfaceType;

    Utility::OrderPreservingMap<std::u32string, const CallableType *> resolvedMethods;
    for (const std::u32string &methodName : interfaceDefinition->Methods().GetAllKeys())
    {
        LambdaExpression *methodDefinition = interfaceDefinition->Methods().GetItemByKey(methodName);
        resolvedMethods.AddItem(methodName, BuildCallableType(methodDefinition));
    }
    interfaceType->SetMethods(resolvedMethods);

    std::vector<const InterfaceType *> resolvedBaseInterfaces;
    for (const TypeSyntax *baseInterfaceTypeSyntax : interfaceDefinition->BaseInterfaces())
    {
        auto resolvedType = ResolveTypeSyntax(baseInterfaceTypeSyntax);
        if (resolvedType->GetTypeCode() == TypeCode::Interface)
        {
            resolvedBaseInterfaces.push_back(static_cast<const InterfaceType *>(resolvedType));
        }
        else
        {
            throw TreeException(__FILE__, __LINE__,
                                "The type specified in the 'extends' clause is not an interface: got '" +
                                    Utility::EnumToString(resolvedType->GetTypeCode()) + "'.",
                                interfaceDefinition, nullptr);
        }
    }
    interfaceType->SetBaseInterfaces(resolvedBaseInterfaces);

    return interfaceType;
}

CallableType *TypeChecker::BuildCallableType(const LambdaExpression *node)
{
    std::vector<const Type *> parameterTypes;
    parameterTypes.reserve(node->Parameters().size());
    for (const auto &parameter : node->Parameters())
    {
        parameterTypes.push_back(ResolveTypeSyntax(parameter->GetTypeSyntax()));
    }
    const Type *returnType = ResolveTypeSyntax(node->ReturnTypeSyntax());
    return Types.CreateCallableType(parameterTypes, returnType);
}

const Type *TypeChecker::CheckAssignment(const BinaryExpression *node, Scope<const Type *> *scope)
{
    const Type *right = Visit(node->Right(), scope);
    if (node->Left()->NodeType() == ExpressionType::Parameter)
    {
        /* TODO: check if the variable is modifiable. */
        const Type *left = Visit(node->Left(), scope);

        if (TypeFactory::AreTypesEqual(left, right) || Types.IsSubtype(right, left))
        {
            return Register(node, TypeFactory::CreateBasicType(TypeCode::Empty));
        }
        else
        {
            throw TreeException(__FILE__, __LINE__,
                                "Parameter assignment type mismatch: expected '" +
                                    Utility::EnumToString(left->GetTypeCode()) + "' but got '" +
                                    Utility::EnumToString(right->GetTypeCode()) + "'.",
                                node, nullptr);
        }
    }
    else if (node->Left()->NodeType() == ExpressionType::MemberAccess)
    {
        /* TODO: check if the field is modifiable. */
        const Type *left = Visit(node->Left(), scope);

        if (TypeFactory::AreTypesEqual(left, right) || Types.IsSubtype(right, left))
        {
            return Register(node, TypeFactory::CreateBasicType(TypeCode::Empty));
        }
        else
        {
            throw TreeException(__FILE__, __LINE__,
                                "Member assignment type mismatch: expected '" +
                                    Utility::EnumToString(left->GetTypeCode()) + "' but got '" +
                                    Utility::EnumToString(right->GetTypeCode()) + "'.",
                                node, nullptr);
        }
    }
    else if (node->Left()->NodeType() == ExpressionType::Call)
    {
        const CallExpression *callExpression = static_cast<const CallExpression *>(node->Left());
        const Type *functionType = Visit(callExpression->Function(), scope);
        if (functionType->GetTypeCode() == TypeCode::Array)
        {
            const ArrayType *arrayType = static_cast<const ArrayType *>(functionType);
            if (callExpression->Arguments().size() != 1)
            {
                throw TreeException(__FILE__, __LINE__, "array assignment must have exactly one index argument.", node,
                                    nullptr);
            }
            else
            {
                const Type *indexType = Visit(callExpression->Arguments().front(), scope);
                if (indexType->GetTypeCode() != TypeCode::Int32)
                {
                    throw TreeException(__FILE__, __LINE__,
                                        "Array index must be of type 'Int32', but got '" +
                                            Utility::EnumToString(indexType->GetTypeCode()) + "'.",
                                        node, nullptr);
                }
            }
            if (TypeFactory::AreTypesEqual(arrayType->ElementType(), right))
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Empty));
            }
            if (Types.IsSubtype(right, arrayType->ElementType()))
            {
                /* TODO: Cast */
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Empty));
            }
            else
            {
                throw TreeException(__FILE__, __LINE__,
                                    "Array element assignment type mismatch: expected '" +
                                        Utility::EnumToString(arrayType->ElementType()->GetTypeCode()) + "' but got '" +
                                        Utility::EnumToString(right->GetTypeCode()) + "'.",
                                    node, nullptr);
            }
        }
        else
        {
            throw TreeException(__FILE__, __LINE__,
                                "The left-hand side of the assignment is a call expression, but it's not an array "
                                "access. It is currently not supported.",
                                node, nullptr);
        }
    }
    else
    {
        spdlog::error("Unsupported left side in assignment. Expected a parameter, but got: {}",
                      Utility::EnumToString(node->Left()->NodeType()));

        throw TreeException(__FILE__, __LINE__,
                            "Unsupported left side in assignment. Expected a parameter, but got: " +
                                Utility::EnumToString(node->Left()->NodeType()),
                            node, nullptr);
    }
}

const Type *TypeChecker::VisitMethodCall(const CallExpression *node, Scope<const Type *> *scope)
{
    if (node->Function()->NodeType() != ExpressionType::MemberAccess)
    {
        throw TreeException(__FILE__, __LINE__, "Expected a member access expression for method call.", node, nullptr);
    }
    else
    {
        const MemberExpression *memberAccess = static_cast<const MemberExpression *>(node->Function());
        const Type *objectType = Visit(memberAccess->GetExpression(), scope);
        if (objectType->GetTypeCode() == TypeCode::Structure)
        {
            const StructureType *structureType = static_cast<const StructureType *>(objectType);
            if (structureType->Methods().ContainsKey(memberAccess->FieldName()))
            {
                const CallableType *methodType = structureType->Methods().GetItemByKey(memberAccess->FieldName());
                Register(memberAccess, methodType);

                return CheckArguments(node, methodType, scope);
            }
            else if (structureType->Fields().ContainsKey(memberAccess->FieldName()))
            {
                const Type *fieldType = structureType->Fields().GetItemByKey(memberAccess->FieldName());
                if (fieldType->GetTypeCode() == TypeCode::Callable)
                {
                    const CallableType *callableFieldType = static_cast<const CallableType *>(fieldType);
                    Register(memberAccess, callableFieldType);

                    return CheckArguments(node, callableFieldType, scope);
                }
                else
                {
                    throw TreeException(
                        __FILE__, __LINE__,
                        "The member '" + Utility::UTF32ToUTF8(memberAccess->FieldName()) +
                            "' is a field, not a method, and cannot be called. If you want to call it as a "
                            "function, make sure it's declared as a callable type.",
                        node, nullptr);
                }
            }
            else
            {
                throw TreeException(
                    __FILE__, __LINE__,
                    "Method '" + Utility::UTF32ToUTF8(memberAccess->FieldName()) + "' is not defined in structure '" +
                        Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", structureType->QualifiedName())) + "'.",
                    node, nullptr);
            }
        }
        else if (objectType->GetTypeCode() == TypeCode::Interface)
        {
            const InterfaceType *interfaceType = static_cast<const InterfaceType *>(objectType);
            if (interfaceType->Methods().ContainsKey(memberAccess->FieldName()))
            {
                const CallableType *methodType = interfaceType->Methods().GetItemByKey(memberAccess->FieldName());
                Register(memberAccess, methodType);

                return CheckArguments(node, methodType, scope);
            }
            else
            {
                throw TreeException(
                    __FILE__, __LINE__,
                    "Method '" + Utility::UTF32ToUTF8(memberAccess->FieldName()) + "' is not defined in interface '" +
                        Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", interfaceType->QualifiedName())) + "'.",
                    node, nullptr);
            }
        }
        else
        {
            throw TreeException(__FILE__, __LINE__,
                                "Cannot call member '" + Utility::UTF32ToUTF8(memberAccess->FieldName()) +
                                    "' on type '" + Utility::EnumToString(objectType->GetTypeCode()) +
                                    "'. Member access is only supported on structures and interfaces.",
                                node, nullptr);
        }
    }
}

const Type *TypeChecker::CheckArguments(const CallExpression *node, const Type *callableType,
                                        Scope<const Type *> *scope)
{

    auto t = static_cast<const CallableType *>(callableType);
    if (t->Arguments().size() == node->Arguments().size())
    {
        for (size_t i = 0; i < node->Arguments().size(); i++)
        {
            auto argType = Visit(node->Arguments().at(i), scope);
            if (!TypeFactory::AreTypesEqual(argType, t->Arguments().at(i)))
            {
                throw TreeException(__FILE__, __LINE__,
                                    "Argument " + std::to_string(i + 1) + " type mismatch: expected '" +
                                        Utility::EnumToString(t->Arguments().at(i)->GetTypeCode()) + "' but got '" +
                                        Utility::EnumToString(argType->GetTypeCode()) + "'.",
                                    node, nullptr);
            }
        }
        return Register(node, t->GetReturnType());
    }
    else
    {
        throw TreeException(__FILE__, __LINE__,
                            "Argument count mismatch: function expects " + std::to_string(t->Arguments().size()) +
                                " argument(s), but got " + std::to_string(node->Arguments().size()) + ".",
                            node, nullptr);
    }
}

void TypeChecker::CheckInterfaceImplementation(const StructureExpression *node, const StructureType *structureType,
                                               const InterfaceType *interfaceType)
{
    for (const auto &methodName : interfaceType->Methods().GetAllKeys())
    {
        if (structureType->Methods().ContainsKey(methodName))
        {
            const CallableType *interfaceMethodType = interfaceType->Methods().GetItemByKey(methodName);
            const CallableType *structureMethodType = structureType->Methods().GetItemByKey(methodName);
            if (!CheckFunctionType(interfaceMethodType, structureMethodType))
            {
                throw TreeException(
                    __FILE__, __LINE__,
                    "Method '" + Utility::UTF32ToUTF8(methodName) + "' in structure '" +
                        Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", structureType->QualifiedName())) +
                        "' does not match the method type required by interface '" +
                        Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", interfaceType->QualifiedName())) + "'.",
                    node, nullptr);
            }
        }
        else
        {
            throw TreeException(
                __FILE__, __LINE__,
                "Structure '" +
                    Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", structureType->QualifiedName())) +
                    "' does not implement method '" + Utility::UTF32ToUTF8(methodName) + "' required by interface '" +
                    Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", interfaceType->QualifiedName())) + "'.",
                node, nullptr);
        }
    }
}

std::vector<const InterfaceType *> TypeChecker::GetAllImplementedInterfaces(const StructureType *structureType)
{
    std::unordered_set<const InterfaceType *> visited;
    std::stack<const InterfaceType *> stack;
    for (const InterfaceType *interfaceType : structureType->Interfaces())
    {
        stack.push(interfaceType);
    }
    while (!stack.empty())
    {
        const InterfaceType *current = stack.top();
        stack.pop();
        if (visited.find(current) == visited.end())
        {
            visited.insert(current);
            for (const InterfaceType *baseInterface : current->BaseInterfaces())
            {
                stack.push(baseInterface);
            }
        }
    }

    return std::vector<const InterfaceType *>(visited.begin(), visited.end());
}

}; /* namespace Visitors */
}; /* namespace Cygni */