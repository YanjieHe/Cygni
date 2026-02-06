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

TEST_CASE("test variable locating", "[Variable]")
{
    Parser parser = CreateParser(U"func Add(x: Int, y: Int): Int { var z = x + y; z; }");
    auto exp = parser.FunctionDeclarationStatement({});
    TypeChecker typeChecker(parser.GetNamespaceFactory(), parser.GetExpressionFactory());

    Scope<const Type *> scope;
    const Type *type = typeChecker.Visit(exp, &scope);
    REQUIRE(type->GetTypeCode() == TypeCode::Callable);
    const CallableType *callableType = static_cast<const CallableType *>(type);
    REQUIRE(callableType->Arguments().size() == 2);
    REQUIRE(callableType->Arguments().at(0)->GetTypeCode() == TypeCode::Int32);
    REQUIRE(callableType->Arguments().at(1)->GetTypeCode() == TypeCode::Int32);
    REQUIRE(callableType->GetReturnType()->GetTypeCode() == TypeCode::Int32);

    Scope<NameInfo> nameScope;
    NameLocator nameLocator(parser.GetNamespaceFactory());
    nameLocator.Visit(exp, &nameScope);

    bool checkedX = false;
    bool checkedY = false;
    bool checkedZ = false;
    for (const auto &item : nameLocator.NameInfoTable())
    {
        if (item.first.first->NodeType() == ExpressionType::Parameter)
        {
            const ParameterExpression *parameter = static_cast<const ParameterExpression *>(item.first.first);

            if (parameter->Name() == U"x")
            {
                REQUIRE(item.second.Kind() == LocationKind::FunctionVariable);
                REQUIRE(item.second.Number() == 0);
                checkedX = true;
            }
            else if (parameter->Name() == U"y")
            {
                REQUIRE(item.second.Kind() == LocationKind::FunctionVariable);
                REQUIRE(item.second.Number() == 1);
                checkedY = true;
            }
            else if (parameter->Name() == U"z")
            {
                REQUIRE(item.second.Kind() == LocationKind::FunctionVariable);
                REQUIRE(item.second.Number() == 2);
                checkedZ = true;
            }
        }
    }

    REQUIRE(checkedX);
    REQUIRE(checkedY);
    REQUIRE(checkedZ);
}