#include "SyntaxAnalysis/Parser.hpp"

#include <magic_enum/magic_enum.hpp>

#include "SyntaxAnalysis/ParserException.hpp"
#include "Utility/Format.hpp"
#include "Utility/UTF32Functions.hpp"

#include <spdlog/spdlog.h>

using std::unordered_map;
using std::vector;

namespace Cygni
{
namespace SyntaxAnalysis
{

using LexicalAnalysis::SourceCodeFile;
using LexicalAnalysis::Token;
using LexicalAnalysis::TokenTag;
using namespace Expressions;

using Utility::Format;

Parser::Parser(std::vector<Token> tokens, std::shared_ptr<SourceCodeFile> document,
               Compilation::CompilationContext &compilationContext)
    : tokens{tokens}, document{document}, offset{0}, compilationContext{compilationContext}
{
    namespaceStack.push(GetNamespaceFactory().GetRoot());
}

const Token &Parser::Match(TokenTag tag)
{
    if (tag == Look().tag)
    {
        const Token &t = Look();
        Advance();
        return t;
    }
    else
    {
        spdlog::error("Source code file: {}, line: {}, column: {}. Expecting '{}', got '{}'.", document->FileName(),
                      Look().line + 1, Look().column + 1, Utility::EnumToString(tag),
                      Utility::EnumToString(Look().tag));

        throw ParserException(
            __FILE__, __LINE__,
            SourceRange(document, Look().line, Look().column, Look().line, Look().column + Look().text.size()),
            "Expecting '" + Utility::EnumToString(tag) + "', got '" + Utility::EnumToString(Look().tag) + "'.",
            nullptr);
    }
}

ExpPtr Parser::Statement()
{
    switch (Look().tag)
    {
    case TokenTag::If:
        return IfStatement();
    case TokenTag::While:
        return WhileStatement();
    case TokenTag::Var: {
        auto variableDeclaration = VariableDeclarationStatement();
        Match(TokenTag::Semicolon);
        return variableDeclaration;
    }
    default: {
        auto expression = ParseAssign();
        Match(TokenTag::Semicolon);
        return expression;
    }
    }
}

ExpPtr Parser::ParseAssign()
{
    const Token &start = Look();
    auto x = ParseOr();
    if (Look().tag == TokenTag::Assign)
    {
        Match(TokenTag::Assign);
        auto y = ParseOr();
        return GetExpressionFactory().Create<BinaryExpression>(Pos(start), ExpressionType::Assign, x, y);
    }
    else
    {
        return x;
    }
}

ExpPtr Parser::ParseOr()
{
    const Token &start = Look();
    auto x = ParseAnd();
    while (Look().tag == TokenTag::Or)
    {
        Match(TokenTag::Or);
        auto y = ParseAnd();
        x = GetExpressionFactory().Create<BinaryExpression>(Pos(start), ExpressionType::Or, x, y);
    }
    return x;
}

ExpPtr Parser::ParseAnd()
{
    const Token &start = Look();
    auto x = ParseNot();
    while (Look().tag == TokenTag::And)
    {
        Match(TokenTag::And);
        auto y = ParseNot();
        x = GetExpressionFactory().Create<BinaryExpression>(Pos(start), ExpressionType::And, x, y);
    }
    return x;
}

ExpPtr Parser::ParseNot()
{
    const Token &start = Look();
    if (Look().tag == TokenTag::Not)
    {
        Match(TokenTag::Not);
        auto x = ParseEquality();
        return GetExpressionFactory().Create<UnaryExpression>(Pos(start), ExpressionType::Not, x, nullptr);
    }
    else
    {
        return ParseEquality();
    }
}

ExpPtr Parser::ParseEquality()
{
    const Token &start = Look();
    auto x = ParseRelation();
    while (Look().tag == TokenTag::Equal || Look().tag == TokenTag::NotEqual)
    {
        Token t = Look();
        Advance();
        auto y = ParseRelation();
        if (t.tag == TokenTag::Equal)
        {
            x = GetExpressionFactory().Create<BinaryExpression>(Pos(start), ExpressionType::Equal, x, y);
        }
        else
        {
            x = GetExpressionFactory().Create<BinaryExpression>(Pos(start), ExpressionType::NotEqual, x, y);
        }
    }
    return x;
}

ExpPtr Parser::ParseRelation()
{
    const Token &start = Look();
    auto x = ParseExpr();
    if (Look().tag == TokenTag::GreaterThan || Look().tag == TokenTag::LessThan ||
        Look().tag == TokenTag::GreaterThanOrEqual || Look().tag == TokenTag::LessThanOrEqual)
    {
        Token t = Look();
        Advance();
        auto y = ParseExpr();
        if (t.tag == TokenTag::GreaterThan)
        {
            return GetExpressionFactory().Create<BinaryExpression>(Pos(start), ExpressionType::GreaterThan, x, y);
        }
        else if (t.tag == TokenTag::LessThan)
        {
            return GetExpressionFactory().Create<BinaryExpression>(Pos(start), ExpressionType::LessThan, x, y);
        }
        else if (t.tag == TokenTag::GreaterThanOrEqual)
        {
            return GetExpressionFactory().Create<BinaryExpression>(Pos(start), ExpressionType::GreaterThanOrEqual, x,
                                                                   y);
        }
        else
        {
            return GetExpressionFactory().Create<BinaryExpression>(Pos(start), ExpressionType::LessThanOrEqual, x, y);
        }
    }
    else
    {
        return x;
    }
}

ExpPtr Parser::ParseExpr()
{
    const Token &start = Look();
    auto x = ParseTerm();
    while (Look().tag == TokenTag::Add || Look().tag == TokenTag::Subtract)
    {
        Token t = Look();
        Advance();
        auto y = ParseTerm();
        if (t.tag == TokenTag::Add)
        {
            x = GetExpressionFactory().Create<BinaryExpression>(Pos(start), ExpressionType::Add, x, y);
        }
        else
        {
            x = GetExpressionFactory().Create<BinaryExpression>(Pos(start), ExpressionType::Subtract, x, y);
        }
    }
    return x;
}

ExpPtr Parser::ParseTerm()
{
    const Token &start = Look();
    auto x = ParseUnary();
    while (Look().tag == TokenTag::Multiply || Look().tag == TokenTag::Divide)
    {
        Token t = Look();
        Advance();
        auto y = ParseUnary();
        if (t.tag == TokenTag::Multiply)
        {
            x = GetExpressionFactory().Create<BinaryExpression>(Pos(start), ExpressionType::Multiply, x, y);
        }
        else
        {
            x = GetExpressionFactory().Create<BinaryExpression>(Pos(start), ExpressionType::Divide, x, y);
        }
    }
    return x;
}

ExpPtr Parser::ParseUnary()
{
    const Token &start = Look();
    if (Look().tag == TokenTag::Add)
    {
        Advance();
        auto x = ParseUnary();
        return GetExpressionFactory().Create<UnaryExpression>(Pos(start), ExpressionType::UnaryPlus, x, nullptr);
    }
    else if (Look().tag == TokenTag::Subtract)
    {
        Advance();
        auto x = ParseUnary();
        return GetExpressionFactory().Create<UnaryExpression>(Pos(start), ExpressionType::UnaryMinus, x, nullptr);
    }
    else if (Look().tag == TokenTag::Not)
    {
        Advance();
        auto x = ParseUnary();
        return GetExpressionFactory().Create<UnaryExpression>(Pos(start), ExpressionType::Not, x, nullptr);
    }
    else
    {
        return ParsePostfix();
    }
}

ExpPtr Parser::ParsePostfix()
{
    auto x = ParseFactor();
    while (Look().tag == TokenTag::LeftParenthesis || Look().tag == TokenTag::LeftBracket ||
           Look().tag == TokenTag::Dot || Look().tag == TokenTag::ScopeResolutionOperator)
    {
        const Token &start = Look();

        if (Look().tag == TokenTag::LeftParenthesis)
        {
            auto arguments = ParseArguments();
            x = GetExpressionFactory().Create<CallExpression>(Pos(start), x, arguments);
        }
        else if (Look().tag == TokenTag::ScopeResolutionOperator)
        {
            if (x->NodeType() == ExpressionType::Parameter)
            {
                auto parameter = static_cast<ParameterExpression *>(x);
                std::vector<std::u32string> qualifiedName = parameter->QualifiedName();
                while (Look().tag == TokenTag::ScopeResolutionOperator)
                {
                    Match(TokenTag::ScopeResolutionOperator);
                    qualifiedName.push_back(Match(TokenTag::Identifier).text);
                }
                x = GetExpressionFactory().Create<ParameterExpression>(Pos(start), qualifiedName, nullptr);
            }
            else
            {

                spdlog::error("Source code file: {}, line: {}, column: {}. Expecting 'Identifier' when parsing a "
                              "qualified name, got '{}'.",
                              document->FileName(), Look().line + 1, Look().column + 1,
                              Utility::EnumToString(Look().tag));

                throw ParserException(
                    __FILE__, __LINE__,
                    SourceRange(document, Look().line, Look().column, Look().line, Look().column + Look().text.size()),
                    Utility::UTF32ToUTF8(Format(U"Unexpected token type: '{}'. Expecting Identifier.",
                                                Utility::EnumToString(Look().tag))),
                    nullptr);
            }
        }
        else if (Look().tag == TokenTag::Dot)
        {
            Match(TokenTag::Dot);
            std::u32string fieldName = Match(TokenTag::Identifier).text;

            x = GetExpressionFactory().Create<MemberExpression>(Pos(start), x, fieldName);
        }
        else
        {
            /* TODO: '[' and '.' */
        }
    }
    return x;
}

ExpPtr Parser::ParseFactor()
{
    if (Look().tag == TokenTag::LeftParenthesis)
    {
        Advance();
        ExpPtr x = ParseOr();
        Match(TokenTag::RightParenthesis);
        return x;
    }
    else if (Look().tag == TokenTag::LeftBrace)
    {
        return ParseBlock();
    }
    else if (Look().tag == TokenTag::Integer)
    {
        std::u32string text = Look().text;
        const Token &start = Look();
        Advance();
        int32_t i = stoi(Utility::UTF32ToUTF8(text));
        return GetExpressionFactory().Create<ConstantExpression>(Pos(start), i, TypeCode::Int32);
    }
    else if (Look().tag == TokenTag::Float)
    {
        std::u32string text = Look().text;
        const Token &start = Look();
        Advance();
        double_t d = stod(Utility::UTF32ToUTF8(text));
        return GetExpressionFactory().Create<ConstantExpression>(Pos(start), d, TypeCode::Float64);
    }
    else if (Look().tag == TokenTag::Character)
    {
        std::u32string text = Look().text;
        const Token &start = Look();
        Advance();
        return GetExpressionFactory().Create<ConstantExpression>(Pos(start), text.front(), TypeCode::Char);
    }
    else if (Look().tag == TokenTag::String)
    {
        std::u32string text = Look().text;
        const Token &start = Look();
        Advance();
        return GetExpressionFactory().Create<ConstantExpression>(Pos(start), text, TypeCode::String);
    }
    else if (Look().tag == TokenTag::True)
    {
        const Token &start = Look();
        Advance();
        return GetExpressionFactory().Create<ConstantExpression>(Pos(start), true, TypeCode::Boolean);
    }
    else if (Look().tag == TokenTag::False)
    {
        const Token &start = Look();
        Advance();
        return GetExpressionFactory().Create<ConstantExpression>(Pos(start), false, TypeCode::Boolean);
    }
    else if (Look().tag == TokenTag::Identifier)
    {
        std::u32string name = Look().text;
        const Token &start = Look();
        Advance();
        return GetExpressionFactory().Create<ParameterExpression>(Pos(start), std::vector<std::u32string>{name},
                                                                  nullptr);
    }
    else if (Look().tag == TokenTag::New)
    {
        spdlog::info("Encounter object initialization expression.");
        return ParseNewExpression();
    }
    else
    {
        throw ParserException(
            __FILE__, __LINE__,
            SourceRange(document, Look().line, Look().column, Look().line, Look().column + Look().text.size()),
            Utility::UTF32ToUTF8(Format(U"Unexpected token type: '{}'.", Utility::EnumToString(Look().tag))), nullptr);
    }
}

ExpPtr Parser::ParseBlock()
{
    const Token &start = Look();
    Match(TokenTag::LeftBrace);
    vector<ExpPtr> expressions;
    while (!IsEof() && Look().tag != TokenTag::RightBrace)
    {
        expressions.push_back(Statement());
    }
    Match(TokenTag::RightBrace);
    return GetExpressionFactory().Create<BlockExpression>(Pos(start), expressions);
}

ExpPtr Parser::IfStatement()
{
    const Token &start = Look();
    Match(TokenTag::If);
    Match(TokenTag::LeftParenthesis);
    ExpPtr condition = ParseOr();
    Match(TokenTag::RightParenthesis);
    ExpPtr ifTrue = ParseBlock();
    if (Look().tag == TokenTag::Else)
    {
        Match(TokenTag::Else);
        if (Look().tag == TokenTag::If)
        {
            auto chunk = IfStatement();
            return GetExpressionFactory().Create<ConditionalExpression>(Pos(start), condition, ifTrue, chunk);
        }
        else
        {
            auto chunk = ParseBlock();
            return GetExpressionFactory().Create<ConditionalExpression>(Pos(start), condition, ifTrue, chunk);
        }
    }
    else
    {
        auto emptyRange = Pos(Look());
        auto *voidType = GetTypeSyntaxFactory().Create(emptyRange, std::vector<std::u32string>{U"Void"}, {});
        auto empty = GetExpressionFactory().Create<DefaultExpression>(emptyRange, voidType);
        return GetExpressionFactory().Create<ConditionalExpression>(Pos(start), condition, ifTrue, empty);
    }
}

ExpPtr Parser::WhileStatement()
{
    const Token &start = Look();
    Match(TokenTag::While);
    Match(TokenTag::LeftParenthesis);
    auto condition = ParseOr();
    Match(TokenTag::RightParenthesis);
    auto body = ParseBlock();
    return GetExpressionFactory().Create<WhileLoopExpression>(Pos(start), condition, body);
}

Expressions::VariableDeclarationExpression *Parser::VariableDeclarationStatement()
{
    const Token &start = Look();
    Match(TokenTag::Var);
    std::u32string name = Match(TokenTag::Identifier).text;
    TypeSyntax *typeSyntax = nullptr;
    if (Look().tag == TokenTag::Colon)
    {
        Match(TokenTag::Colon);
        typeSyntax = ParseType();
    }
    Match(TokenTag::Assign);
    auto initializer = ParseOr();

    return GetExpressionFactory().Create<VariableDeclarationExpression>(Pos(start), name, typeSyntax, initializer);
}

Expressions::LambdaExpression *Parser::FunctionDeclarationStatement(const std::vector<Annotation> &annotations)
{
    const Token &start = Look();
    Match(TokenTag::Func);
    std::u32string name = Match(TokenTag::Identifier).text;
    Match(TokenTag::LeftParenthesis);

    bool isFirstParameter = true;
    std::vector<ParameterExpression *> parameters;
    while (Look().tag != TokenTag::RightParenthesis)
    {
        if (isFirstParameter)
        {
            parameters.push_back(ParseParameter());
            isFirstParameter = false;
        }
        else
        {
            Match(TokenTag::Comma);
            parameters.push_back(ParseParameter());
        }
    }
    Match(TokenTag::RightParenthesis);
    Match(TokenTag::Colon);
    TypeSyntaxPtr returnType = ParseType();
    if (Look().tag == TokenTag::Semicolon)
    {
        ExpPtr body = GetExpressionFactory().Create<DefaultExpression>(Pos(Look()), returnType);
        Match(TokenTag::Semicolon);

        return GetExpressionFactory().Create<LambdaExpression>(Pos(start), name, body, parameters, returnType,
                                                               annotations);
    }
    else
    {
        ExpPtr body = ParseBlock();

        return GetExpressionFactory().Create<LambdaExpression>(Pos(start), name, body, parameters, returnType,
                                                               annotations);
    }
}

Expressions::VariableDeclarationExpression *Parser::ParseGlobalVariable()
{
    const Token &start = Look();
    Match(TokenTag::Var);
    std::u32string name = Match(TokenTag::Identifier).text;
    TypeSyntax *typeSyntax = nullptr;
    if (Look().tag == TokenTag::Colon)
    {
        Match(TokenTag::Colon);
        typeSyntax = ParseType();
    }
    else
    {
        spdlog::error("Type annotation is required for declaring a global variable.");
        throw ParserException(
            __FILE__, __LINE__,
            SourceRange(document, Look().line, Look().column, Look().line, Look().column + Look().text.size()),
            "Type annotation is required for declaring a global variable.", nullptr);
    }
    Match(TokenTag::Assign);
    auto initializer = ParseOr();
    Match(TokenTag::Semicolon);

    return GetExpressionFactory().Create<VariableDeclarationExpression>(Pos(start), name, typeSyntax, initializer);
}

Expressions::StructureExpression *Parser::ParseStructureDefinition()
{
    const Token &start = Look();
    Match(TokenTag::Structure);
    std::u32string name = Match(TokenTag::Identifier).text;
    Match(TokenTag::LeftBrace);

    Utility::OrderPreservingMap<std::u32string, TypeSyntax *> fields;
    while (Look().tag != TokenTag::RightBrace)
    {
        std::u32string name = Match(TokenTag::Identifier).text;
        Match(TokenTag::Colon);
        TypeSyntax *typeSyntax = ParseType();
        fields.AddItem(name, typeSyntax);
        Match(TokenTag::Semicolon);
    }
    Match(TokenTag::RightBrace);

    Namespace *top = namespaceStack.top();
    std::vector<std::u32string> path = top->GetFullQualifiedName();
    std::vector<std::u32string> qualifiedName;
    for (size_t i = 1; i < path.size(); i++)
    {
        qualifiedName.push_back(path.at(i));
    }
    qualifiedName.push_back(name);
    return GetExpressionFactory().Create<StructureExpression>(Pos(start), qualifiedName, fields);
}

std::vector<ExpPtr> Parser::ParseArguments()
{
    vector<ExpPtr> arguments;
    Match(TokenTag::LeftParenthesis);
    if (Look().tag == TokenTag::RightParenthesis)
    {
        Match(TokenTag::RightParenthesis);
    }
    else
    {
        arguments.push_back(ParseArgument());
        while (!IsEof() && Look().tag != TokenTag::RightParenthesis)
        {
            Match(TokenTag::Comma);
            arguments.push_back(ParseArgument());
        }
        Match(TokenTag::RightParenthesis);
    }

    return arguments;
}

ExpPtr Parser::ParseArgument()
{
    ExpPtr value = ParseOr();
    return value;
}

Expressions::ParameterExpression *Parser::ParseParameter()
{
    const Token &start = Look();
    std::u32string name = Match(TokenTag::Identifier).text;
    Match(TokenTag::Colon);
    TypeSyntaxPtr type = ParseType();

    return GetExpressionFactory().Create<ParameterExpression>(Pos(start), std::vector<std::u32string>{name}, type);
}

TypeSyntaxPtr Parser::ParseType()
{
    const Token &start = Look();
    std::u32string name = Match(TokenTag::Identifier).text;
    std::vector<std::u32string> qualifiedName{name};
    while (Look().tag == TokenTag::ScopeResolutionOperator)
    {
        Match(TokenTag::ScopeResolutionOperator);
        qualifiedName.push_back(Match(TokenTag::Identifier).text);
    }
    if (Look().tag == TokenTag::LeftBracket)
    {
        auto arguments = ParseTypeArguments();

        return GetTypeSyntaxFactory().Create(Pos(start), qualifiedName, arguments);
    }
    else
    {
        return GetTypeSyntaxFactory().Create(Pos(start), qualifiedName, {});
    }
}

std::vector<TypeSyntax *> Parser::ParseTypeArguments()
{
    Match(TokenTag::LeftBracket);
    std::vector<TypeSyntax *> arguments;
    arguments.push_back(ParseType());
    while (Look().tag != TokenTag::RightBracket)
    {
        Match(TokenTag::Comma);
        arguments.push_back(ParseType());
    }
    Match(TokenTag::RightBracket);

    return arguments;
}

void Parser::ParseNamespace()
{
    Namespace *top = namespaceStack.top();

    while (Look().tag == TokenTag::Module)
    {
        Match(TokenTag::Module);
        std::u32string name = Match(TokenTag::Identifier).text;
        Match(TokenTag::LeftBrace);

        GetNamespaceFactory().Insert(top, {name});
        Namespace *current = GetNamespaceFactory().Search(top, {name});
        namespaceStack.push(current);

        while (Look().tag != TokenTag::RightBrace)
        {
            switch (Look().tag)
            {
            case TokenTag::Var: {
                VariableDeclarationExpression *varDecl = ParseGlobalVariable();
                current->GlobalVariables().AddItem(varDecl->Name(), varDecl);
                break;
            }
            case TokenTag::Func: {
                LambdaExpression *lambda = FunctionDeclarationStatement({});
                current->Functions().AddItem(lambda->Name(), lambda);
                break;
            }
            case TokenTag::Structure: {
                StructureExpression *structureDefinition = ParseStructureDefinition();
                current->Structures().AddItem(structureDefinition->QualifiedName().back(), structureDefinition);

                break;
            }
            case TokenTag::Module: {
                ParseNamespace();
                break;
            }
            case TokenTag::At: {
                std::vector<Annotation> annotations = ParseAnnotations();
                if (Look().tag == TokenTag::Func)
                {
                    LambdaExpression *lambda = FunctionDeclarationStatement(annotations);
                    current->Functions().AddItem(lambda->Name(), lambda);
                }
                else
                {
                    throw ParserException(__FILE__, __LINE__,
                                          SourceRange(document, Look().line, Look().column, Look().line,
                                                      Look().column + Look().text.size()),
                                          "Expecting a function definition.", nullptr);
                }
                break;
            }
            default: {
                throw ParserException(
                    __FILE__, __LINE__,
                    SourceRange(document, Look().line, Look().column, Look().line, Look().column + Look().text.size()),
                    "Unexpected token '" + Utility::EnumToString(Look().tag) +
                        "' encountered while parsing the module. Expected 'var' or 'func'.",
                    nullptr);
            }
            }
        }
        Match(TokenTag::RightBrace);

        namespaceStack.pop();
    }
}

AnnotationArgument Parser::ParseAnnotationArgument()
{
    std::u32string name = Match(TokenTag::Identifier).text;
    Match(TokenTag::Assign);
    /* TODO: support other compile-time constants. */
    std::u32string value = Match(TokenTag::String).text;

    return AnnotationArgument(name, value);
}

Annotation Parser::ParseAnnotation()
{
    Match(TokenTag::At);
    std::u32string name = Match(TokenTag::Identifier).text;
    Match(TokenTag::LeftParenthesis);
    std::vector<AnnotationArgument> arguments;
    bool isFirstArgument = true;
    while (Look().tag != TokenTag::RightParenthesis)
    {
        if (isFirstArgument)
        {
            arguments.push_back(ParseAnnotationArgument());
            isFirstArgument = false;
        }
        else
        {
            Match(TokenTag::Comma);
            arguments.push_back(ParseAnnotationArgument());
        }
    }
    Match(TokenTag::RightParenthesis);

    return Annotation(name, arguments);
}

std::vector<Annotation> Parser::ParseAnnotations()
{
    std::vector<Annotation> annotations;
    while (Look().tag == TokenTag::At)
    {
        annotations.push_back(ParseAnnotation());
    }

    return annotations;
}

Expressions::NewExpression *Parser::ParseNewExpression()
{
    const Token &start = Look();
    Match(TokenTag::New);
    std::vector<std::u32string> namespacePath = ParseNamespacePath();
    Utility::OrderPreservingMap<std::u32string, Expressions::Expression *> fieldsInitialization;
    Match(TokenTag::LeftBrace);
    while (Look().tag != TokenTag::RightBrace)
    {
        std::u32string fieldName = Match(TokenTag::Identifier).text;
        Match(TokenTag::Assign);
        Expressions::Expression *value = ParseOr();
        Match(TokenTag::Semicolon);
        fieldsInitialization.AddItem(fieldName, value);
    }
    Match(TokenTag::RightBrace);
    TypeSyntax *typeSyntax = GetTypeSyntaxFactory().Create(Pos(start), namespacePath, {});
    NewExpression *newExpression =
        GetExpressionFactory().Create<NewExpression>(Pos(start), typeSyntax, fieldsInitialization);
    spdlog::info("Completing creating the object initialization expression.");

    return newExpression;
}

std::vector<std::u32string> Parser::ParseNamespacePath()
{
    std::vector<std::u32string> path;
    path.push_back(Match(TokenTag::Identifier).text);
    while (Look().tag == TokenTag::ScopeResolutionOperator)
    {
        Match(TokenTag::ScopeResolutionOperator);
        path.push_back(Match(TokenTag::Identifier).text);
    }

    return path;
}

}; /* namespace SyntaxAnalysis */
}; /* namespace Cygni */