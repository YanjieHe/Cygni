#include <catch2/catch.hpp>

#include "LexicalAnalysis/Lexer.hpp"
#include "SyntaxAnalysis/Parser.hpp"
#include "SyntaxAnalysis/ParserException.hpp"
#include "Compilation/CompilationContext.hpp"
#include <spdlog/spdlog.h>

using namespace Cygni::LexicalAnalysis;
using namespace Cygni::SyntaxAnalysis;
using namespace Cygni::Expressions;
using namespace Cygni::Compilation;

static std::vector<Token> Tokenize(const std::shared_ptr<SourceCodeFile> &sourceCodeFile,
                                   const std::u32string &sourceCode)
{
    Lexer lexer(sourceCodeFile, sourceCode);

    return lexer.ReadAll();
}

static Parser CreateParser(CompilationContext &compilationContext, const std::u32string &sourceCode)
{
    std::shared_ptr<SourceCodeFile> sourceCodeFile = std::make_shared<SourceCodeFile>("source-code-file");
    std::vector<Token> tokens = Tokenize(sourceCodeFile, sourceCode);

    return Parser(tokens, sourceCodeFile, compilationContext);
}

TEST_CASE("test (15 * 72)", "[Arithmetic]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"15 * 72");

    auto exp = parser.ParseExpr();
    REQUIRE(exp->NodeType() == ExpressionType::Multiply);
    auto binaryExp = static_cast<BinaryExpression *>(exp);
    REQUIRE(binaryExp->Left() != nullptr);
    REQUIRE(binaryExp->Right() != nullptr);
    REQUIRE(binaryExp->Left()->NodeType() == ExpressionType::Constant);
    REQUIRE(binaryExp->Right()->NodeType() == ExpressionType::Constant);
}

TEST_CASE("test (135 + 27)", "[Arithmetic]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"135 + 27");

    auto exp = parser.ParseExpr();
    REQUIRE(exp->NodeType() == ExpressionType::Add);
    auto binaryExp = static_cast<BinaryExpression *>(exp);
    REQUIRE(binaryExp->Left() != nullptr);
    REQUIRE(binaryExp->Right() != nullptr);
    REQUIRE(binaryExp->Left()->NodeType() == ExpressionType::Constant);
    REQUIRE(binaryExp->Right()->NodeType() == ExpressionType::Constant);
}

TEST_CASE("test (27 / 9)", "[Arithmetic]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"27 / 9");

    auto exp = parser.ParseExpr();
    REQUIRE(exp->NodeType() == ExpressionType::Divide);
    auto binaryExp = static_cast<BinaryExpression *>(exp);
    REQUIRE(binaryExp->Left() != nullptr);
    REQUIRE(binaryExp->Right() != nullptr);
    REQUIRE(binaryExp->Left()->NodeType() == ExpressionType::Constant);
    REQUIRE(binaryExp->Right()->NodeType() == ExpressionType::Constant);
}

TEST_CASE("unary minus binds tighter than multiply", "[Unary]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"-5 * 2");

    auto exp = parser.ParseExpr();
    REQUIRE(exp->NodeType() == ExpressionType::Multiply);

    auto mul = static_cast<BinaryExpression *>(exp);
    REQUIRE(mul->Left()->NodeType() == ExpressionType::UnaryMinus);
    REQUIRE(mul->Right()->NodeType() == ExpressionType::Constant);
}

TEST_CASE("logical not binds tighter than and", "[Unary]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"not true and false");

    auto exp = parser.ParseOr();
    REQUIRE(exp->NodeType() == ExpressionType::And);

    auto andExp = static_cast<BinaryExpression *>(exp);
    REQUIRE(andExp->Left()->NodeType() == ExpressionType::Not);
    REQUIRE(andExp->Right()->NodeType() == ExpressionType::Constant);
}

TEST_CASE("test complex expression", "[Complex]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"(5 + 8) * 2");

    auto exp = parser.ParseExpr();
    REQUIRE(exp->NodeType() == ExpressionType::Multiply);
    auto multiplyExp = static_cast<BinaryExpression *>(exp);
    REQUIRE(multiplyExp->Left() != nullptr);
    REQUIRE(multiplyExp->Right() != nullptr);
    REQUIRE(multiplyExp->Left()->NodeType() == ExpressionType::Add);
    REQUIRE(multiplyExp->Right()->NodeType() == ExpressionType::Constant);
}

TEST_CASE("simple less-than comparison", "[Relation]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"1 < 2");

    auto exp = parser.ParseOr();
    REQUIRE(exp->NodeType() == ExpressionType::LessThan);
}

TEST_CASE("chained comparison is rejected", "[Relation]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"a < b < c");

    REQUIRE_THROWS_AS(parser.Statement(), ParserException);
}

TEST_CASE("test (true and false)", "[Logical]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"true and false");

    auto exp = parser.ParseOr();
    REQUIRE(exp->NodeType() == ExpressionType::And);
    auto binaryExp = static_cast<BinaryExpression *>(exp);
    REQUIRE(binaryExp->Left() != nullptr);
    REQUIRE(binaryExp->Right() != nullptr);
    REQUIRE(binaryExp->Left()->NodeType() == ExpressionType::Constant);
    REQUIRE(binaryExp->Right()->NodeType() == ExpressionType::Constant);
}

TEST_CASE("and binds tighter than or", "[Logical]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"true or false and true");

    auto exp = parser.ParseOr();
    REQUIRE(exp->NodeType() == ExpressionType::Or);

    auto orExp = static_cast<BinaryExpression *>(exp);
    REQUIRE(orExp->Right()->NodeType() == ExpressionType::And);
}

TEST_CASE("logical operators are left associative", "[Logical]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"true and false and true");

    auto exp = parser.ParseOr();
    REQUIRE(exp->NodeType() == ExpressionType::And);
}

TEST_CASE("test (x = 42)", "[Assignment]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"x = 42;");

    auto exp = parser.Statement();
    REQUIRE(exp->NodeType() == ExpressionType::Assign);
    auto assignExp = static_cast<BinaryExpression *>(exp);
    REQUIRE(assignExp->Left() != nullptr);
    REQUIRE(assignExp->Right() != nullptr);
    REQUIRE(assignExp->Left()->NodeType() == ExpressionType::Parameter);
    REQUIRE(assignExp->Right()->NodeType() == ExpressionType::Constant);
}

TEST_CASE("member access chains left to right", "[Postfix]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"a.b.c");

    auto exp = parser.ParseExpr();
    REQUIRE(exp->NodeType() == ExpressionType::MemberAccess);
}

TEST_CASE("member access has higher precedence than add", "[Postfix]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"a.b + c");

    auto exp = parser.ParseExpr();
    REQUIRE(exp->NodeType() == ExpressionType::Add);
}

TEST_CASE("test if-else statement", "[ControlFlow]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"if (true) { 10; } else { 20; }");

    auto exp = parser.Statement();
    REQUIRE(exp->NodeType() == ExpressionType::Conditional);
    auto conditionalExp = static_cast<ConditionalExpression *>(exp);
    REQUIRE(conditionalExp->Test() != nullptr);
    REQUIRE(conditionalExp->IfTrue() != nullptr);
    REQUIRE(conditionalExp->IfFalse() != nullptr);
}

TEST_CASE("if else-if else forms nested conditionals", "[ControlFlow]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"if (a) { 1; } else if (b) { 2; } else { 3; }");

    auto stmt = parser.Statement();
    REQUIRE(stmt->NodeType() == ExpressionType::Conditional);
}

TEST_CASE("test while statement", "[ControlFlow]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"while (true) { 35; }");

    auto exp = parser.Statement();
    REQUIRE(exp->NodeType() == ExpressionType::WhileLoop);
    auto loopExp = static_cast<WhileLoopExpression *>(exp);
    REQUIRE(loopExp->Condition() != nullptr);
    REQUIRE(loopExp->Body() != nullptr);

    REQUIRE(loopExp->Condition()->NodeType() == ExpressionType::Constant);
    REQUIRE(loopExp->Body()->NodeType() == ExpressionType::Block);
}

TEST_CASE("nested if inside while", "[ControlFlow]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"while (a) { if (b) { 1; } }");

    auto stmt = parser.Statement();
    REQUIRE(stmt->NodeType() == ExpressionType::WhileLoop);
}

TEST_CASE("nested while inside if", "[ControlFlow]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"if (a) { while (b) { 1; } }");

    auto stmt = parser.Statement();
    REQUIRE(stmt->NodeType() == ExpressionType::Conditional);
}

TEST_CASE("empty block is allowed", "[Block]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"{ }");

    auto exp = parser.ParseExpr();
    REQUIRE(exp->NodeType() == ExpressionType::Block);
}

TEST_CASE("block can contain multiple statements", "[Block]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"{ 1; 2; 3; }");

    auto exp = parser.ParseExpr();
    auto block = static_cast<BlockExpression *>(exp);
    REQUIRE(block->Expressions().size() == 3);
}

TEST_CASE("test function call", "[Function]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"f(12, 13);");

    auto exp = parser.Statement();
    REQUIRE(exp->NodeType() == ExpressionType::Call);
    auto callExp = static_cast<CallExpression *>(exp);
    REQUIRE(callExp->Function() != nullptr);
    REQUIRE(callExp->Arguments().size() == 2);

    REQUIRE(callExp->Function()->NodeType() == ExpressionType::Parameter);
    for (size_t i = 0; i < 2; i++)
    {
        REQUIRE(callExp->Arguments().at(i)->NodeType() == ExpressionType::Constant);
    }
}

TEST_CASE("function declaration with body", "[Function]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"module M { func f(): Int { 1; } }");

    REQUIRE_NOTHROW(parser.ParseNamespace());
}

TEST_CASE("function declaration without body", "[Function]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"module M { func f(): Void; }");

    REQUIRE_NOTHROW(parser.ParseNamespace());
}

TEST_CASE("nested function calls are allowed", "[Call]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"f()(1)");

    auto exp = parser.ParseExpr();
    REQUIRE(exp->NodeType() == ExpressionType::Call);
}

TEST_CASE("call after member access", "[Call]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"a.b(c)");

    auto exp = parser.ParseExpr();
    REQUIRE(exp->NodeType() == ExpressionType::Call);
}

TEST_CASE("local variable declaration inside block", "[Var]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"{ var x = 1; x; }");

    auto exp = parser.ParseExpr();
    auto block = static_cast<BlockExpression *>(exp);
    REQUIRE(block->Expressions().size() == 2);
}

TEST_CASE("typed local variable declaration", "[Var]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"var x: Int = 1;");

    auto stmt = parser.Statement();
    REQUIRE(stmt->NodeType() == ExpressionType::VariableDeclaration);
}

TEST_CASE("test parsing namespace path", "[Namespace]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"apple::banana::orange");

    std::vector<std::u32string> path = parser.ParseNamespacePath();
    REQUIRE(path.size() == 3);
    REQUIRE(path == std::vector<std::u32string>{U"apple", U"banana", U"orange"});
}

TEST_CASE("qualified namespace path parses as parameter expression", "[Scope]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"apple::banana::orange");

    auto exp = parser.ParseExpr();
    REQUIRE(exp->NodeType() == ExpressionType::Parameter);
}

TEST_CASE("test parsing structure definition with one field", "[Structure]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"struct Apple { weight: Double; }");

    StructureExpression *structureDefinition = parser.ParseStructureDefinition();
    REQUIRE(structureDefinition->Fields().GetAllItems().size() == 1);
    REQUIRE(structureDefinition->QualifiedName().size() == 1);
    REQUIRE(structureDefinition->QualifiedName().back() == U"Apple");
}

TEST_CASE("test parsing structure definition with two fields", "[Structure]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"struct Apple { weight: Double; price: Double; }");

    StructureExpression *structureDefinition = parser.ParseStructureDefinition();
    REQUIRE(structureDefinition->Fields().GetAllItems().size() == 2);
    REQUIRE(structureDefinition->QualifiedName().size() == 1);
    REQUIRE(structureDefinition->QualifiedName().back() == U"Apple");
}

TEST_CASE("test parsing structure definition with two fields inside a namespace", "[Structure]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"module Fruits { struct Apple { weight: Double; price: Double; } }");

    parser.ParseNamespace();
    StructureExpression *structureDefinition =
        parser.GetNamespaceFactory().SearchStructure(parser.GetNamespaceFactory().GetRoot(), {U"Fruits", U"Apple"});
    REQUIRE(structureDefinition != nullptr);
    REQUIRE(structureDefinition->Fields().GetAllItems().size() == 2);
    REQUIRE(structureDefinition->QualifiedName().size() == 2);
    REQUIRE(structureDefinition->QualifiedName().at(0) == U"Fruits");
    REQUIRE(structureDefinition->QualifiedName().at(1) == U"Apple");
}

TEST_CASE("annotation on function declaration", "[Annotation]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"module M { @A(x=\"1\") func f(): Void; }");

    REQUIRE_NOTHROW(parser.ParseNamespace());
}

TEST_CASE("if without parentheses is rejected", "[Error]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"if true { 1; }");

    REQUIRE_THROWS_AS(parser.Statement(), ParserException);
}

TEST_CASE("missing initializer in variable declaration", "[Error]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"var x = ;");

    REQUIRE_THROWS_AS(parser.Statement(), ParserException);
}

TEST_CASE("missing semicolon in new expression", "[Error]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"new Foo { a = 1 }");

    REQUIRE_THROWS_AS(parser.ParseExpr(), ParserException);
}

// ============================================================================
// Structure with Methods Tests
// ============================================================================

TEST_CASE("parse struct with fields only", "[Structure]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"module M { struct Point { x: Int; y: Int; } }");
    parser.ParseNamespace();

    StructureExpression *structDef =
        parser.GetNamespaceFactory().SearchStructure(parser.GetNamespaceFactory().GetRoot(), {U"M", U"Point"});
    REQUIRE(structDef != nullptr);
    REQUIRE(structDef->Fields().GetAllItems().size() == 2);
    REQUIRE(structDef->Methods().GetAllItems().size() == 0);
}

TEST_CASE("parse struct with one method", "[Structure][Method]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { struct Counter { value: Int; func increment(): Int { this.value + 1; } } }");
    parser.ParseNamespace();

    StructureExpression *structDef =
        parser.GetNamespaceFactory().SearchStructure(parser.GetNamespaceFactory().GetRoot(), {U"M", U"Counter"});
    REQUIRE(structDef != nullptr);
    REQUIRE(structDef->Fields().GetAllItems().size() == 1);
    REQUIRE(structDef->Fields().ContainsKey(U"value"));
    REQUIRE(structDef->Methods().GetAllItems().size() == 1);
    REQUIRE(structDef->Methods().ContainsKey(U"increment"));
}

TEST_CASE("parse struct with multiple methods", "[Structure][Method]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { struct Calc { "
        U"  value: Int; "
        U"  func add(x: Int): Int { this.value + x; } "
        U"  func sub(x: Int): Int { this.value - x; } "
        U"} }");
    parser.ParseNamespace();

    StructureExpression *structDef =
        parser.GetNamespaceFactory().SearchStructure(parser.GetNamespaceFactory().GetRoot(), {U"M", U"Calc"});
    REQUIRE(structDef != nullptr);
    REQUIRE(structDef->Fields().GetAllItems().size() == 1);
    REQUIRE(structDef->Methods().GetAllItems().size() == 2);
    REQUIRE(structDef->Methods().ContainsKey(U"add"));
    REQUIRE(structDef->Methods().ContainsKey(U"sub"));
}

TEST_CASE("parse struct with methods only no fields", "[Structure][Method]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext,
        U"module M { struct Greeter { func greet(): Int { 42; } } }");
    parser.ParseNamespace();

    StructureExpression *structDef =
        parser.GetNamespaceFactory().SearchStructure(parser.GetNamespaceFactory().GetRoot(), {U"M", U"Greeter"});
    REQUIRE(structDef != nullptr);
    REQUIRE(structDef->Fields().GetAllItems().size() == 0);
    REQUIRE(structDef->Methods().GetAllItems().size() == 1);
    REQUIRE(structDef->Methods().ContainsKey(U"greet"));
}

TEST_CASE("parse new expression with field initialization", "[New]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"new Point { x = 1; y = 2; }");

    auto exp = parser.ParseOr();
    REQUIRE(exp->NodeType() == ExpressionType::New);
    const NewExpression *newExp = static_cast<const NewExpression *>(exp);
    REQUIRE(newExp->GetTypeSyntax() != nullptr);
    REQUIRE(newExp->FieldsInitialization().GetAllKeys().size() == 2);
    REQUIRE(newExp->FieldsInitialization().ContainsKey(U"x"));
    REQUIRE(newExp->FieldsInitialization().ContainsKey(U"y"));
}

TEST_CASE("parse this dot field as member access", "[MemberAccess]")
{
    CompilationContext compilationContext;
    Parser parser = CreateParser(compilationContext, U"this.value");

    auto exp = parser.ParseOr();
    REQUIRE(exp->NodeType() == ExpressionType::MemberAccess);
    const MemberExpression *memberExp = static_cast<const MemberExpression *>(exp);
    REQUIRE(memberExp->FieldName() == U"value");
    REQUIRE(memberExp->GetExpression()->NodeType() == ExpressionType::Parameter);
}