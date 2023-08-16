#include <catch2/catch.hpp>

#include "LexicalAnalysis/Lexer.hpp"

using namespace Cygni::LexicalAnalysis;

TEST_CASE("test (32 * 64)", "[Arithmetic]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"32 * 64");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {TokenTag::Integer, TokenTag::Multiply,
                                        TokenTag::Integer, TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }
}

TEST_CASE("a = 4.5 * 2.3;", "[Arithmetic]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"a = 4.5 * 2.3;");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::Identifier, TokenTag::Assign, TokenTag::Float,
      TokenTag::Multiply,   TokenTag::Float,  TokenTag::Semicolon,
      TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }
}

TEST_CASE("Nested Operations with Mixed Types", "[Arithmetic]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile,
              U"((5 + 3.5) * (8.0 / 2) - 3 % 2) + 4.5E2 - 0.005E-3");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {TokenTag::LeftParenthesis,
                                        TokenTag::LeftParenthesis,
                                        TokenTag::Integer,
                                        TokenTag::Add,
                                        TokenTag::Float,
                                        TokenTag::RightParenthesis,
                                        TokenTag::Multiply,
                                        TokenTag::LeftParenthesis,
                                        TokenTag::Float,
                                        TokenTag::Divide,
                                        TokenTag::Integer,
                                        TokenTag::RightParenthesis,
                                        TokenTag::Subtract,
                                        TokenTag::Integer,
                                        TokenTag::Modulo,
                                        TokenTag::Integer,
                                        TokenTag::RightParenthesis,
                                        TokenTag::Add,
                                        TokenTag::Float,
                                        TokenTag::Subtract,
                                        TokenTag::Float,
                                        TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }
}

TEST_CASE("Multi-line Expressions with Comments", "[Arithmetic]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(
      sourceCodeFile,
      U"var result = x + y  // Adding x and y\n                 - (z * w) // "
      U"Multiplying z and w\n                 / v; // Final division");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::Var,        TokenTag::Identifier,
      TokenTag::Assign,     TokenTag::Identifier,
      TokenTag::Add,        TokenTag::Identifier,
      TokenTag::Subtract,   TokenTag::LeftParenthesis,
      TokenTag::Identifier, TokenTag::Multiply,
      TokenTag::Identifier, TokenTag::RightParenthesis,
      TokenTag::Divide,     TokenTag::Identifier,
      TokenTag::Semicolon,  TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }
}

TEST_CASE("var f1 = 3.14;", "[VariableDeclaration]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"var f1 = 3.14;");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::Var,   TokenTag::Identifier, TokenTag::Assign,
      TokenTag::Float, TokenTag::Semicolon,  TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }
}

TEST_CASE("var f2 = 2.71E-4;", "[VariableDeclaration]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"var f2 = 2.71E-4;");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::Var,   TokenTag::Identifier, TokenTag::Assign,
      TokenTag::Float, TokenTag::Semicolon,  TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }
}

TEST_CASE("var result = (5 + x) * 7 / (y - 3);", "[VariableDeclaration]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"var result = (5 + x) * 7 / (y - 3);");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::Var,        TokenTag::Identifier,
      TokenTag::Assign,     TokenTag::LeftParenthesis,
      TokenTag::Integer,    TokenTag::Add,
      TokenTag::Identifier, TokenTag::RightParenthesis,
      TokenTag::Multiply,   TokenTag::Integer,
      TokenTag::Divide,     TokenTag::LeftParenthesis,
      TokenTag::Identifier, TokenTag::Subtract,
      TokenTag::Integer,    TokenTag::RightParenthesis,
      TokenTag::Semicolon,  TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }
}

TEST_CASE("var greet = \"Hello, @User!\";", "[VariableDeclaration]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"var greet = \"Hello, @User!\";");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::Var,    TokenTag::Identifier, TokenTag::Assign,
      TokenTag::String, TokenTag::Semicolon,  TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }
}

TEST_CASE("var escape1 = '\\n';", "[SpeicalCharacter]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"var escape1 = '\\n';");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::Var,       TokenTag::Identifier, TokenTag::Assign,
      TokenTag::Character, TokenTag::Semicolon,  TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }
}

TEST_CASE("var escape2 = '\\t';", "[SpeicalCharacter]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"var escape2 = '\\t';");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::Var,       TokenTag::Identifier, TokenTag::Assign,
      TokenTag::Character, TokenTag::Semicolon,  TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }
}

TEST_CASE("var unicodeEscape = '\\u006A'; // Represents the letter 'j'",
          "[CharacterLiterals]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile,
              U"var unicodeEscape = '\\u006A'; // Represents the letter 'j'");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::Var,       TokenTag::Identifier, TokenTag::Assign,
      TokenTag::Character, TokenTag::Semicolon,  TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  REQUIRE(tokens.at(3).text == U"j");
}

TEST_CASE("var hexEscape = '\\x4A'; // Represents the letter 'J'",
          "[CharacterLiterals]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile,
              U"var hexEscape = '\\x4A'; // Represents the letter 'J'");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {
      TokenTag::Var,       TokenTag::Identifier, TokenTag::Assign,
      TokenTag::Character, TokenTag::Semicolon,  TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }

  REQUIRE(tokens.at(3).text == U"J");
}

TEST_CASE("Long String with Multiple Escape Sequences", "[String]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile,
              U"\"This is a \\\"complicated\\\" string with newlines\\n\\t and "
              U"tabs and a backslash \\\\.\"");

  std::vector<Token> tokens = lexer.ReadAll();

  std::vector<TokenTag> expectedTags = {TokenTag::String, TokenTag::Eof};

  REQUIRE(tokens.size() == expectedTags.size());
  for (size_t i = 0; i < tokens.size(); i++) {
    REQUIRE(tokens.at(i).tag == expectedTags.at(i));
  }
}

TEST_CASE("&unsupportedSymbol;", "[UnexpectedTokens]") {
  std::shared_ptr<SourceCodeFile> sourceCodeFile =
      std::make_shared<SourceCodeFile>("source-code-file");

  Lexer lexer(sourceCodeFile, U"&unsupportedSymbol;");

  REQUIRE_THROWS_AS(lexer.ReadAll(), LexicalException);
}