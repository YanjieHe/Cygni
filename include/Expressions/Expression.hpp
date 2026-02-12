#ifndef CYGNI_EXPRESSIONS_EXPRESSION_HPP
#define CYGNI_EXPRESSIONS_EXPRESSION_HPP
#include "Expressions/SourceRange.hpp"
#include "Expressions/Type.hpp"
#include "Utility/Assert.hpp"
#include "Utility/OrderPreservingMap.hpp"
#include <cmath>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace Cygni
{
namespace Expressions
{

enum class ExpressionType
{
    Add = 0,
    And = 1,
    Assign = 2,
    Block = 3,
    Call = 4,
    Conditional = 5,
    Constant = 6,
    Convert = 7,
    Default = 8,
    Divide = 9,
    Equal = 10,
    GreaterThan = 11,
    GreaterThanOrEqual = 12,
    Halt = 13,
    Lambda = 14,
    LessThan = 15,
    LessThanOrEqual = 16,
    MemberAccess = 17,
    Modulo = 18,
    Multiply = 19,
    New = 20,
    Not = 21,
    NotEqual = 22,
    Or = 23,
    Parameter = 24,
    StructureDeclaration = 25,
    Subtract = 26,
    UnaryMinus = 27,
    UnaryPlus = 28,
    VariableDeclaration = 29,
    WhileLoop = 30,
    InterfaceDeclaration = 31
};

class Expression
{
  protected:
    SourceRange sourceRange;

  public:
    explicit Expression(SourceRange sourceRange) : sourceRange{sourceRange}
    {
    }
    virtual ~Expression() = default;
    const SourceRange &GetSourceRange() const
    {
        return sourceRange;
    }
    virtual ExpressionType NodeType() const = 0;
};

inline bool IsBinaryOperator(ExpressionType type)
{
    switch (type)
    {
    case ExpressionType::Add:
    case ExpressionType::Subtract:
    case ExpressionType::Multiply:
    case ExpressionType::Divide:
    case ExpressionType::Modulo:
    case ExpressionType::And:
    case ExpressionType::Or:
    case ExpressionType::Equal:
    case ExpressionType::NotEqual:
    case ExpressionType::GreaterThan:
    case ExpressionType::GreaterThanOrEqual:
    case ExpressionType::LessThan:
    case ExpressionType::LessThanOrEqual:
    case ExpressionType::Assign:
        return true;
    default:
        return false;
    }
}

inline bool IsUnaryOperator(ExpressionType type)
{
    switch (type)
    {
    case ExpressionType::UnaryPlus:
    case ExpressionType::UnaryMinus:
    case ExpressionType::Not:
    case ExpressionType::Convert:
        return true;
    default:
        return false;
    }
}

class TypeSyntax
{
  private:
    SourceRange sourceRange;
    std::vector<std::u32string> qualifiedName;
    std::vector<TypeSyntax *> arguments;

  public:
    TypeSyntax(SourceRange sourceRange, const std::vector<std::u32string> &qualifiedName,
               const std::vector<TypeSyntax *> &arguments)
        : sourceRange{sourceRange}, qualifiedName{qualifiedName}, arguments{arguments}
    {
        CYGNI_ASSERT(!qualifiedName.empty(), "TypeSyntax must have at least one name segment");
    }
    virtual ~TypeSyntax() = default;

    const SourceRange &GetSourceRange() const
    {
        return sourceRange;
    }

    const std::vector<std::u32string> &QualifiedName() const
    {
        return qualifiedName;
    }

    const std::vector<TypeSyntax *> &Arguments() const
    {
        return arguments;
    }
};

using ConstantValue = std::variant<int32_t, int64_t, float_t, double_t, bool, char32_t, std::u32string>;

class ConstantExpression : public Expression
{
  private:
    ConstantValue value;
    TypeCode typeCode;

  public:
    ConstantExpression(SourceRange sourceRange, ConstantValue value, TypeCode typeCode)
        : Expression(sourceRange), value{value}, typeCode{typeCode}
    {
        CYGNI_ASSERT(Validate(typeCode), "Constant value type does not match the specified TypeCode");
    }

    ExpressionType NodeType() const override
    {
        return ExpressionType::Constant;
    }

    const ConstantValue &Value() const
    {
        return value;
    }

    TypeCode GetTypeCode() const
    {
        return typeCode;
    }

  private:
    bool Validate(TypeCode typeCode)
    {
        return std::visit(
            [&](auto &&v) -> bool {
                using T = std::decay_t<decltype(v)>;

                switch (typeCode)
                {
                case TypeCode::Int32:
                    return std::is_same_v<T, int32_t>;
                case TypeCode::Int64:
                    return std::is_same_v<T, int64_t>;
                case TypeCode::Float32:
                    return std::is_same_v<T, float_t>;
                case TypeCode::Float64:
                    return std::is_same_v<T, double_t>;
                case TypeCode::Boolean:
                    return std::is_same_v<T, bool>;
                case TypeCode::Char:
                    return std::is_same_v<T, char32_t>;
                case TypeCode::String:
                    return std::is_same_v<T, std::u32string>;
                default:
                    return false;
                }
            },
            value);
    }
};

class BinaryExpression : public Expression
{
  private:
    ExpressionType nodeType;
    Expression *left;
    Expression *right;

  public:
    BinaryExpression(SourceRange sourceRange, ExpressionType nodeType, Expression *left, Expression *right)
        : Expression(sourceRange), nodeType(nodeType), left(left), right(right)
    {
        CYGNI_ASSERT(left && right, "BinaryExpression operands must not be null");
        CYGNI_ASSERT(IsBinaryOperator(nodeType), "ExpressionType is not a binary operator");
    }

    ExpressionType NodeType() const override
    {
        return nodeType;
    }

    Expression *Left() const
    {
        return left;
    }

    Expression *Right() const
    {
        return right;
    }
};

class UnaryExpression : public Expression
{
  private:
    ExpressionType nodeType;
    Expression *operand;
    TypeSyntax *targetTypeSyntax;

  public:
    UnaryExpression(SourceRange sourceRange, ExpressionType nodeType, Expression *operand, TypeSyntax *targetTypeSyntax)
        : Expression(sourceRange), nodeType(nodeType), operand(operand), targetTypeSyntax{targetTypeSyntax}
    {
        CYGNI_ASSERT(operand, "UnaryExpression operand must not be null");
        CYGNI_ASSERT(IsUnaryOperator(nodeType), "ExpressionType is not a unary operator");
    }

    ExpressionType NodeType() const override
    {
        return nodeType;
    }

    Expression *Operand() const
    {
        return operand;
    }

    const TypeSyntax *GetTargetTypeSyntax() const
    {
        return targetTypeSyntax;
    }
};

class ParameterExpression : public Expression
{
  private:
    std::vector<std::u32string> qualifiedName;
    TypeSyntax *typeSyntax;

  public:
    ParameterExpression(SourceRange sourceRange, std::vector<std::u32string> qualifiedName, TypeSyntax *typeSyntax)
        : Expression(sourceRange), qualifiedName(qualifiedName), typeSyntax{typeSyntax}
    {
        CYGNI_ASSERT(!qualifiedName.empty(), "ParameterExpression qualified name must not be empty");
    }

    ExpressionType NodeType() const override
    {
        return ExpressionType::Parameter;
    }

    const std::vector<std::u32string> &QualifiedName() const
    {
        return qualifiedName;
    }

    const std::u32string &Name() const
    {
        return qualifiedName.back();
    }

    const TypeSyntax *GetTypeSyntax() const
    {
        return typeSyntax;
    }
};

class VariableDeclarationExpression : public Expression
{
  private:
    std::u32string name;
    TypeSyntax *typeSyntax;
    Expression *initializer;

  public:
    VariableDeclarationExpression(SourceRange sourceRange, const std::u32string &name, TypeSyntax *typeSyntax,
                                  Expression *initializer)
        : Expression(sourceRange), name{name}, typeSyntax{typeSyntax}, initializer{initializer}
    {
    }

    ExpressionType NodeType() const override
    {
        return ExpressionType::VariableDeclaration;
    }

    const std::u32string &Name() const
    {
        return name;
    }

    Expression *Initializer() const
    {
        return initializer;
    }

    const TypeSyntax *GetTypeSyntax() const
    {
        return typeSyntax;
    }
};

class BlockExpression : public Expression
{
  private:
    std::vector<Expression *> expressions;

  public:
    BlockExpression(SourceRange sourceRange, const std::vector<Expression *> &expressions)
        : Expression(sourceRange), expressions{expressions}
    {
    }

    ExpressionType NodeType() const override
    {
        return ExpressionType::Block;
    }

    const std::vector<Expression *> &Expressions() const
    {
        return expressions;
    }
};

class ConditionalExpression : public Expression
{
  private:
    Expression *test;
    Expression *ifTrue;
    Expression *ifFalse;

  public:
    ConditionalExpression(SourceRange sourceRange, Expression *test, Expression *ifTrue, Expression *ifFalse)
        : Expression(sourceRange), test(test), ifTrue(ifTrue), ifFalse(ifFalse)
    {
    }

    ExpressionType NodeType() const override
    {
        return ExpressionType::Conditional;
    }

    Expression *Test() const
    {
        return test;
    }

    Expression *IfTrue() const
    {
        return ifTrue;
    }

    Expression *IfFalse() const
    {
        return ifFalse;
    }
};

class CallExpression : public Expression
{
  private:
    Expression *function;
    std::vector<Expression *> arguments;

  public:
    CallExpression(SourceRange sourceRange, Expression *function, const std::vector<Expression *> &arguments)
        : Expression(sourceRange), function(function), arguments(arguments)
    {
    }

    ExpressionType NodeType() const override
    {
        return ExpressionType::Call;
    }

    Expression *Function() const
    {
        return function;
    }

    const std::vector<Expression *> &Arguments() const
    {
        return arguments;
    }
};

using AnnotationValue = std::variant<int32_t, int64_t, float_t, double_t, bool, char32_t, std::u32string>;

class AnnotationArgument
{
  private:
    std::u32string name;
    AnnotationValue value;

  public:
    AnnotationArgument() = default;

    AnnotationArgument(std::u32string name, AnnotationValue value) : name{name}, value{value}
    {
    }

    const std::u32string &Name() const
    {
        return name;
    }

    const AnnotationValue &Value() const
    {
        return value;
    }
};

class Annotation
{
  private:
    std::u32string name;
    std::vector<AnnotationArgument> arguments;

  public:
    Annotation() : name{}, arguments{}
    {
    }
    Annotation(std::u32string name, std::vector<AnnotationArgument> arguments) : name{name}, arguments{arguments}
    {
    }
    const std::u32string &Name() const
    {
        return name;
    }
    const std::vector<AnnotationArgument> &Arguments() const
    {
        return arguments;
    }
};

class LambdaExpression : public Expression
{
  private:
    std::u32string name;
    Expression *body;
    std::vector<ParameterExpression *> parameters;
    TypeSyntax *returnType;
    std::vector<Annotation> annotations;

  public:
    LambdaExpression(SourceRange sourceRange, std::u32string name, Expression *body,
                     const std::vector<ParameterExpression *> &parameters, TypeSyntax *returnType,
                     std::vector<Annotation> annotations)
        : Expression(sourceRange), name{name}, body{body}, parameters{parameters}, returnType{returnType},
          annotations{annotations}
    {
    }

    ExpressionType NodeType() const override
    {
        return ExpressionType::Lambda;
    }

    const std::u32string &Name() const
    {
        return name;
    }

    Expression *Body() const
    {
        return body;
    }

    const std::vector<ParameterExpression *> &Parameters() const
    {
        return parameters;
    }

    const TypeSyntax *ReturnTypeSyntax() const
    {
        return returnType;
    }

    const std::vector<Annotation> &Annotations() const
    {
        return annotations;
    }

    bool IsNativeFunction() const
    {
        for (const auto &annotation : Annotations())
        {
            if (annotation.Name() == U"External")
            {

                return true;
            }
        }

        return false;
    }
};

class WhileLoopExpression : public Expression
{
  private:
    Expression *condition;
    Expression *body;

  public:
    WhileLoopExpression(SourceRange sourceRange, Expression *condition, Expression *body)
        : Expression(sourceRange), condition{condition}, body{body}
    {
    }

    ExpressionType NodeType() const override
    {
        return ExpressionType::WhileLoop;
    }

    Expression *Condition() const
    {
        return condition;
    }

    Expression *Body() const
    {
        return body;
    }
};

class DefaultExpression : public Expression
{
  private:
    TypeSyntax *typeSyntax;

  public:
    DefaultExpression(SourceRange sourceRange, TypeSyntax *typeSyntax) : Expression(sourceRange), typeSyntax{typeSyntax}
    {
    }

    ExpressionType NodeType() const override
    {
        return ExpressionType::Default;
    }

    const TypeSyntax *GetTypeSyntax() const
    {
        return typeSyntax;
    }
};

class NewExpression : public Expression
{
  private:
    TypeSyntax *typeSyntax;
    Utility::OrderPreservingMap<std::u32string, Expression *> fieldsInitialization;

  public:
    NewExpression(SourceRange sourceRange, TypeSyntax *typeSyntax,
                  Utility::OrderPreservingMap<std::u32string, Expression *> fieldsInitialization)
        : Expression(sourceRange), typeSyntax{typeSyntax}, fieldsInitialization{fieldsInitialization}
    {
    }

    ExpressionType NodeType() const override
    {
        return ExpressionType::New;
    }

    const TypeSyntax *GetTypeSyntax() const
    {
        return typeSyntax;
    }

    const Utility::OrderPreservingMap<std::u32string, Expression *> &FieldsInitialization() const
    {
        return fieldsInitialization;
    }
};

class MemberExpression : public Expression
{
  private:
    Expression *expression;
    std::u32string fieldName;

  public:
    MemberExpression(SourceRange sourceRange, Expression *expression, std::u32string fieldName)
        : Expression(sourceRange), expression{expression}, fieldName{fieldName}
    {
    }

    ExpressionType NodeType() const override
    {
        return ExpressionType::MemberAccess;
    }
    Expression *GetExpression() const
    {
        return expression;
    }
    const std::u32string &FieldName() const
    {
        return fieldName;
    }
};

class StructureExpression : public Expression
{
  private:
    std::vector<std::u32string> qualifiedName;
    Utility::OrderPreservingMap<std::u32string, TypeSyntax *> fields;
    Utility::OrderPreservingMap<std::u32string, LambdaExpression *> methods;
    std::vector<TypeSyntax *> interfaces;

  public:
    StructureExpression(SourceRange sourceRange, const std::vector<std::u32string> &qualifiedName,
                        const Utility::OrderPreservingMap<std::u32string, TypeSyntax *> &fields,
                        const Utility::OrderPreservingMap<std::u32string, LambdaExpression *> &methods,
                        const std::vector<TypeSyntax *> &interfaces)
        : Expression(sourceRange), qualifiedName{qualifiedName}, fields{fields}, methods{methods},
          interfaces{interfaces}
    {
    }

    ExpressionType NodeType() const override
    {
        return ExpressionType::StructureDeclaration;
    }

    const std::vector<std::u32string> &QualifiedName() const
    {
        return qualifiedName;
    }

    const Utility::OrderPreservingMap<std::u32string, TypeSyntax *> &Fields() const
    {
        return fields;
    }

    const Utility::OrderPreservingMap<std::u32string, LambdaExpression *> &Methods() const
    {
        return methods;
    }

    const std::vector<TypeSyntax *> &Interfaces() const
    {
        return interfaces;
    }
};

class InterfaceExpression : public Expression
{
  private:
    std::vector<std::u32string> qualifiedName;
    std::vector<TypeSyntax *> baseInterfaces;
    Utility::OrderPreservingMap<std::u32string, LambdaExpression *> methods;

  public:
    InterfaceExpression(SourceRange sourceRange, const std::vector<std::u32string> qualifiedName,
                        const std::vector<TypeSyntax *> &baseInterfaces,
                        const Utility::OrderPreservingMap<std::u32string, LambdaExpression *> &methods)
        : Expression(sourceRange), qualifiedName{qualifiedName}, baseInterfaces{baseInterfaces}, methods{methods}
    {
    }

    ExpressionType NodeType() const override
    {
        return ExpressionType::InterfaceDeclaration;
    }

    const std::vector<std::u32string> &QualifiedName() const
    {
        return qualifiedName;
    }

    const std::vector<TypeSyntax *> &BaseInterfaces() const
    {
        return baseInterfaces;
    }

    const Utility::OrderPreservingMap<std::u32string, LambdaExpression *> &Methods() const
    {
        return methods;
    }
};

class ExpressionFactory
{
  private:
    std::vector<Expression *> nodes;

  public:
    ExpressionFactory() = default;
    ExpressionFactory(const ExpressionFactory &) = delete;
    ~ExpressionFactory()
    {
        for (auto node : nodes)
        {
            delete node;
        }
    }
    template <typename TExpression, typename... ArgTypes>
    TExpression *Create(ArgTypes... arguments)
    {
        static_assert(std::is_base_of_v<Expression, TExpression>, "ExpressionFactory can only create Expression nodes");
        auto node = new TExpression(arguments...);
        nodes.push_back(node);
        return node;
    }
};

class TypeSyntaxFactory
{
  private:
    std::vector<TypeSyntax *> nodes;

  public:
    TypeSyntaxFactory() = default;
    TypeSyntaxFactory(const TypeSyntaxFactory &) = delete;

    ~TypeSyntaxFactory()
    {
        for (auto *node : nodes)
        {
            delete node;
        }
    }

    TypeSyntax *Create(SourceRange sourceRange, std::vector<std::u32string> qualifiedName,
                       std::vector<TypeSyntax *> arguments)
    {
        auto *node = new TypeSyntax(sourceRange, std::move(qualifiedName), std::move(arguments));

        nodes.push_back(node);
        return node;
    }
};

}; /* namespace Expressions */
}; /* namespace Cygni */

#endif /* CYGNI_EXPRESSIONS_EXPRESSION_HPP */