#include "SyntaxAnalysis/Parser.hpp"

#include <magic_enum/magic_enum.hpp>

#include "Utility/Format.hpp"
#include "Utility/UTF32Functions.hpp"
#include "SyntaxAnalysis/ParserException.hpp"

using std::unordered_map;
using std::vector;

namespace Cygni {
namespace SyntaxAnalysis {

using LexicalAnalysis::Token;
using LexicalAnalysis::TokenTag;
using LexicalAnalysis::SourceCodeFile;
using namespace Expressions;

using Utility::Format;

Parser::Parser(std::vector<Token> tokens,
               std::shared_ptr<SourceCodeFile> document)
    : tokens{tokens}, document{document}, offset{0} {}

const Token &Parser::Match(TokenTag tag) {
  if (tag == Look().tag) {
    const Token &t = Look();
    Advance();
    return t;
  } else {
    auto sv = magic_enum::enum_name(tag);
    std::string tagStr(sv.begin(), sv.end());
    sv = magic_enum::enum_name(Look().tag);
    std::string lookTagStr(sv.begin(), sv.end());

    throw ParserException(
        __FILE__, __LINE__,
        SourceRange(document, Look().line, Look().column, Look().line,
                    Look().column + Look().text.size()),
        "Expecting '" + tagStr + "', got '" + lookTagStr + "'.", nullptr);
  }
}

ExpPtr Parser::Statement() {
  switch (Look().tag) {
  case TokenTag::If:
    return IfStatement();
  case TokenTag::While:
    return WhileStatement();
  case TokenTag::Var: {
    auto variableDeclaration = VariableDeclarationStatement();
    Match(TokenTag::Semicolon);
    return variableDeclaration;
  }
  default: {
    auto expression = ParseAssign();
    Match(TokenTag::Semicolon);
    return expression;
  }
  }
}

ExpPtr Parser::ParseAssign() {
  const Token &start = Look();
  auto x = ParseOr();
  if (Look().tag == TokenTag::Assign) {
    Match(TokenTag::Assign);
    auto y = ParseOr();
    return expressionFactory.Create<BinaryExpression>(
        Pos(start), ExpressionType::Assign, x, y);
  } else {
    return x;
  }
}

ExpPtr Parser::ParseOr() {
  const Token &start = Look();
  auto x = ParseAnd();
  while (Look().tag == TokenTag::Or) {
    Match(TokenTag::Or);
    auto y = ParseAnd();
    x = expressionFactory.Create<BinaryExpression>(Pos(start),
                                                   ExpressionType::Or, x, y);
  }
  return x;
}

ExpPtr Parser::ParseAnd() {
  const Token &start = Look();
  auto x = ParseEquality();
  while (Look().tag == TokenTag::And) {
    Match(TokenTag::And);
    auto y = ParseEquality();
    x = expressionFactory.Create<BinaryExpression>(Pos(start),
                                                   ExpressionType::And, x, y);
  }
  return x;
}

ExpPtr Parser::ParseEquality() {
  const Token &start = Look();
  auto x = ParseRelation();
  while (Look().tag == TokenTag::Equal || Look().tag == TokenTag::NotEqual) {
    Token t = Look();
    Advance();
    auto y = ParseRelation();
    if (t.tag == TokenTag::Equal) {
      x = expressionFactory.Create<BinaryExpression>(
          Pos(start), ExpressionType::Equal, x, y);
    } else {
      x = expressionFactory.Create<BinaryExpression>(
          Pos(start), ExpressionType::NotEqual, x, y);
    }
  }
  return x;
}

ExpPtr Parser::ParseRelation() {
  const Token &start = Look();
  auto x = ParseExpr();
  if (Look().tag == TokenTag::GreaterThan || Look().tag == TokenTag::LessThan ||
      Look().tag == TokenTag::GreaterThanOrEqual ||
      Look().tag == TokenTag::LessThanOrEqual) {
    Token t = Look();
    Advance();
    auto y = ParseExpr();
    if (t.tag == TokenTag::GreaterThan) {
      return expressionFactory.Create<BinaryExpression>(
          Pos(start), ExpressionType::GreaterThan, x, y);
    } else if (t.tag == TokenTag::LessThan) {
      return expressionFactory.Create<BinaryExpression>(
          Pos(start), ExpressionType::LessThan, x, y);
    } else if (t.tag == TokenTag::GreaterThanOrEqual) {
      return expressionFactory.Create<BinaryExpression>(
          Pos(start), ExpressionType::GreaterThanOrEqual, x, y);
    } else {
      return expressionFactory.Create<BinaryExpression>(
          Pos(start), ExpressionType::LessThanOrEqual, x, y);
    }
  } else {
    return x;
  }
}

ExpPtr Parser::ParseExpr() {
  const Token &start = Look();
  auto x = ParseTerm();
  while (Look().tag == TokenTag::Add || Look().tag == TokenTag::Subtract) {
    Token t = Look();
    Advance();
    auto y = ParseTerm();
    if (t.tag == TokenTag::Add) {
      x = expressionFactory.Create<BinaryExpression>(Pos(start),
                                                     ExpressionType::Add, x, y);
    } else {
      x = expressionFactory.Create<BinaryExpression>(
          Pos(start), ExpressionType::Subtract, x, y);
    }
  }
  return x;
}

ExpPtr Parser::ParseTerm() {
  const Token &start = Look();
  auto x = ParseUnary();
  while (Look().tag == TokenTag::Multiply || Look().tag == TokenTag::Divide) {
    Token t = Look();
    Advance();
    auto y = ParseUnary();
    if (t.tag == TokenTag::Multiply) {
      x = expressionFactory.Create<BinaryExpression>(
          Pos(start), ExpressionType::Multiply, x, y);
    } else {
      x = expressionFactory.Create<BinaryExpression>(
          Pos(start), ExpressionType::Divide, x, y);
    }
  }
  return x;
}

ExpPtr Parser::ParseUnary() {
  const Token &start = Look();
  if (Look().tag == TokenTag::Add) {
    Advance();
    auto x = ParseUnary();
    return expressionFactory.Create<UnaryExpression>(
        Pos(start), ExpressionType::UnaryPlus, x,
        TypeFactory::CreateBasicType(TypeCode::Unknown));
  } else if (Look().tag == TokenTag::Subtract) {
    Advance();
    auto x = ParseUnary();
    return expressionFactory.Create<UnaryExpression>(
        Pos(start), ExpressionType::UnaryMinus, x,
        TypeFactory::CreateBasicType(TypeCode::Unknown));
  } else if (Look().tag == TokenTag::Not) {
    Advance();
    auto x = ParseUnary();
    return expressionFactory.Create<UnaryExpression>(
        Pos(start), ExpressionType::Not, x,
        TypeFactory::CreateBasicType(TypeCode::Unknown));
  } else {
    return ParsePostfix();
  }
}

ExpPtr Parser::ParsePostfix() {
  auto x = ParseFactor();
  while (Look().tag == TokenTag::LeftParenthesis ||
         Look().tag == TokenTag::LeftBracket || Look().tag == TokenTag::Dot) {
    const Token &start = Look();

    if (Look().tag == TokenTag::LeftParenthesis) {
      auto arguments = ParseArguments();
      x = expressionFactory.Create<CallExpression>(Pos(start), x, arguments);
    } else {
      /* TODO: '[' and '.' */
    }
  }
  return x;
}

ExpPtr Parser::ParseFactor() {
  if (Look().tag == TokenTag::LeftParenthesis) {
    Advance();
    ExpPtr x = ParseOr();
    Match(TokenTag::RightParenthesis);
    return x;
  } else if (Look().tag == TokenTag::Integer) {
    std::u32string v = Look().text;
    const Token &start = Look();
    Advance();
    return expressionFactory.Create<ConstantExpression>(Pos(start), v,
                                                        TypeCode::Int32);
  } else if (Look().tag == TokenTag::Float) {
    std::u32string v = Look().text;
    const Token &start = Look();
    Advance();
    return expressionFactory.Create<ConstantExpression>(Pos(start), v,
                                                        TypeCode::Float64);
  } else if (Look().tag == TokenTag::Character) {
    std::u32string v = Look().text;
    const Token &start = Look();
    Advance();
    return expressionFactory.Create<ConstantExpression>(Pos(start), v,
                                                        TypeCode::Char);
  } else if (Look().tag == TokenTag::String) {
    std::u32string v = Look().text;
    const Token &start = Look();
    Advance();
    return expressionFactory.Create<ConstantExpression>(Pos(start), v,
                                                        TypeCode::String);
  } else if (Look().tag == TokenTag::True) {
    const Token &start = Look();
    Advance();
    return expressionFactory.Create<ConstantExpression>(Pos(start), U"true",
                                                        TypeCode::Boolean);
  } else if (Look().tag == TokenTag::False) {
    const Token &start = Look();
    Advance();
    return expressionFactory.Create<ConstantExpression>(Pos(start), U"false",
                                                        TypeCode::Boolean);
  } else if (Look().tag == TokenTag::Identifier) {
    std::u32string name = Look().text;
    const Token &start = Look();
    Advance();
    return expressionFactory.Create<ParameterExpression>(
        Pos(start), name, TypeFactory::CreateBasicType(TypeCode::Unknown));
  } else {
    auto sv = magic_enum::enum_name(Look().tag);
    std::string lookTagStr(sv.begin(), sv.end());
    throw ParserException(
        __FILE__, __LINE__,
        SourceRange(document, Look().line, Look().column, Look().line,
                    Look().column + Look().text.size()),
        Utility::UTF32ToUTF8(
            Format(U"Unexpected token type: '{}'.", lookTagStr)),
        nullptr);
  }
}

ExpPtr Parser::ParseBlock() {
  const Token &start = Look();
  Match(TokenTag::LeftBrace);
  vector<ExpPtr> expressions;
  while (!IsEof() && Look().tag != TokenTag::RightBrace) {
    expressions.push_back(Statement());
  }
  Match(TokenTag::RightBrace);
  return expressionFactory.Create<BlockExpression>(Pos(start), expressions);
}

ExpPtr Parser::IfStatement() {
  const Token &start = Look();
  Match(TokenTag::If);
  ExpPtr condition = ParseOr();
  ExpPtr ifTrue = ParseBlock();
  if (Look().tag == TokenTag::Else) {
    Match(TokenTag::Else);
    if (Look().tag == TokenTag::If) {
      auto chunk = IfStatement();
      return expressionFactory.Create<ConditionalExpression>(
          Pos(start), condition, ifTrue, chunk);
    } else {
      auto chunk = ParseBlock();
      return expressionFactory.Create<ConditionalExpression>(
          Pos(start), condition, ifTrue, chunk);
    }
  } else {
    auto empty = expressionFactory.Create<DefaultExpression>(
        Pos(Look()), TypeFactory::CreateBasicType(TypeCode::Unknown));
    return expressionFactory.Create<ConditionalExpression>(
        Pos(start), condition, ifTrue, empty);
  }
}

ExpPtr Parser::WhileStatement() {
  const Token &start = Look();
  Match(TokenTag::While);
  Match(TokenTag::LeftParenthesis);
  auto empty = expressionFactory.Create<DefaultExpression>(
      Pos(start), TypeFactory::CreateBasicType(TypeCode::Unknown));
  auto condition = ParseOr();
  Match(TokenTag::RightParenthesis);
  auto body = ParseBlock();
  return expressionFactory.Create<LoopExpression>(Pos(start), empty, condition,
                                                  body);
}

ExpPtr Parser::VariableDeclarationStatement() {
  const Token &start = Look();
  Match(TokenTag::Var);
  std::u32string name = Match(TokenTag::Identifier).text;
  Match(TokenTag::Assign);
  auto initializer = ParseOr();

  return expressionFactory.Create<VariableDeclarationExpression>(
      Pos(start), name, initializer);
}

std::vector<ExpPtr> Parser::ParseArguments() {
  vector<ExpPtr> arguments;
  Match(TokenTag::LeftParenthesis);
  if (Look().tag == TokenTag::RightParenthesis) {
    Match(TokenTag::RightParenthesis);
  } else {
    arguments.push_back(ParseArgument());
    while (!IsEof() && Look().tag != TokenTag::RightParenthesis) {
      Match(TokenTag::Comma);
      arguments.push_back(ParseArgument());
    }
    Match(TokenTag::RightParenthesis);
  }
  return arguments;
}

ExpPtr Parser::ParseArgument() {
  ExpPtr value = ParseOr();
  return value;
}

}; /* namespace SyntaxAnalysis */
}; /* namespace Cygni */