#ifndef CYGNI_EXPRESSIONS_EXPRESSION_HPP
#define CYGNI_EXPRESSIONS_EXPRESSION_HPP
#include "Expressions/Type.hpp"
#include "Expressions/SourceRange.hpp"
#include <any>
#include <vector>
#include <unordered_map>

namespace Cygni {
namespace Expressions {

enum class ExpressionType {
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
  Modulo = 17,
  Multiply = 18,
  Not = 19,
  NotEqual = 20,
  Or = 21,
  Parameter = 22,
  Subtract = 23,
  UnaryMinus = 24,
  UnaryPlus = 25,
  VariableDeclaration = 26,
  WhileLoop = 27
};

class Expression {
protected:
  SourceRange sourceRange;

public:
  explicit Expression(SourceRange sourceRange) : sourceRange{sourceRange} {}
  const SourceRange &GetSourceRange() const { return sourceRange; }
  virtual ExpressionType NodeType() const = 0;
};

class ConstantExpression : public Expression {
private:
  std::any value;
  TypeCode typeCode;

public:
  ConstantExpression(SourceRange sourceRange, std::any value, TypeCode typeCode)
      : Expression(sourceRange), value{value}, typeCode{typeCode} {}

  ExpressionType NodeType() const override { return ExpressionType::Constant; }

  const std::any &Value() const { return value; }

  TypeCode GetTypeCode() const { return typeCode; }
};

class BinaryExpression : public Expression {
private:
  ExpressionType nodeType;
  Expression *left;
  Expression *right;

public:
  BinaryExpression(SourceRange sourceRange, ExpressionType nodeType,
                   Expression *left, Expression *right)
      : Expression(sourceRange), nodeType(nodeType), left(left), right(right) {}

  ExpressionType NodeType() const override { return nodeType; }

  const Expression *Left() const { return left; }

  const Expression *Right() const { return right; }
};

class UnaryExpression : public Expression {
private:
  ExpressionType nodeType;
  Expression *operand;
  Type *type;

public:
  UnaryExpression(SourceRange sourceRange, ExpressionType nodeType,
                  Expression *operand, Type *type)
      : Expression(sourceRange), nodeType(nodeType), operand(operand),
        type{type} {}

  ExpressionType NodeType() const override { return nodeType; }

  const Expression *Operand() const { return operand; }

  const Type *GetType() const { return type; }
};

class ParameterExpression : public Expression {
private:
  std::vector<std::u32string> prefix;
  std::u32string name;
  Type *type;

public:
  ParameterExpression(SourceRange sourceRange, const std::u32string &name,
                      Type *type)
      : Expression(sourceRange), name(name), type{type} {}
  ParameterExpression(SourceRange sourceRange,
                      std::vector<std::u32string> prefix,
                      const std::u32string &name, Type *type)
      : Expression(sourceRange), name(name), type{type} {}

  ExpressionType NodeType() const override { return ExpressionType::Parameter; }

  const std::vector<std::u32string> &Prefix() const { return prefix; }

  const std::u32string &Name() const { return name; }

  const Type *GetType() const { return type; }
};

class VariableDeclarationExpression : public Expression {
private:
  std::u32string name;
  Type *type;
  Expression *initializer;

public:
  VariableDeclarationExpression(SourceRange sourceRange,
                                const std::u32string &name, Type *type,
                                Expression *initializer)
      : Expression(sourceRange), name{name}, type{type},
        initializer{initializer} {}

  ExpressionType NodeType() const override {
    return ExpressionType::VariableDeclaration;
  }

  const std::u32string &Name() const { return name; }

  const Expression *Initializer() const { return initializer; }

  const Type *GetType() const { return type; }
};

class BlockExpression : public Expression {
private:
  std::vector<Expression *> expressions;
  std::vector<ParameterExpression *> variables;

public:
  BlockExpression(SourceRange sourceRange,
                  const std::vector<Expression *> &expressions)
      : Expression(sourceRange), expressions{expressions} {}

  ExpressionType NodeType() const override { return ExpressionType::Block; }

  const std::vector<Expression *> &Expressions() const { return expressions; }
};

class ConditionalExpression : public Expression {
private:
  Expression *test;
  Expression *ifTrue;
  Expression *ifFalse;

public:
  ConditionalExpression(SourceRange sourceRange, Expression *test,
                        Expression *ifTrue, Expression *ifFalse)
      : Expression(sourceRange), test(test), ifTrue(ifTrue), ifFalse(ifFalse) {}

  ExpressionType NodeType() const override {
    return ExpressionType::Conditional;
  }

  const Expression *Test() const { return test; }

  const Expression *IfTrue() const { return ifTrue; }

  const Expression *IfFalse() const { return ifFalse; }
};

class CallExpression : public Expression {
private:
  Expression *function;
  std::vector<Expression *> arguments;

public:
  CallExpression(SourceRange sourceRange, Expression *function,
                 const std::vector<Expression *> &arguments)
      : Expression(sourceRange), function(function), arguments(arguments) {}

  ExpressionType NodeType() const override { return ExpressionType::Call; }

  const Expression *Function() const { return function; }

  const std::vector<Expression *> &Arguments() const { return arguments; }
};

class AnnotationArgument {
private:
  std::u32string name;
  std::any value;

public:
  AnnotationArgument() = default;
  AnnotationArgument(std::u32string name, std::any value)
      : name{name}, value{value} {}

  const std::u32string &Name() const { return name; }
  const std::any &Value() const { return value; }
};

class Annotation {
private:
  std::u32string name;
  std::vector<AnnotationArgument> arguments;

public:
  Annotation() : name{}, arguments{} {}
  Annotation(std::u32string name, std::vector<AnnotationArgument> arguments)
      : name{name}, arguments{arguments} {}
  const std::u32string &Name() const { return name; }
  const std::vector<AnnotationArgument> Arguments() const { return arguments; }
};

class LambdaExpression : public Expression {
private:
  std::u32string name;
  const Expression *body;
  std::vector<ParameterExpression *> parameters;
  const Type *returnType;
  std::vector<Annotation> annotations;

public:
  LambdaExpression(SourceRange sourceRange, std::u32string name,
                   const Expression *body,
                   const std::vector<ParameterExpression *> &parameters,
                   const Type *returnType, std::vector<Annotation> annotations)
      : Expression(sourceRange), name{name}, body{body}, parameters{parameters},
        returnType{returnType}, annotations{annotations} {}

  ExpressionType NodeType() const override { return ExpressionType::Lambda; }

  const std::u32string &Name() const { return name; }

  const Expression *Body() const { return body; }

  const std::vector<ParameterExpression *> &Parameters() const {
    return parameters;
  }

  const Type *ReturnType() const { return returnType; }

  CallableType *GetCallableType(TypeFactory &typeFactory) const {
    std::vector<const Type *> parameterTypes;
    for (const auto &parameter : Parameters()) {
      parameterTypes.push_back(parameter->GetType());
    }
    CallableType *callableType =
        typeFactory.CreateCallableType(parameterTypes, ReturnType());

    return callableType;
  }

  const std::vector<Annotation> &Annotations() const { return annotations; }

  bool IsNativeFunction() const {
    for (auto annotation : Annotations()) {
      if (annotation.Name() == U"External") {

        return true;
      }
    }

    return false;
  }
};

class WhileLoopExpression : public Expression {
private:
  Expression *condition;
  Expression *body;

public:
  WhileLoopExpression(SourceRange sourceRange, Expression *condition,
                      Expression *body)
      : Expression(sourceRange), condition{condition}, body{body} {}

  ExpressionType NodeType() const override { return ExpressionType::WhileLoop; }

  const Expression *Condition() const { return condition; }

  const Expression *Body() const { return body; }
};

class DefaultExpression : public Expression {
private:
  Type *type;

public:
  DefaultExpression(SourceRange sourceRange, Type *type)
      : Expression(sourceRange), type{type} {}

  ExpressionType NodeType() const override { return ExpressionType::Default; }

  const Type *GetType() const { return type; }
};

class ExpressionFactory {
private:
  std::vector<Expression *> nodes;

public:
  ExpressionFactory() = default;
  ExpressionFactory(const ExpressionFactory &) = delete;
  ~ExpressionFactory() {
    for (auto node : nodes) {
      delete node;
    }
  }
  template <typename TExpression, typename... ArgTypes>
  TExpression *Create(ArgTypes... arguments) {
    auto node = new TExpression(arguments...);
    nodes.push_back(node);
    return node;
  }
};

}; /* namespace Expressions */
}; /* namespace Cygni */

#endif /* CYGNI_EXPRESSIONS_EXPRESSION_HPP */