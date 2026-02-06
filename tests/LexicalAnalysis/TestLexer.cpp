#include <catch2/catch.hpp>

#include "LexicalAnalysis/Lexer.hpp"

using namespace Cygni::LexicalAnalysis;

static std::vector<Token> Tokenize(const std::string &sourceCodeFileName, const std::u32string &sourceCode)
{
    auto file = std::make_shared<SourceCodeFile>(sourceCodeFileName);
    Lexer lexer(file, sourceCode);
    return lexer.ReadAll();
}

static void RequireTags(const std::vector<Token> &tokens, const std::vector<TokenTag> &expectedTags)
{
    REQUIRE(tokens.size() == expectedTags.size());
    for (size_t i = 0; i < tokens.size(); i++)
    {
        REQUIRE(tokens.at(i).tag == expectedTags.at(i));
    }
}

TEST_CASE("test (32 * 64)", "[Arithmetic]")
{
    std::vector<Token> tokens = Tokenize("source-code-file", U"32 * 64");

    std::vector<TokenTag> expectedTags = {TokenTag::Integer, TokenTag::Multiply, TokenTag::Integer, TokenTag::Eof};

    RequireTags(tokens, expectedTags);
}

TEST_CASE("a = 4.5 * 2.3;", "[Arithmetic]")
{
    std::vector<Token> tokens = Tokenize("source-code-file", U"a = 4.5 * 2.3;");

    std::vector<TokenTag> expectedTags = {TokenTag::Identifier, TokenTag::Assign, TokenTag::Float,
                                          TokenTag::Multiply,   TokenTag::Float,  TokenTag::Semicolon,
                                          TokenTag::Eof};

    RequireTags(tokens, expectedTags);
}

TEST_CASE("Invalid Exponent", "[Arithmetic]")
{
    REQUIRE_THROWS_AS(Tokenize("source-code-file", U"1.23E"), LexicalException);
    REQUIRE_THROWS_AS(Tokenize("source-code-file", U"1.23E+"), LexicalException);
}

TEST_CASE("Nested Operations with Mixed Types", "[Arithmetic]")
{
    std::vector<Token> tokens = Tokenize("source-code-file", U"((5 + 3.5) * (8.0 / 2) - 3 % 2) + 4.5E2 - 0.005E-3");

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

    RequireTags(tokens, expectedTags);
}

TEST_CASE("Multi-line Expressions with Comments", "[Arithmetic]")
{
    std::vector<Token> tokens =
        Tokenize("source-code-file", U"var result = x + y  // Adding x and y\n                 - (z * w) // "
                                     U"Multiplying z and w\n                 / v; // Final division");

    std::vector<TokenTag> expectedTags = {
        TokenTag::Var,        TokenTag::Identifier, TokenTag::Assign,     TokenTag::Identifier,
        TokenTag::Add,        TokenTag::Identifier, TokenTag::Subtract,   TokenTag::LeftParenthesis,
        TokenTag::Identifier, TokenTag::Multiply,   TokenTag::Identifier, TokenTag::RightParenthesis,
        TokenTag::Divide,     TokenTag::Identifier, TokenTag::Semicolon,  TokenTag::Eof};

    RequireTags(tokens, expectedTags);
}

TEST_CASE("var f1 = 3.14;", "[VariableDeclaration]")
{
    std::vector<Token> tokens = Tokenize("source-code-file", U"var f1 = 3.14;");

    std::vector<TokenTag> expectedTags = {TokenTag::Var,   TokenTag::Identifier, TokenTag::Assign,
                                          TokenTag::Float, TokenTag::Semicolon,  TokenTag::Eof};

    RequireTags(tokens, expectedTags);
}

TEST_CASE("var f2 = 2.71E-4;", "[VariableDeclaration]")
{
    std::vector<Token> tokens = Tokenize("source-code-file", U"var f2 = 2.71E-4;");

    std::vector<TokenTag> expectedTags = {TokenTag::Var,   TokenTag::Identifier, TokenTag::Assign,
                                          TokenTag::Float, TokenTag::Semicolon,  TokenTag::Eof};

    RequireTags(tokens, expectedTags);
}

TEST_CASE("var result = (5 + x) * 7 / (y - 3);", "[VariableDeclaration]")
{
    std::vector<Token> tokens = Tokenize("source-code-file", U"var result = (5 + x) * 7 / (y - 3);");

    std::vector<TokenTag> expectedTags = {
        TokenTag::Var,        TokenTag::Identifier, TokenTag::Assign,     TokenTag::LeftParenthesis,
        TokenTag::Integer,    TokenTag::Add,        TokenTag::Identifier, TokenTag::RightParenthesis,
        TokenTag::Multiply,   TokenTag::Integer,    TokenTag::Divide,     TokenTag::LeftParenthesis,
        TokenTag::Identifier, TokenTag::Subtract,   TokenTag::Integer,    TokenTag::RightParenthesis,
        TokenTag::Semicolon,  TokenTag::Eof};

    RequireTags(tokens, expectedTags);
}

TEST_CASE("var greet = \"Hello, @User!\";", "[VariableDeclaration]")
{
    std::vector<Token> tokens = Tokenize("source-code-file", U"var greet = \"Hello, @User!\";");

    std::vector<TokenTag> expectedTags = {TokenTag::Var,    TokenTag::Identifier, TokenTag::Assign,
                                          TokenTag::String, TokenTag::Semicolon,  TokenTag::Eof};

    RequireTags(tokens, expectedTags);
}

TEST_CASE("var escape1 = '\\n';", "[SpeicalCharacter]")
{
    std::vector<Token> tokens = Tokenize("source-code-file", U"var escape1 = '\\n';");

    std::vector<TokenTag> expectedTags = {TokenTag::Var,       TokenTag::Identifier, TokenTag::Assign,
                                          TokenTag::Character, TokenTag::Semicolon,  TokenTag::Eof};

    RequireTags(tokens, expectedTags);
}

TEST_CASE("var escape2 = '\\t';", "[SpeicalCharacter]")
{
    std::vector<Token> tokens = Tokenize("source-code-file", U"var escape2 = '\\t';");

    std::vector<TokenTag> expectedTags = {TokenTag::Var,       TokenTag::Identifier, TokenTag::Assign,
                                          TokenTag::Character, TokenTag::Semicolon,  TokenTag::Eof};

    RequireTags(tokens, expectedTags);
}

TEST_CASE("var unicodeEscape = '\\u006A'; // Represents the letter 'j'", "[CharacterLiterals]")
{
    std::vector<Token> tokens =
        Tokenize("source-code-file", U"var unicodeEscape = '\\u006A'; // Represents the letter 'j'");

    std::vector<TokenTag> expectedTags = {TokenTag::Var,       TokenTag::Identifier, TokenTag::Assign,
                                          TokenTag::Character, TokenTag::Semicolon,  TokenTag::Eof};

    RequireTags(tokens, expectedTags);

    REQUIRE(tokens.at(3).text == U"j");
}

TEST_CASE("var hexEscape = '\\x4A'; // Represents the letter 'J'", "[CharacterLiterals]")
{
    std::vector<Token> tokens = Tokenize("source-code-file", U"var hexEscape = '\\x4A'; // Represents the letter 'J'");

    std::vector<TokenTag> expectedTags = {TokenTag::Var,       TokenTag::Identifier, TokenTag::Assign,
                                          TokenTag::Character, TokenTag::Semicolon,  TokenTag::Eof};

    RequireTags(tokens, expectedTags);

    REQUIRE(tokens.at(3).text == U"J");
}

TEST_CASE("Invalid Character Literal", "[CharacterLiterals]")
{
    REQUIRE_THROWS_AS(Tokenize("source-code-file", U"'"), LexicalException);
    REQUIRE_THROWS_AS(Tokenize("source-code-file", U"'ab'"), LexicalException);
    REQUIRE_THROWS_AS(Tokenize("source-code-file", U"'\\q'"), LexicalException);
}

TEST_CASE("Long String with Multiple Escape Sequences", "[String]")
{
    std::vector<Token> tokens =
        Tokenize("source-code-file", U"\"This is a \\\"complicated\\\" string with newlines\\n\\t and "
                                     U"tabs and a backslash \\\\.\"");

    std::vector<TokenTag> expectedTags = {TokenTag::String, TokenTag::Eof};

    RequireTags(tokens, expectedTags);

    REQUIRE(tokens.at(0).text == U"This is a \"complicated\" string with newlines\n\t and tabs and a backslash \\.");
}

TEST_CASE("Invalid String Escape", "[String]")
{
    REQUIRE_THROWS_AS(Tokenize("source-code-file", U"\"\\q\""), LexicalException);
}

TEST_CASE("Unterminated String", "[String]")
{
    REQUIRE_THROWS_AS(Tokenize("source-code-file", U"\"unterminated"), LexicalException);
}

TEST_CASE("&unsupportedSymbol;", "[UnexpectedTokens]")
{
    REQUIRE_THROWS_AS(Tokenize("source-code-file", U"&unsupportedSymbol;"), LexicalException);
}