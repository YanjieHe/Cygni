#include <catch2/catch.hpp>

#include "LexicalAnalysis/Lexer.hpp"
#include "SyntaxAnalysis/Parser.hpp"
#include "Visitors/TypeChecker.hpp"

using namespace Cygni::LexicalAnalysis;
using namespace Cygni::SyntaxAnalysis;
using namespace Cygni::Expressions;
using namespace Cygni::Visitors;

static Parser CreateParser(const std::u32string &sourceCode)
{
    std::shared_ptr<SourceCodeFile> sourceCodeFile = std::make_shared<SourceCodeFile>("source-code-file");
    Lexer lexer(sourceCodeFile, sourceCode);
    std::vector<Token> tokens = lexer.ReadAll();

    return Parser(tokens, sourceCodeFile);
}

TEST_CASE("test (36 / 9)", "[Arithmetic]")
{
    Parser parser = CreateParser(U"36 / 9");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test variable declaration", "[Variable]")
{
    Parser parser = CreateParser(U"{ var a = 'a'; a; }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Char);
}

TEST_CASE("test shadowing outer variable", "[Variable]")
{
    Parser parser = CreateParser(U"{ var x = 10; { var x = \"variable x\"; x; }; }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::String);
}

TEST_CASE("test conditional", "[Conditional]")
{
    Parser parser = CreateParser(U"if (1 < 2) { 10.3; } else { false; }");
    auto exp = parser.Statement();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Union);
}

TEST_CASE("test loop", "[WhileLoop]")
{
    Parser parser = CreateParser(U"{ var i = 0; while (i < 10) { i = i + 1; } }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Empty);
}

TEST_CASE("test function declaration", "[Function]")
{
    Parser parser = CreateParser(U"func f(x: Double, y: Double): Double { x + y; }");
    auto exp = parser.FunctionDeclarationStatement({});
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Callable);
    const CallableType *callableType = static_cast<const CallableType *>(type);
    REQUIRE(callableType->Arguments().size() == 2);
    REQUIRE(callableType->Arguments().at(0)->GetTypeCode() == TypeCode::Float64);
    REQUIRE(callableType->Arguments().at(1)->GetTypeCode() == TypeCode::Float64);
    REQUIRE(callableType->GetReturnType()->GetTypeCode() == TypeCode::Float64);
}

// ============================================================================
// Arithmetic Operations Tests
// ============================================================================

TEST_CASE("test addition of integers", "[Arithmetic]")
{
    Parser parser = CreateParser(U"15 + 27");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test subtraction of integers", "[Arithmetic]")
{
    Parser parser = CreateParser(U"100 - 42");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test multiplication of integers", "[Arithmetic]")
{
    Parser parser = CreateParser(U"7 * 8");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test arithmetic with doubles", "[Arithmetic]")
{
    Parser parser = CreateParser(U"3.14 + 2.71");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Float64);
}

TEST_CASE("test arithmetic type mismatch throws exception", "[Arithmetic][Error]")
{
    Parser parser = CreateParser(U"10 + 3.14");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.Visit(exp, &scope), TreeException);
}

// ============================================================================
// Comparison Operations Tests
// ============================================================================

TEST_CASE("test less than comparison", "[Comparison]")
{
    Parser parser = CreateParser(U"5 < 10");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test greater than comparison", "[Comparison]")
{
    Parser parser = CreateParser(U"100 > 50");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test equality comparison", "[Comparison]")
{
    Parser parser = CreateParser(U"42 == 42");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test inequality comparison", "[Comparison]")
{
    Parser parser = CreateParser(U"1 != 2");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test string comparison", "[Comparison]")
{
    Parser parser = CreateParser(U"\"hello\" < \"world\"");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test char comparison", "[Comparison]")
{
    Parser parser = CreateParser(U"'a' < 'z'");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

// ============================================================================
// Logical Operations Tests
// ============================================================================

TEST_CASE("test logical and", "[Logical]")
{
    Parser parser = CreateParser(U"true and false");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test logical or", "[Logical]")
{
    Parser parser = CreateParser(U"true or false");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test logical not", "[Logical][Unary]")
{
    Parser parser = CreateParser(U"not true");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test logical not with non-boolean throws exception", "[Logical][Unary][Error]")
{
    Parser parser = CreateParser(U"not 123");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.Visit(exp, &scope), TreeException);
}

TEST_CASE("test logical and with non-boolean throws exception", "[Logical][Error]")
{
    Parser parser = CreateParser(U"true and 42");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.Visit(exp, &scope), TreeException);
}

// ============================================================================
// Variable Tests
// ============================================================================

TEST_CASE("test undefined variable throws exception", "[Variable][Error]")
{
    Parser parser = CreateParser(U"{ undefinedVar; }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.Visit(exp, &scope), TreeException);
}

TEST_CASE("test variable assignment", "[Variable]")
{
    Parser parser = CreateParser(U"{ var x = 10; x = 20; }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Empty);
}

TEST_CASE("test variable type annotation", "[Variable]")
{
    Parser parser = CreateParser(U"{ var x: Int = 42; x; }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test variable type mismatch throws exception", "[Variable][Error]")
{
    Parser parser = CreateParser(U"{ var x: Int = 3.14; }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.Visit(exp, &scope), TreeException);
}

// ============================================================================
// Conditional Tests
// ============================================================================

TEST_CASE("test conditional with same branch types", "[Conditional]")
{
    Parser parser = CreateParser(U"if (true) { 10; } else { 20; }");
    auto exp = parser.Statement();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test conditional with non-boolean condition throws exception", "[Conditional][Error]")
{
    Parser parser = CreateParser(U"if (123) { 10; } else { 20; }");
    auto exp = parser.Statement();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.Visit(exp, &scope), TreeException);
}

// ============================================================================
// While Loop Tests
// ============================================================================

TEST_CASE("test while loop with non-boolean condition throws exception", "[WhileLoop][Error]")
{
    Parser parser = CreateParser(U"{ while (42) { 1; } }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.Visit(exp, &scope), TreeException);
}

// ============================================================================
// Function Call Tests
// ============================================================================

TEST_CASE("test function call with correct arguments", "[Function][Call]")
{
    Parser parser = CreateParser(U"module M { func add(a: Int, b: Int): Int { a + b; } func Main(): Int { add(1, 2); } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_NOTHROW(typeChecker.CheckNamespace(&scope));
}

TEST_CASE("test function call with wrong argument count throws exception", "[Function][Call][Error]")
{
    Parser parser = CreateParser(U"module M { func add(a: Int, b: Int): Int { a + b; } func Main(): Int { add(1); } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.CheckNamespace(&scope), TreeException);
}

TEST_CASE("test function call with wrong argument type throws exception", "[Function][Call][Error]")
{
    Parser parser = CreateParser(U"module M { func add(a: Int, b: Int): Int { a + b; } func Main(): Int { add(1, 3.14); } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.CheckNamespace(&scope), TreeException);
}

TEST_CASE("test calling non-callable throws exception", "[Function][Call][Error]")
{
    Parser parser = CreateParser(U"{ var x = 10; x(1); }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.Visit(exp, &scope), TreeException);
}

// ============================================================================
// Constant Tests
// ============================================================================

TEST_CASE("test integer constant", "[Constant]")
{
    Parser parser = CreateParser(U"42");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test float constant", "[Constant]")
{
    Parser parser = CreateParser(U"3.14159");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Float64);
}

TEST_CASE("test boolean constant true", "[Constant]")
{
    Parser parser = CreateParser(U"true");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test boolean constant false", "[Constant]")
{
    Parser parser = CreateParser(U"false");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test string constant", "[Constant]")
{
    Parser parser = CreateParser(U"\"hello world\"");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::String);
}

TEST_CASE("test character constant", "[Constant]")
{
    Parser parser = CreateParser(U"'x'");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Char);
}

// ============================================================================
// Block Expression Tests
// ============================================================================

TEST_CASE("test empty block", "[Block]")
{
    Parser parser = CreateParser(U"{ }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Empty);
}

TEST_CASE("test block returns last expression type", "[Block]")
{
    Parser parser = CreateParser(U"{ 1; 2; 3; }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test nested blocks with variable scoping", "[Block][Variable]")
{
    Parser parser = CreateParser(U"{ var x = 1; { var y = 2; x + y; }; }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}