#include <catch2/catch.hpp>

#include "LexicalAnalysis/Lexer.hpp"
#include "SyntaxAnalysis/Parser.hpp"

using namespace Cygni::LexicalAnalysis;
using namespace Cygni::SyntaxAnalysis;
using namespace Cygni::Expressions;

TEST_CASE("test (15 * 72)", "[Arithmetic]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"15 * 72");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {TokenTag::Integer, TokenTag::Multiply,
                                        TokenTag::Integer, TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  Parser parser(tokens, sourceCodeFile);
  auto exp = parser.ParseExpr();
  REQUIRE(exp->NodeType() == ExpressionType::Multiply);
  auto binaryExp = static_cast<BinaryExpression *>(exp);
  REQUIRE(binaryExp->Left() != nullptr);
  REQUIRE(binaryExp->Right() != nullptr);
  REQUIRE(binaryExp->Left()->NodeType() == ExpressionType::Constant);
  REQUIRE(binaryExp->Right()->NodeType() == ExpressionType::Constant);
}

TEST_CASE("test (135 + 27)", "[Arithmetic]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"135 + 27");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {TokenTag::Integer, TokenTag::Add,
                                        TokenTag::Integer, TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  Parser parser(tokens, sourceCodeFile);
  auto exp = parser.ParseExpr();

  REQUIRE(exp->NodeType() == ExpressionType::Add);
  auto binaryExp = static_cast<BinaryExpression *>(exp);
  REQUIRE(binaryExp->Left() != nullptr);
  REQUIRE(binaryExp->Right() != nullptr);
  REQUIRE(binaryExp->Left()->NodeType() == ExpressionType::Constant);
  REQUIRE(binaryExp->Right()->NodeType() == ExpressionType::Constant);
}

TEST_CASE("test (27 / 9)", "[Arithmetic]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"27 / 9");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {TokenTag::Integer, TokenTag::Divide,
                                        TokenTag::Integer, TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  Parser parser(tokens, sourceCodeFile);
  auto exp = parser.ParseExpr();
  REQUIRE(exp->NodeType() == ExpressionType::Divide);
  auto binaryExp = static_cast<BinaryExpression *>(exp);
  REQUIRE(binaryExp->Left() != nullptr);
  REQUIRE(binaryExp->Right() != nullptr);
  REQUIRE(binaryExp->Left()->NodeType() == ExpressionType::Constant);
  REQUIRE(binaryExp->Right()->NodeType() == ExpressionType::Constant);
}

TEST_CASE("test complex expression", "[Complex]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"(5 + 8) * 2");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {TokenTag::LeftParenthesis,
                                        TokenTag::Integer,
                                        TokenTag::Add,
                                        TokenTag::Integer,
                                        TokenTag::RightParenthesis,
                                        TokenTag::Multiply,
                                        TokenTag::Integer,
                                        TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  Parser parser(tokens, sourceCodeFile);
  auto exp = parser.ParseExpr();
  REQUIRE(exp->NodeType() == ExpressionType::Multiply);
  auto multiplyExp = static_cast<BinaryExpression *>(exp);
  REQUIRE(multiplyExp->Left() != nullptr);
  REQUIRE(multiplyExp->Right() != nullptr);
  REQUIRE(multiplyExp->Left()->NodeType() == ExpressionType::Add);
  REQUIRE(multiplyExp->Right()->NodeType() == ExpressionType::Constant);
}

TEST_CASE("test (true and false)", "[Logical]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"true and false");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {TokenTag::True, TokenTag::And,
                                        TokenTag::False, TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  Parser parser(tokens, sourceCodeFile);
  auto exp = parser.ParseOr();

  REQUIRE(exp->NodeType() == ExpressionType::And);
  auto binaryExp = static_cast<BinaryExpression *>(exp);
  REQUIRE(binaryExp->Left() != nullptr);
  REQUIRE(binaryExp->Right() != nullptr);
  REQUIRE(binaryExp->Left()->NodeType() == ExpressionType::Constant);
  REQUIRE(binaryExp->Right()->NodeType() == ExpressionType::Constant);
}

TEST_CASE("test (x = 42)", "[Assignment]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"x = 42;");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {TokenTag::Identifier, TokenTag::Assign,
                                        TokenTag::Integer, TokenTag::Semicolon,
                                        TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  Parser parser(tokens, sourceCodeFile);
  auto exp = parser.Statement();
  REQUIRE(exp->NodeType() == ExpressionType::Assign);
  auto assignExp = static_cast<BinaryExpression *>(exp);
  REQUIRE(assignExp->Left() != nullptr);
  REQUIRE(assignExp->Right() != nullptr);
  REQUIRE(assignExp->Left()->NodeType() == ExpressionType::Parameter);
  REQUIRE(assignExp->Right()->NodeType() == ExpressionType::Constant);
}

TEST_CASE("test if-else statement", "[ControlFlow]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"if (true) { 10; } else { 20; }");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::If,         TokenTag::LeftParenthesis,
      TokenTag::True,       TokenTag::RightParenthesis,
      TokenTag::LeftBrace,  TokenTag::Integer,
      TokenTag::Semicolon,  TokenTag::RightBrace,
      TokenTag::Else,       TokenTag::LeftBrace,
      TokenTag::Integer,    TokenTag::Semicolon,
      TokenTag::RightBrace, TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  Parser parser(tokens, sourceCodeFile);
  auto exp = parser.Statement();

  REQUIRE(exp->NodeType() == ExpressionType::Conditional);
  auto conditionalExp = static_cast<ConditionalExpression *>(exp);
  REQUIRE(conditionalExp->Test() != nullptr);
  REQUIRE(conditionalExp->IfTrue() != nullptr);
  REQUIRE(conditionalExp->IfFalse() != nullptr);
}

TEST_CASE("test while statement", "[ControlFlow]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"while (true) { 35; }");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::While,     TokenTag::LeftParenthesis,
      TokenTag::True,      TokenTag::RightParenthesis,
      TokenTag::LeftBrace, TokenTag::Integer,
      TokenTag::Semicolon, TokenTag::RightBrace,
      TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  Parser parser(tokens, sourceCodeFile);
  auto exp = parser.Statement();

  REQUIRE(exp->NodeType() == ExpressionType::WhileLoop);
  auto loopExp = static_cast<WhileLoopExpression *>(exp);
  REQUIRE(loopExp->Condition() != nullptr);
  REQUIRE(loopExp->Body() != nullptr);

  REQUIRE(loopExp->Condition()->NodeType() == ExpressionType::Constant);
  REQUIRE(loopExp->Body()->NodeType() == ExpressionType::Block);
}

TEST_CASE("test function call", "[Function]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"f(12, 13);");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::Identifier, TokenTag::LeftParenthesis,
      TokenTag::Integer,    TokenTag::Comma,
      TokenTag::Integer,    TokenTag::RightParenthesis,
      TokenTag::Semicolon,  TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  Parser parser(tokens, sourceCodeFile);
  auto exp = parser.Statement();

  REQUIRE(exp->NodeType() == ExpressionType::Call);
  auto callExp = static_cast<CallExpression *>(exp);
  REQUIRE(callExp->Function() != nullptr);
  REQUIRE(callExp->Arguments().size() == 2);

  REQUIRE(callExp->Function()->NodeType() == ExpressionType::Parameter);
  for (size_t i = 0; i < 2; i++) {
    REQUIRE(callExp->Arguments().at(i)->NodeType() == ExpressionType::Constant);
  }
}