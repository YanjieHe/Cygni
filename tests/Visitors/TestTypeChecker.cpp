#include <catch2/catch.hpp>

#include "Compilation/CompilationContext.hpp"
#include "LexicalAnalysis/Lexer.hpp"
#include "SyntaxAnalysis/Parser.hpp"
#include "Visitors/TypeChecker.hpp"

using namespace Cygni::LexicalAnalysis;
using namespace Cygni::SyntaxAnalysis;
using namespace Cygni::Expressions;
using namespace Cygni::Visitors;

static Parser CreateParser(Cygni::Compilation::CompilationContext &compilationContext, const std::u32string &sourceCode)
{
    std::shared_ptr<SourceCodeFile> sourceCodeFile = std::make_shared<SourceCodeFile>("source-code-file");
    Lexer lexer(sourceCodeFile, sourceCode);
    std::vector<Token> tokens = lexer.ReadAll();

    return Parser(tokens, sourceCodeFile, compilationContext);
}

TEST_CASE("test (36 / 9)", "[Arithmetic]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"36 / 9");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test variable declaration", "[Variable]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"{ var a = 'a'; a; }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Char);
}

TEST_CASE("test shadowing outer variable", "[Variable]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"{ var x = 10; { var x = \"variable x\"; x; }; }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::String);
}

TEST_CASE("test conditional", "[Conditional]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"if (1 < 2) { 10.3; } else { false; }");
    auto exp = parser.Statement();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Union);
}

TEST_CASE("test loop", "[WhileLoop]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"{ var i = 0; while (i < 10) { i = i + 1; } }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Empty);
}

TEST_CASE("test function declaration", "[Function]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"func f(x: Double, y: Double): Double { x + y; }");
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
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"15 + 27");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test subtraction of integers", "[Arithmetic]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"100 - 42");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test multiplication of integers", "[Arithmetic]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"7 * 8");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test arithmetic with doubles", "[Arithmetic]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"3.14 + 2.71");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Float64);
}

TEST_CASE("test arithmetic type mismatch throws exception", "[Arithmetic][Error]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"10 + 3.14");
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
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"5 < 10");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test greater than comparison", "[Comparison]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"100 > 50");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test equality comparison", "[Comparison]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"42 == 42");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test inequality comparison", "[Comparison]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"1 != 2");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test string comparison", "[Comparison]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"\"hello\" < \"world\"");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test char comparison", "[Comparison]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"'a' < 'z'");
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
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"true and false");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test logical or", "[Logical]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"true or false");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test logical not", "[Logical][Unary]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"not true");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test logical not with non-boolean throws exception", "[Logical][Unary][Error]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"not 123");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.Visit(exp, &scope), TreeException);
}

TEST_CASE("test logical and with non-boolean throws exception", "[Logical][Error]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"true and 42");
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
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"{ undefinedVar; }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.Visit(exp, &scope), TreeException);
}

TEST_CASE("test variable assignment", "[Variable]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"{ var x = 10; x = 20; }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Empty);
}

TEST_CASE("test variable type annotation", "[Variable]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"{ var x: Int = 42; x; }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test variable type mismatch throws exception", "[Variable][Error]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"{ var x: Int = 3.14; }");
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
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"if (true) { 10; } else { 20; }");
    auto exp = parser.Statement();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test conditional with non-boolean condition throws exception", "[Conditional][Error]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"if (123) { 10; } else { 20; }");
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
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"{ while (42) { 1; } }");
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
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser =
        CreateParser(compilationContext, U"module M { func add(a: Int, b: Int): Int { a + b; } func Main(): Int { add(1, 2); } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_NOTHROW(typeChecker.CheckNamespace(&scope));
}

TEST_CASE("test function call with wrong argument count throws exception", "[Function][Call][Error]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"module M { func add(a: Int, b: Int): Int { a + b; } func Main(): Int { add(1); } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.CheckNamespace(&scope), TreeException);
}

TEST_CASE("test function call with wrong argument type throws exception", "[Function][Call][Error]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser =
        CreateParser(compilationContext, U"module M { func add(a: Int, b: Int): Int { a + b; } func Main(): Int { add(1, 3.14); } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.CheckNamespace(&scope), TreeException);
}

TEST_CASE("test calling non-callable throws exception", "[Function][Call][Error]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"{ var x = 10; x(1); }");
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
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"42");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test float constant", "[Constant]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"3.14159");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Float64);
}

TEST_CASE("test boolean constant true", "[Constant]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"true");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test boolean constant false", "[Constant]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"false");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Boolean);
}

TEST_CASE("test string constant", "[Constant]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"\"hello world\"");
    auto exp = parser.ParseOr();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::String);
}

TEST_CASE("test character constant", "[Constant]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"'x'");
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
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"{ }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Empty);
}

TEST_CASE("test block returns last expression type", "[Block]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"{ 1; 2; 3; }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

TEST_CASE("test nested blocks with variable scoping", "[Block][Variable]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"{ var x = 1; { var y = 2; x + y; }; }");
    auto exp = parser.ParseBlock();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Int32);
}

// ============================================================================
// Structure and Method Tests
// ============================================================================

TEST_CASE("type check struct with fields only", "[Structure]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { struct Point { x: Int; y: Int; } "
        U"func Main(): Int { var p = new Point { x = 1; y = 2; }; p.x; } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_NOTHROW(typeChecker.CheckNamespace(&scope));
}

TEST_CASE("type check struct method returns correct type", "[Structure][Method]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { struct Counter { value: Int; "
        U"  func get(): Int { this.value; } } "
        U"func Main(): Int { var c = new Counter { value = 10; }; c.get(); } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_NOTHROW(typeChecker.CheckNamespace(&scope));
}

TEST_CASE("type check struct method with parameters", "[Structure][Method]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { struct Calc { value: Int; "
        U"  func add(x: Int): Int { this.value + x; } } "
        U"func Main(): Int { var c = new Calc { value = 5; }; c.add(3); } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_NOTHROW(typeChecker.CheckNamespace(&scope));
}

TEST_CASE("type check struct method this field access", "[Structure][Method]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { struct Rect { width: Int; height: Int; "
        U"  func area(): Int { this.width * this.height; } } "
        U"func Main(): Int { var r = new Rect { width = 3; height = 4; }; r.area(); } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_NOTHROW(typeChecker.CheckNamespace(&scope));
}

TEST_CASE("type check method call with wrong argument count throws", "[Structure][Method][Error]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { struct Adder { base: Int; "
        U"  func add(x: Int): Int { this.base + x; } } "
        U"func Main(): Int { var a = new Adder { base = 1; }; a.add(1, 2); } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.CheckNamespace(&scope), TreeException);
}

TEST_CASE("type check method call with wrong argument type throws", "[Structure][Method][Error]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { struct Adder { base: Int; "
        U"  func add(x: Int): Int { this.base + x; } } "
        U"func Main(): Int { var a = new Adder { base = 1; }; a.add(3.14); } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.CheckNamespace(&scope), TreeException);
}

TEST_CASE("type check calling non-existent method throws", "[Structure][Method][Error]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { struct Point { x: Int; } "
        U"func Main(): Int { var p = new Point { x = 1; }; p.nonexistent(); } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.CheckNamespace(&scope), TreeException);
}

TEST_CASE("type check method return type mismatch throws", "[Structure][Method][Error]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { struct Counter { value: Int; "
        U"  func getAsDouble(): Double { this.value; } } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.CheckNamespace(&scope), TreeException);
}

TEST_CASE("type check method call on non-struct throws", "[Structure][Method][Error]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { func Main(): Int { var n = 10; n.increment(); } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.CheckNamespace(&scope), TreeException);
}

TEST_CASE("type check struct with multiple methods", "[Structure][Method]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { struct Vec2 { x: Int; y: Int; "
        U"  func getX(): Int { this.x; } "
        U"  func getY(): Int { this.y; } "
        U"  func sum(): Int { this.x + this.y; } } "
        U"func Main(): Int { var v = new Vec2 { x = 3; y = 4; }; v.sum(); } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_NOTHROW(typeChecker.CheckNamespace(&scope));
}

// ============================================================================
// New Expression Error Tests
// ============================================================================

TEST_CASE("type check new with field type mismatch throws", "[Structure][New][Error]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { struct Point { x: Int; y: Int; } "
        U"func Main(): Int { var p = new Point { x = 1; y = 3.14; }; p.x; } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.CheckNamespace(&scope), TreeException);
}

TEST_CASE("type check new with missing fields throws", "[Structure][New][Error]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { struct Point { x: Int; y: Int; } "
        U"func Main(): Int { var p = new Point { x = 1; }; p.x; } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.CheckNamespace(&scope), TreeException);
}

TEST_CASE("type check new with non-existent field throws", "[Structure][New][Error]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { struct Point { x: Int; y: Int; } "
        U"func Main(): Int { var p = new Point { x = 1; y = 2; z = 3; }; p.x; } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_THROWS_AS(typeChecker.CheckNamespace(&scope), TreeException);
}

// ============================================================================
// Field Access Type Tests
// ============================================================================

TEST_CASE("type check field access returns correct type", "[Structure][MemberAccess]")
{
    Cygni::Compilation::CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { struct Pair { first: Int; second: Double; } "
        U"func GetFirst(): Int { var p = new Pair { first = 1; second = 2.0; }; p.first; } "
        U"func GetSecond(): Double { var p = new Pair { first = 1; second = 2.0; }; p.second; } }");
    parser.ParseNamespace();
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    REQUIRE_NOTHROW(typeChecker.CheckNamespace(&scope));
}