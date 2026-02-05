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
        const Type *right = Visit(node->Right(), scope);
        if (node->Left()->NodeType() == ExpressionType::Parameter)
        {
            /* TODO: check if the variable is modifiable. */
            const Type *left = Visit(node->Left(), scope);

            if (TypeFactory::AreTypesEqual(left, right))
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Empty));
            }
            else
            {
                throw TreeException(__FILE__, __LINE__, "parameter assignment type mismatch error.", node, nullptr);
            }
        }
        else if (node->Left()->NodeType() == ExpressionType::MemberAccess)
        {
            /* TODO: check if the field is modifiable. */
            const Type *left = Visit(node->Left(), scope);

            if (TypeFactory::AreTypesEqual(left, right))
            {
                return Register(node, TypeFactory::CreateBasicType(TypeCode::Empty));
            }
            else
            {
                throw TreeException(__FILE__, __LINE__, "member assignment type mismatch error.", node, nullptr);
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
                throw TreeException(__FILE__, __LINE__, "arithmetic operation type mismatch error.", node, nullptr);
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
                spdlog::error("left: {}, right: {}", Utility::EnumToString(left->GetTypeCode()),
                              Utility::EnumToString(right->GetTypeCode()));
                throw TreeException(__FILE__, __LINE__, "comparison type mismatch error.", node, nullptr);
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
                throw TreeException(__FILE__, __LINE__, "equality type mismatch error.", node, nullptr);
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
                spdlog::info("The operands of logical operations (and, or) must be boolean type.");
                throw TreeException(__FILE__, __LINE__,
                                    "The operands of logical operations (and, or) must be boolean type.", node,
                                    nullptr);
            }
            break;
        }
        default: {
            throw TreeException(__FILE__, __LINE__, "type mismatch error.", node, nullptr);
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
            const Type *type = ResolveTypeSyntax(varDecl->GetTypeSyntax());

            return Register(node, type);
        }
        LambdaExpression *funcDecl = namespaceFactory.SearchFunction(top, node->QualifiedName());
        if (funcDecl != nullptr)
        {
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
                            "The type of condition of the conditional expression must be a boolean type.", node,
                            nullptr);
    }
}

const Type *TypeChecker::VisitUnary(const UnaryExpression *node, Scope<const Type *> *scope)
{
    const Type *operand = Visit(node->Operand(), scope);
    switch (node->NodeType())
    {
    case ExpressionType::Not: {
        if (operand->GetTypeCode() == TypeCode::Boolean)
        {
            return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
        }
        else
        {
            throw TreeException(__FILE__, __LINE__, "type mismatch error.", node, nullptr);
        }
    }
    case ExpressionType::Halt: {
        if (operand->GetTypeCode() == TypeCode::Int32)
        {
            return Register(node, TypeFactory::CreateBasicType(TypeCode::Empty));
        }
        else
        {
            throw TreeException(__FILE__, __LINE__, "type mismatch error.", node, nullptr);
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
            throw TreeException(__FILE__, __LINE__, "convert type mismatch error.", node, nullptr);
        }
    }
    default: {
        throw TreeException(__FILE__, __LINE__, "type mismatch error.", node, nullptr);
    }
    }
}

const Type *TypeChecker::VisitCall(const CallExpression *node, Scope<const Type *> *scope)
{
    auto callableType = Visit(node->Function(), scope);
    if (callableType->GetTypeCode() == TypeCode::Callable)
    {
        auto t = static_cast<const CallableType *>(callableType);
        if (t->Arguments().size() == node->Arguments().size())
        {
            for (size_t i = 0; i < node->Arguments().size(); i++)
            {
                auto argType = Visit(node->Arguments().at(i), scope);
                if (!TypeFactory::AreTypesEqual(argType, t->Arguments().at(i)))
                {
                    throw TreeException(__FILE__, __LINE__, "argument " + std::to_string(i) + " type mismatch error.",
                                        node, nullptr);
                }
            }
            return Register(node, t->GetReturnType());
        }
        else
        {
            throw TreeException(__FILE__, __LINE__, "argument size mismatch error.", node, nullptr);
        }
    }
    else
    {
        throw TreeException(__FILE__, __LINE__, "The type of function of the call expression must be a callable type.",
                            node, nullptr);
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
        throw TreeException(__FILE__, __LINE__, "The condition of the loop expression must return a boolean value.",
                            node, nullptr);
    }
}

const Type *TypeChecker::VisitDefault(const DefaultExpression *node, Scope<const Type *> *scope)
{
    return ResolveTypeSyntax(node->GetTypeSyntax());
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
            spdlog::error("The value assigned to variable '{}' does not match its "
                          "declared type.",
                          Utility::UTF32ToUTF8(node->Name()));
            throw TreeException(__FILE__, __LINE__, "The assigned value does not match the declared variable type.",
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
                            spdlog::error("The type of the value assigned does not match the "
                                          "type of the field '{}'.",
                                          Utility::UTF32ToUTF8(key));

                            throw TreeException(__FILE__, __LINE__,
                                                "The type of the value assigned does not "
                                                "match the type of the field.",
                                                static_cast<const Expression *>(node), nullptr);
                        }
                    }
                }
                else
                {
                    /* The field doesn't exist. */
                    spdlog::error("The field '{}' doesn't exist.", Utility::UTF32ToUTF8(key));

                    throw TreeException(__FILE__, __LINE__, "The field doesn't exist.",
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
                spdlog::error("Not all fields have been assigned values.");

                throw TreeException(__FILE__, __LINE__, "Not all fields have been assigned values.", node, nullptr);
            }
        }
        else
        {
            spdlog::error("The structure is not defined. Qualified name: '{}'",
                          Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", path)));

            throw TreeException(__FILE__, __LINE__, "The structure is not defined.",
                                static_cast<const Expression *>(node), nullptr);
        }
    }
    else
    {
        spdlog::error("The object being instantiated is not recognized as a structure. The object type is '{}'.",
                      Utility::EnumToString(type->GetTypeCode()));

        throw TreeException(__FILE__, __LINE__, "The object being instantiated is not recognized as a structure.",
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
        else
        {
            spdlog::error("The field '{}' is not defined.", Utility::UTF32ToUTF8(node->FieldName()));

            throw TreeException(__FILE__, __LINE__, "The field is not defined.", node, nullptr);
        }
    }
    else
    {
        /* TODO: string */
        spdlog::error("The type of object that the expression is trying to access is not supported.");

        throw TreeException(__FILE__, __LINE__,
                            "The type of object that the expression is trying to access is not supported.", node,
                            nullptr);
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
        spdlog::error("The value assigned to global variable '{}' does not match its "
                      "declared type.",
                      Utility::UTF32ToUTF8(node->Name()));

        throw TreeException(__FILE__, __LINE__, "The assigned value does not match the declared variable type.",
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
    if (structureDefinition == nullptr)
    {
        spdlog::error("The structure '{}' is not defined.",
                      Utility::UTF32ToUTF8(Utility::StringUtils::Join(U"::", qualifiedName)));
        throw TreeException(__FILE__, __LINE__, "The structure is not defined.", nullptr, nullptr);
    }

    return ResolveStructureDefinition(structureDefinition);
}

StructureType *TypeChecker::ResolveStructureDefinition(StructureExpression *structureDefinition)
{
    Utility::OrderPreservingMap<std::u32string, const Type *> resolvedFields;
    for (const std::u32string &fieldName : structureDefinition->Fields().GetAllKeys())
    {
        resolvedFields.AddItem(fieldName, ResolveTypeSyntax(structureDefinition->Fields().GetItemByKey(fieldName)));
    }

    /* TODO: resolve implemented interfaces */
    return Types.CreateStructureType(structureDefinition->QualifiedName(), resolvedFields, {});
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

}; /* namespace Visitors */
}; /* namespace Cygni */