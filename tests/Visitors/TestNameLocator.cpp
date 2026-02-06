#include <catch2/catch.hpp>

#include "LexicalAnalysis/Lexer.hpp"
#include "SyntaxAnalysis/Parser.hpp"
#include "Visitors/NameLocator.hpp"
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

static std::optional<NameInfo> FindLocalVariableInfo(NameLocator &nameLocator, const std::u32string &name)
{
    for (const auto &item : nameLocator.NameInfoTable())
    {
        if (item.first.second == LocationKind::FunctionVariable)
        {
            if (item.first.first->NodeType() == ExpressionType::Parameter)
            {
                const ParameterExpression *param = static_cast<const ParameterExpression *>(item.first.first);
                if (param->Name() == name)
                {
                    return item.second;
                }
            }
            else if (item.first.first->NodeType() == ExpressionType::VariableDeclaration)
            {
                const VariableDeclarationExpression *varDecl =
                    static_cast<const VariableDeclarationExpression *>(item.first.first);
                if (varDecl->Name() == name)
                {
                    return item.second;
                }
            }
        }
    }
    return std::nullopt;
}

TEST_CASE("test variable locating", "[Variable]")
{
    Parser parser = CreateParser(U"func Add(x: Int, y: Int): Int { var z = x + y; z; }");
    auto exp = parser.FunctionDeclarationStatement({});
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    typeChecker.Visit(exp, &scope);

    Scope<NameInfo> nameScope;
    NameLocator nameLocator(parser.GetNamespaceFactory());
    nameLocator.Visit(exp, &nameScope);

    auto x = FindLocalVariableInfo(nameLocator, U"x");
    auto y = FindLocalVariableInfo(nameLocator, U"y");
    auto z = FindLocalVariableInfo(nameLocator, U"z");
    REQUIRE(x.has_value());
    REQUIRE(y.has_value());
    REQUIRE(z.has_value());
    REQUIRE(x.value().Kind() == LocationKind::FunctionVariable);
    REQUIRE(x.value().Number() == 0);
    REQUIRE(y.value().Kind() == LocationKind::FunctionVariable);
    REQUIRE(y.value().Number() == 1);
    REQUIRE(z.value().Kind() == LocationKind::FunctionVariable);
    REQUIRE(z.value().Number() == 2);
}

// ============================================================================
// Constant Tests
// ============================================================================

TEST_CASE("test single constant locating", "[Constant]")
{
    Parser parser = CreateParser(U"func GetValue(): Int { 42; }");
    auto exp = parser.FunctionDeclarationStatement({});
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> typeScope;
    typeChecker.Visit(exp, &typeScope);

    Scope<NameInfo> nameScope;
    NameLocator nameLocator(parser.GetNamespaceFactory());
    nameLocator.Visit(exp, &nameScope);

    // Check that the lambda has constant count info
    REQUIRE(nameLocator.ExistsNameInfo(exp, LocationKind::FunctionConstantCount));
    REQUIRE(nameLocator.GetNameInfo(exp, LocationKind::FunctionConstantCount).Number() == 1);
}

TEST_CASE("test multiple constants locating", "[Constant]")
{
    Parser parser = CreateParser(U"func Sum(): Int { 1 + 2 + 3; }");
    auto exp = parser.FunctionDeclarationStatement({});
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> typeScope;
    typeChecker.Visit(exp, &typeScope);

    Scope<NameInfo> nameScope;
    NameLocator nameLocator(parser.GetNamespaceFactory());
    nameLocator.Visit(exp, &nameScope);

    REQUIRE(nameLocator.ExistsNameInfo(exp, LocationKind::FunctionConstantCount));
    REQUIRE(nameLocator.GetNameInfo(exp, LocationKind::FunctionConstantCount).Number() == 3);
}

// ============================================================================
// Function Variable Count Tests
// ============================================================================

TEST_CASE("test function variable count", "[Function]")
{
    Parser parser = CreateParser(U"func Compute(a: Int, b: Int): Int { var c = a; var d = b; c + d; }");
    auto exp = parser.FunctionDeclarationStatement({});
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> typeScope;
    typeChecker.Visit(exp, &typeScope);

    Scope<NameInfo> nameScope;
    NameLocator nameLocator(parser.GetNamespaceFactory());
    nameLocator.Visit(exp, &nameScope);

    // 2 parameters + 2 local variables = 4
    REQUIRE(nameLocator.ExistsNameInfo(exp, LocationKind::FunctionVariableCount));
    REQUIRE(nameLocator.GetNameInfo(exp, LocationKind::FunctionVariableCount).Number() == 4);
}

// ============================================================================
// Namespace Level Tests (Function, Global Variable, Structure)
// ============================================================================

TEST_CASE("test function locating in namespace", "[Function][Namespace]")
{
    Parser parser = CreateParser(U"module M { func Add(x: Int, y: Int): Int { x + y; } }");
    parser.ParseNamespace();

    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());
    Scope<const Type *> typeScope;
    typeChecker.CheckNamespace(&typeScope);

    Scope<NameInfo> nameScope;
    NameLocator nameLocator(parser.GetNamespaceFactory());
    nameLocator.InitializeSymbolCounters(&nameScope);
    nameLocator.RegisterAllInfo(&nameScope);

    LambdaExpression *funcDecl =
        parser.GetNamespaceFactory().SearchFunction(parser.GetNamespaceFactory().GetRoot(), {U"M", U"Add"});
    REQUIRE(funcDecl != nullptr);
    REQUIRE(nameLocator.ExistsNameInfo(funcDecl, LocationKind::Function));
    REQUIRE(nameLocator.GetNameInfo(funcDecl, LocationKind::Function).Number() == 0);
}

TEST_CASE("test multiple functions locating in namespace", "[Function][Namespace]")
{
    Parser parser = CreateParser(U"module M { func First(): Int { 1; } func Second(): Int { 2; } }");
    parser.ParseNamespace();

    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());
    Scope<const Type *> typeScope;
    typeChecker.CheckNamespace(&typeScope);

    Scope<NameInfo> nameScope;
    NameLocator nameLocator(parser.GetNamespaceFactory());
    nameLocator.InitializeSymbolCounters(&nameScope);
    nameLocator.RegisterAllInfo(&nameScope);

    LambdaExpression *first =
        parser.GetNamespaceFactory().SearchFunction(parser.GetNamespaceFactory().GetRoot(), {U"M", U"First"});
    LambdaExpression *second =
        parser.GetNamespaceFactory().SearchFunction(parser.GetNamespaceFactory().GetRoot(), {U"M", U"Second"});

    REQUIRE(first != nullptr);
    REQUIRE(second != nullptr);

    REQUIRE(nameLocator.GetNameInfo(first, LocationKind::Function).Number() == 0);
    REQUIRE(nameLocator.GetNameInfo(second, LocationKind::Function).Number() == 1);

    // Check global function count
    REQUIRE(nameScope.Get(GLOBAL_FUNCTION_COUNT).Number() == 2);
}

TEST_CASE("test global variable locating", "[GlobalVariable][Namespace]")
{
    Parser parser = CreateParser(U"module M { var count: Int = 0; func Get(): Int { count; } }");
    parser.ParseNamespace();

    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());
    Scope<const Type *> typeScope;
    typeChecker.CheckNamespace(&typeScope);

    Scope<NameInfo> nameScope;
    NameLocator nameLocator(parser.GetNamespaceFactory());
    nameLocator.InitializeSymbolCounters(&nameScope);
    nameLocator.RegisterAllInfo(&nameScope);

    VariableDeclarationExpression *varDecl =
        parser.GetNamespaceFactory().SearchGlobalVariable(parser.GetNamespaceFactory().GetRoot(), {U"M", U"count"});
    REQUIRE(varDecl != nullptr);
    REQUIRE(nameLocator.ExistsNameInfo(varDecl, LocationKind::GlobalVariable));
    REQUIRE(nameLocator.GetNameInfo(varDecl, LocationKind::GlobalVariable).Number() == 0);

    // Check global variable count
    REQUIRE(nameScope.Get(GLOBAL_VARIABLE_COUNT).Number() == 1);
}

TEST_CASE("test structure locating", "[Structure][Namespace]")
{
    Parser parser = CreateParser(U"module M { struct Point { x: Int; y: Int; } }");
    parser.ParseNamespace();

    Scope<NameInfo> nameScope;
    NameLocator nameLocator(parser.GetNamespaceFactory());
    nameLocator.InitializeSymbolCounters(&nameScope);
    nameLocator.RegisterAllInfo(&nameScope);

    StructureExpression *structDecl =
        parser.GetNamespaceFactory().SearchStructure(parser.GetNamespaceFactory().GetRoot(), {U"M", U"Point"});
    REQUIRE(structDecl != nullptr);
    REQUIRE(nameLocator.ExistsNameInfo(structDecl, LocationKind::Structure));
    REQUIRE(nameLocator.GetNameInfo(structDecl, LocationKind::Structure).Number() == 0);

    // Check global structure count
    REQUIRE(nameScope.Get(GLOBAL_STRUCTURE_COUNT).Number() == 1);
}

// ============================================================================
// CheckNamespace Tests
// ============================================================================

TEST_CASE("test CheckNamespace processes function body", "[Namespace]")
{
    Parser parser = CreateParser(U"module M { func Square(n: Int): Int { n * n; } }");
    parser.ParseNamespace();

    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());
    Scope<const Type *> typeScope;
    typeChecker.CheckNamespace(&typeScope);

    Scope<NameInfo> nameScope;
    NameLocator nameLocator(parser.GetNamespaceFactory());
    nameLocator.InitializeSymbolCounters(&nameScope);
    nameLocator.RegisterAllInfo(&nameScope);
    nameLocator.CheckNamespace(&nameScope);

    LambdaExpression *funcDecl =
        parser.GetNamespaceFactory().SearchFunction(parser.GetNamespaceFactory().GetRoot(), {U"M", U"Square"});
    REQUIRE(funcDecl != nullptr);

    // After CheckNamespace, function should have variable count and constant count
    REQUIRE(nameLocator.ExistsNameInfo(funcDecl, LocationKind::FunctionVariableCount));
    REQUIRE(nameLocator.GetNameInfo(funcDecl, LocationKind::FunctionVariableCount).Number() == 1); // parameter n

    REQUIRE(nameLocator.ExistsNameInfo(funcDecl, LocationKind::FunctionConstantCount));
}

// ============================================================================
// Nested Scope Tests
// ============================================================================

TEST_CASE("test nested block variable locating", "[Variable][Block]")
{
    Parser parser = CreateParser(U"func Test(a: Int): Int { var b = 1; { var c = 2; a + b + c; }; }");
    auto exp = parser.FunctionDeclarationStatement({});
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> typeScope;
    typeChecker.Visit(exp, &typeScope);

    Scope<NameInfo> nameScope;
    NameLocator nameLocator(parser.GetNamespaceFactory());
    nameLocator.Visit(exp, &nameScope);

    auto a = FindLocalVariableInfo(nameLocator, U"a");
    auto b = FindLocalVariableInfo(nameLocator, U"b");
    auto c = FindLocalVariableInfo(nameLocator, U"c");

    REQUIRE(a.has_value());
    REQUIRE(b.has_value());
    REQUIRE(c.has_value());

    REQUIRE(a.value().Number() == 0);
    REQUIRE(b.value().Number() == 1);
    REQUIRE(c.value().Number() == 2);

    // Total variable count should be 3
    REQUIRE(nameLocator.GetNameInfo(exp, LocationKind::FunctionVariableCount).Number() == 3);
}

// ============================================================================
// Conditional and Loop Tests
// ============================================================================

TEST_CASE("test conditional expression locating", "[Conditional]")
{
    Parser parser = CreateParser(U"func Max(a: Int, b: Int): Int { if (a > b) { a; } else { b; } }");
    auto exp = parser.FunctionDeclarationStatement({});
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> typeScope;
    typeChecker.Visit(exp, &typeScope);

    Scope<NameInfo> nameScope;
    NameLocator nameLocator(parser.GetNamespaceFactory());
    nameLocator.Visit(exp, &nameScope);

    auto a = FindLocalVariableInfo(nameLocator, U"a");
    auto b = FindLocalVariableInfo(nameLocator, U"b");

    REQUIRE(a.has_value());
    REQUIRE(b.has_value());
    REQUIRE(a.value().Number() == 0);
    REQUIRE(b.value().Number() == 1);
}

TEST_CASE("test while loop variable locating", "[WhileLoop]")
{
    Parser parser = CreateParser(U"func Count(n: Int): Int { var i = 0; while (i < n) { i = i + 1; } i; }");
    auto exp = parser.FunctionDeclarationStatement({});
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> typeScope;
    typeChecker.Visit(exp, &typeScope);

    Scope<NameInfo> nameScope;
    NameLocator nameLocator(parser.GetNamespaceFactory());
    nameLocator.Visit(exp, &nameScope);

    auto n = FindLocalVariableInfo(nameLocator, U"n");
    auto i = FindLocalVariableInfo(nameLocator, U"i");

    REQUIRE(n.has_value());
    REQUIRE(i.has_value());
    REQUIRE(n.value().Number() == 0);
    REQUIRE(i.value().Number() == 1);
}