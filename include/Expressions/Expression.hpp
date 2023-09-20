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
  Divide = 8,
  Equal = 9,
  GreaterThan = 10,
  GreaterThanOrEqual = 11,
  Lambda = 12,
  LessThan = 13,
  LessThanOrEqual = 14,
  Modulo = 15,
  Multiply = 16,
  Not = 17,
  NotEqual = 18,
  Or = 19,
  Parameter = 20,
  Subtract = 21,
  Halt = 22,
  Loop = 23,
  Default = 24,
  UnaryPlus = 25,
  UnaryMinus = 26,
  VariableDeclaration,
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
  std::u32string name;
  Type *type;

public:
  ParameterExpression(SourceRange sourceRange, const std::u32string &name,
                      Type *type)
      : Expression(sourceRange), name(name), type{type} {}

  ExpressionType NodeType() const override { return ExpressionType::Parameter; }

  const std::u32string &Name() const { return name; }

  const Type *GetType() const { return type; }
};

class VariableDeclarationExpression : public Expression {
private:
  std::u32string name;
  Expression *initializer;

public:
  VariableDeclarationExpression(SourceRange sourceRange,
                                const std::u32string &name,
                                Expression *initializer)
      : Expression(sourceRange), name(name), initializer(initializer) {}

  ExpressionType NodeType() const override {
    return ExpressionType::VariableDeclaration;
  }

  const std::u32string &Name() const { return name; }

  const Expression *Initializer() const { return initializer; }
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

class LambdaExpression : public Expression {
private:
  Expression *body;
  std::vector<ParameterExpression *> parameters;
  Type *returnType;

public:
  LambdaExpression(SourceRange sourceRange, Expression *body,
                   const std::vector<ParameterExpression *> &parameters,
                   Type *returnType)
      : Expression(sourceRange), body{body}, parameters{parameters},
        returnType{returnType} {}

  ExpressionType NodeType() const override { return ExpressionType::Lambda; }

  const Expression *Body() const { return body; }

  const std::vector<ParameterExpression *> &Parameters() const {
    return parameters;
  }

  const Type *ReturnType() const { return returnType; }
};

class LoopExpression : public Expression {
private:
  Expression *initializer;
  Expression *condition;
  Expression *body;

public:
  LoopExpression(SourceRange sourceRange, Expression *initializer,
                 Expression *condition, Expression *body)
      : Expression(sourceRange), initializer{initializer}, condition{condition},
        body{body} {}

  ExpressionType NodeType() const override { return ExpressionType::Loop; }

  const Expression *Initializer() const { return initializer; }

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