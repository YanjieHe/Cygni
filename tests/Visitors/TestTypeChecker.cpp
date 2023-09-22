#include <catch2/catch.hpp>

#include "LexicalAnalysis/Lexer.hpp"
#include "SyntaxAnalysis/Parser.hpp"
#include "Visitors/TypeChecker.hpp"

using namespace Cygni::LexicalAnalysis;
using namespace Cygni::SyntaxAnalysis;
using namespace Cygni::Expressions;
using namespace Cygni::Visitors;

TEST_CASE("test (36 / 9)", "[Arithmetic]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"36 / 9");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {TokenTag::Integer, TokenTag::Divide,
                                        TokenTag::Integer, TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  Parser parser(tokens, sourceCodeFile);
  auto exp = parser.ParseExpr();
  TypeChecker typeChecker;

  Scope<const Type *> scope;
  const Type *type = typeChecker.Visit(exp, &scope);
  REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test variable declaration", "[Variable]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"{ var a = 'a'; a; }");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::LeftBrace,  TokenTag::Var,       TokenTag::Identifier,
      TokenTag::Assign,     TokenTag::Character, TokenTag::Semicolon,
      TokenTag::Identifier, TokenTag::Semicolon, TokenTag::RightBrace,
      TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  Parser parser(tokens, sourceCodeFile);
  auto exp = parser.ParseBlock();

  TypeChecker typeChecker;

  Scope<const Type *> scope;
  const Type *type = typeChecker.Visit(exp, &scope);
  REQUIRE(type->GetTypeCode() == TypeCode::Char);
}

TEST_CASE("test shadowing outer variable", "[Variable]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile,
              U"{ var x = 10; { var x = \"variable x\"; x; }; }");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::LeftBrace,  TokenTag::Var,        TokenTag::Identifier,
      TokenTag::Assign,     TokenTag::Integer,    TokenTag::Semicolon,
      TokenTag::LeftBrace,  TokenTag::Var,        TokenTag::Identifier,
      TokenTag::Assign,     TokenTag::String,     TokenTag::Semicolon,
      TokenTag::Identifier, TokenTag::Semicolon,  TokenTag::RightBrace,
      TokenTag::Semicolon,  TokenTag::RightBrace, TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  Parser parser(tokens, sourceCodeFile);
  auto exp = parser.ParseBlock();

  TypeChecker typeChecker;

  Scope<const Type *> scope;
  const Type *type = typeChecker.Visit(exp, &scope);
  REQUIRE(type->GetTypeCode() == TypeCode::String);
}

TEST_CASE("test conditional", "[Conditional]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"if (1 < 2) { 10.3; } else { false; }");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::If,         TokenTag::LeftParenthesis,
      TokenTag::Integer,    TokenTag::LessThan,
      TokenTag::Integer,    TokenTag::RightParenthesis,
      TokenTag::LeftBrace,  TokenTag::Float,
      TokenTag::Semicolon,  TokenTag::RightBrace,
      TokenTag::Else,       TokenTag::LeftBrace,
      TokenTag::False,      TokenTag::Semicolon,
      TokenTag::RightBrace, TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  Parser parser(tokens, sourceCodeFile);
  auto exp = parser.Statement();

  TypeChecker typeChecker;

  Scope<const Type *> scope;
  const Type *type = typeChecker.Visit(exp, &scope);
  REQUIRE(type->GetTypeCode() == TypeCode::Union);
}

TEST_CASE("test loop", "[Loop]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"{ var i = 0; while (i < 10) { i = i + 1; } }");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::LeftBrace,  TokenTag::Var,
      TokenTag::Identifier, TokenTag::Assign,
      TokenTag::Integer,    TokenTag::Semicolon,
      TokenTag::While,      TokenTag::LeftParenthesis,
      TokenTag::Identifier, TokenTag::LessThan,
      TokenTag::Integer,    TokenTag::RightParenthesis,
      TokenTag::LeftBrace,  TokenTag::Identifier,
      TokenTag::Assign,     TokenTag::Identifier,
      TokenTag::Add,        TokenTag::Integer,
      TokenTag::Semicolon,  TokenTag::RightBrace,
      TokenTag::RightBrace, TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  Parser parser(tokens, sourceCodeFile);
  auto exp = parser.ParseBlock();

  TypeChecker typeChecker;

  Scope<const Type *> scope;
  const Type *type = typeChecker.Visit(exp, &scope);
  REQUIRE(type->GetTypeCode() == TypeCode::Empty);
}

TEST_CASE("test function declaration", "[Function]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile,
              U"func f(x: Double, y: Double): Double { x + y; }");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {TokenTag::Func,
                                        TokenTag::Identifier,
                                        TokenTag::LeftParenthesis,
                                        TokenTag::Identifier,
                                        TokenTag::Colon,
                                        TokenTag::Identifier,
                                        TokenTag::Comma,
                                        TokenTag::Identifier,
                                        TokenTag::Colon,
                                        TokenTag::Identifier,
                                        TokenTag::RightParenthesis,
                                        TokenTag::Colon,
                                        TokenTag::Identifier,
                                        TokenTag::LeftBrace,
                                        TokenTag::Identifier,
                                        TokenTag::Add,
                                        TokenTag::Identifier,
                                        TokenTag::Semicolon,
                                        TokenTag::RightBrace,
                                        TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  Parser parser(tokens, sourceCodeFile);
  auto exp = parser.FunctionDeclarationStatement();

  TypeChecker typeChecker;

  Scope<const Type *> scope;
  const Type *type = typeChecker.Visit(exp, &scope);
  REQUIRE(type->GetTypeCode() == TypeCode::Callable);
  const CallableType *callableType = static_cast<const CallableType *>(type);
  REQUIRE(callableType->Arguments().size() == 2);
  REQUIRE(callableType->Arguments().at(0)->GetTypeCode() == TypeCode::Float64);
  REQUIRE(callableType->Arguments().at(1)->GetTypeCode() == TypeCode::Float64);
  REQUIRE(callableType->GetReturnType()->GetTypeCode() == TypeCode::Float64);
}