#include "Visitors/TypeChecker.hpp"

#include <spdlog/spdlog.h>
#include "Utility/UTF32Functions.hpp"

#include <iostream>
using std::cout;
using std::endl;

namespace Cygni {
namespace Visitors {

TypeChecker::TypeChecker() { spdlog::debug("Type checker initialized."); }

const Type *TypeChecker::VisitBinary(const BinaryExpression *node,
                                     Scope<const Type *> *scope) {
  spdlog::debug("{}: {}", __FUNCTION__, (int)node->NodeType());
  if (node->NodeType() == ExpressionType::Assign) {
    const Type *right = Visit(node->Right(), scope);
    if (node->Left()->NodeType() == ExpressionType::Parameter) {
      const Type *left = Visit(node->Left(), scope);

      if (TypeFactory::AreTypesEqual(left, right)) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Empty));
      } else {
        throw TreeException(__FILE__, __LINE__, "type mismatch error.", node,
                            nullptr);
      }
    } else {
      throw TreeException(__FILE__, __LINE__,
                          "type checking not supported error.", node, nullptr);
    }
  } else {
    const Type *left = Visit(node->Left(), scope);
    const Type *right = Visit(node->Right(), scope);
    switch (node->NodeType()) {
    case ExpressionType::Add:
    case ExpressionType::Subtract:
    case ExpressionType::Multiply:
    case ExpressionType::Divide: {
      if (left->GetTypeCode() == TypeCode::Int32 &&
          right->GetTypeCode() == TypeCode::Int32) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Int32));
      } else if (left->GetTypeCode() == TypeCode::Int64 &&
                 right->GetTypeCode() == TypeCode::Int64) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Int64));
      } else if (left->GetTypeCode() == TypeCode::Float32 &&
                 right->GetTypeCode() == TypeCode::Float32) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Float32));
      } else if (left->GetTypeCode() == TypeCode::Float64 &&
                 right->GetTypeCode() == TypeCode::Float64) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Float64));
      } else {
        throw TreeException(__FILE__, __LINE__, "type mismatch error.", node,
                            nullptr);
      }
    }
    case ExpressionType::GreaterThan:
    case ExpressionType::LessThan:
    case ExpressionType::GreaterThanOrEqual:
    case ExpressionType::LessThanOrEqual: {
      if (left->GetTypeCode() == TypeCode::Int32 &&
          right->GetTypeCode() == TypeCode::Int32) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
      } else if (left->GetTypeCode() == TypeCode::Int64 &&
                 right->GetTypeCode() == TypeCode::Int64) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
      } else if (left->GetTypeCode() == TypeCode::Float32 &&
                 right->GetTypeCode() == TypeCode::Float32) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
      } else if (left->GetTypeCode() == TypeCode::Float64 &&
                 right->GetTypeCode() == TypeCode::Float64) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
      } else if (left->GetTypeCode() == TypeCode::Char &&
                 right->GetTypeCode() == TypeCode::Char) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
      } else if (left->GetTypeCode() == TypeCode::String &&
                 right->GetTypeCode() == TypeCode::String) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
      } else {
        throw TreeException(__FILE__, __LINE__, "type mismatch error.", node,
                            nullptr);
      }
    }
    case ExpressionType::Equal:
    case ExpressionType::NotEqual: {
      if (left->GetTypeCode() == TypeCode::Int32 &&
          right->GetTypeCode() == TypeCode::Int32) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
      } else if (left->GetTypeCode() == TypeCode::Int64 &&
                 right->GetTypeCode() == TypeCode::Int64) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
      } else if (left->GetTypeCode() == TypeCode::Float32 &&
                 right->GetTypeCode() == TypeCode::Float32) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
      } else if (left->GetTypeCode() == TypeCode::Float64 &&
                 right->GetTypeCode() == TypeCode::Float64) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
      } else if (left->GetTypeCode() == TypeCode::Boolean &&
                 right->GetTypeCode() == TypeCode::Boolean) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
      } else if (left->GetTypeCode() == TypeCode::Char &&
                 right->GetTypeCode() == TypeCode::Char) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
      } else if (left->GetTypeCode() == TypeCode::String &&
                 right->GetTypeCode() == TypeCode::String) {
        return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
      } else {
        throw TreeException(__FILE__, __LINE__, "type mismatch error.", node,
                            nullptr);
      }
    }
    default: {
      throw TreeException(__FILE__, __LINE__, "type mismatch error.", node,
                          nullptr);
    }
    }
  }
}

const Type *TypeChecker::VisitConstant(const ConstantExpression *node,
                                       Scope<const Type *> *scope) {
  return Register(node, TypeFactory::CreateBasicType(node->GetTypeCode()));
}

const Type *TypeChecker::VisitParameter(const ParameterExpression *node,
                                        Scope<const Type *> *scope) {
  if (scope->Exists(node->Name())) {
    const Type *type = scope->Get(node->Name());
    return Register(node, type);
  } else {
    throw TreeException(
        __FILE__, __LINE__,
        Utility::UTF32ToUTF8(U"'" + node->Name() + U"' not defined."), node,
        nullptr);
  }
}

const Type *TypeChecker::VisitBlock(const BlockExpression *node,
                                    Scope<const Type *> *parent) {
  Scope<const Type *> scope(parent);
  const Type *type = TypeFactory::CreateBasicType(TypeCode::Empty);
  for (const auto &expression : node->Expressions()) {
    type = Visit(expression, &scope);
  }
  return Register(node, type);
}

const Type *TypeChecker::VisitConditional(const ConditionalExpression *node,
                                          Scope<const Type *> *scope) {
  const Type *test = Visit(node->Test(), scope);
  if (test->GetTypeCode() == TypeCode::Boolean) {
    const Type *ifTrue = Visit(node->IfTrue(), scope);
    const Type *ifFalse = Visit(node->IfFalse(), scope);
    return Register(node, Types.CreateUnionType(ifTrue, ifFalse));
  } else {
    throw TreeException(__FILE__, __LINE__, "The type of condition of the "
                                            "conditional expression must be a "
                                            "boolean type.",
                        node, nullptr);
  }
}

const Type *TypeChecker::VisitUnary(const UnaryExpression *node,
                                    Scope<const Type *> *scope) {
  const Type *operand = Visit(node->Operand(), scope);
  switch (node->NodeType()) {
  case ExpressionType::Not: {
    if (operand->GetTypeCode() == TypeCode::Boolean) {
      return Register(node, TypeFactory::CreateBasicType(TypeCode::Boolean));
    } else {
      throw TreeException(__FILE__, __LINE__, "type mismatch error.", node,
                          nullptr);
    }
  }
  case ExpressionType::Halt: {
    if (operand->GetTypeCode() == TypeCode::Int32) {
      return Register(node, TypeFactory::CreateBasicType(TypeCode::Empty));
    } else {
      throw TreeException(__FILE__, __LINE__, "type mismatch error.", node,
                          nullptr);
    }
  }
  case ExpressionType::Convert: {
    if (TypeFactory::AreTypesEqual(operand, node->GetType())) {
      return Register(node, operand);
    } else {
      if (operand->GetTypeCode() == TypeCode::Int32 ||
          operand->GetTypeCode() == TypeCode::Int64 ||
          operand->GetTypeCode() == TypeCode::Float32 ||
          operand->GetTypeCode() == TypeCode::Float64 ||
          operand->GetTypeCode() == TypeCode::Boolean ||
          operand->GetTypeCode() == TypeCode::Char) {
        if (node->GetType()->GetTypeCode() == TypeCode::Int32 ||
            node->GetType()->GetTypeCode() == TypeCode::Int64 ||
            node->GetType()->GetTypeCode() == TypeCode::Float32 ||
            node->GetType()->GetTypeCode() == TypeCode::Float64 ||
            node->GetType()->GetTypeCode() == TypeCode::Boolean ||
            node->GetType()->GetTypeCode() == TypeCode::Char) {
          return Register(node, node->GetType());
        }
      }
      throw TreeException(__FILE__, __LINE__, "type mismatch error.", node,
                          nullptr);
    }
  }
  default: {
    throw TreeException(__FILE__, __LINE__, "type mismatch error.", node,
                        nullptr);
  }
  }
}

const Type *TypeChecker::VisitCall(const CallExpression *node,
                                   Scope<const Type *> *scope) {
  auto callableType = Visit(node->Function(), scope);
  if (callableType->GetTypeCode() == TypeCode::Callable) {
    auto t = static_cast<const CallableType *>(callableType);
    if (t->Arguments().size() == node->Arguments().size()) {
      for (size_t i = 0; i < node->Arguments().size(); i++) {
        auto argType = Visit(node->Arguments().at(i), scope);
        if (!TypeFactory::AreTypesEqual(argType, t->Arguments().at(i))) {
          throw TreeException(__FILE__, __LINE__,
                              "argument " + std::to_string(i) +
                                  " type mismatch error.",
                              node, nullptr);
        }
      }
      return Register(node, t->GetReturnType());
    } else {
      throw TreeException(__FILE__, __LINE__, "argument size mismatch error.",
                          node, nullptr);
    }
  } else {
    throw TreeException(
        __FILE__, __LINE__,
        "The type of function of the call expression must be a callable type.",
        node, nullptr);
  }
}

const Type *TypeChecker::VisitLambda(const LambdaExpression *node,
                                     Scope<const Type *> *parent) {
  Scope<const Type *> scope(parent);
  std::vector<const Type *> argumentTypes;
  argumentTypes.reserve(node->Parameters().size());
  for (const auto &parameter : node->Parameters()) {
    spdlog::debug("Type checker declares parameter \"{}\".",
                  Utility::UTF32ToUTF8(parameter->Name()));
    scope.Declare(parameter->Name(), parameter->GetType());
    argumentTypes.push_back(parameter->GetType());
  }
  const Type *returnType = Visit(node->Body(), &scope);
  return Types.CreateCallableType(argumentTypes, returnType);
}

const Type *TypeChecker::VisitLoop(const LoopExpression *node,
                                   Scope<const Type *> *parent) {
  Scope<const Type *> scope(parent);
  Visit(node->Initializer(), &scope);
  const Type *type = Visit(node->Condition(), &scope);
  if (type->GetTypeCode() == TypeCode::Boolean) {
    return Visit(node->Body(), &scope);
  } else {
    throw TreeException(
        __FILE__, __LINE__,
        "The condition of the loop expression must return a boolean value.",
        node, nullptr);
  }
}

const Type *TypeChecker::VisitDefault(const DefaultExpression *node,
                                      Scope<const Type *> *scope) {
  return node->GetType();
}

const Type *
TypeChecker::VisitVariableDeclaration(const VariableDeclarationExpression *node,
                                      Scope<const Type *> *scope) {
  const Type* initializer = Visit(node->Initializer(), scope);
  scope->Declare(node->Name(), initializer);
  
  return TypeFactory::CreateBasicType(TypeCode::Empty);
}

const Type *TypeChecker::GetType(const Expression *node) {
  return nodeTypes.at(node);
}

const Type *TypeChecker::Register(const Expression *node, const Type *type) {
  nodeTypes[node] = type;
  return type;
}

}; /* namespace Visitors */
}; /* namespace Cygni */