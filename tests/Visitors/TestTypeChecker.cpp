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

  Scope<const Type *> *scope;
  const Type *type = typeChecker.Visit(exp, scope);
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

  Scope<const Type *> *scope;
  const Type *type = typeChecker.Visit(exp, scope);
  REQUIRE(type->GetTypeCode() == TypeCode::Char);
}