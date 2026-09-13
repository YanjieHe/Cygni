#ifndef CYGNI_EXPRESSIONS_PARSER_HPP
#define CYGNI_EXPRESSIONS_PARSER_HPP

#include "Compilation/CompilationContext.hpp"
#include "Expressions/Expression.hpp"
#include "Expressions/Namespace.hpp"
#include "Expressions/SourceRange.hpp"
#include "Expressions/TreeException.hpp"
#include "LexicalAnalysis/Token.hpp"
#include <stack>

namespace Cygni
{
namespace SyntaxAnalysis
{

using LexicalAnalysis::Token;
using namespace Cygni::Expressions;

using ExpPtr = Expressions::Expression *;
using TypeSyntaxPtr = Expressions::TypeSyntax *;

enum class FunctionParseKind
{
    InterfaceMethod,
    StructureMethod,
    ModuleFunction
};

class Parser
{
  private:
    std::vector<Token> tokens;
    std::shared_ptr<LexicalAnalysis::SourceCodeFile> document;
    int offset;
    Compilation::CompilationContext &compilationContext;
    std::stack<Expressions::Namespace *> namespaceStack;

  public:
    Parser(std::vector<Token> tokens, std::shared_ptr<LexicalAnalysis::SourceCodeFile> document,
           Compilation::CompilationContext &compilationContext);

    inline bool IsEof() const
    {
        return Look().tag == LexicalAnalysis::TokenTag::Eof;
    }

    inline const Token &Look() const
    {
        return tokens[offset];
    }

    inline void Advance()
    {
        offset++;
    }

    inline void Back()
    {
        offset--;
    }

    const Token &Match(LexicalAnalysis::TokenTag tag);

    inline Expressions::SourceRange Pos(const Token &token) const
    {
        return Expressions::SourceRange{document, token.line, Look().line, token.column, Look().column};
    }

    NamespaceFactory &GetNamespaceFactory()
    {
        return compilationContext.GetNamespaceFactory();
    }

    ExpressionFactory &GetExpressionFactory()
    {
        return compilationContext.GetExpressionFactory();
    }

    TypeSyntaxFactory &GetTypeSyntaxFactory()
    {
        return compilationContext.GetTypeSyntaxFactory();
    }

    ExpPtr Statement();

    ExpPtr ParseAssign();

    ExpPtr ParseOr();

    ExpPtr ParseAnd();

    ExpPtr ParseNot();

    ExpPtr ParseEquality();

    ExpPtr ParseRelation();

    ExpPtr ParseExpr();

    ExpPtr ParseTerm();

    ExpPtr ParseUnary();

    ExpPtr ParsePostfix();

    ExpPtr ParseFactor();

    ExpPtr ParseBlock();

    ExpPtr IfStatement();

    ExpPtr WhileStatement();

    Expressions::VariableDeclarationExpression *VariableDeclarationStatement();

    Expressions::LambdaExpression *FunctionDeclarationStatement(const std::vector<Annotation> &annotations,
                                                                FunctionParseKind kind);

    Expressions::VariableDeclarationExpression *ParseGlobalVariable();

    Expressions::StructureExpression *ParseStructureDefinition();

    Expressions::InterfaceExpression *ParseInterfaceDefinition();

    std::vector<ExpPtr> ParseArguments();

    ExpPtr ParseArgument();

    Expressions::ParameterExpression *ParseParameter();

    TypeSyntaxPtr ParseType();

    std::vector<TypeSyntax *> ParseTypeArguments();

    void ParseNamespace();

    AnnotationArgument ParseAnnotationArgument();

    Annotation ParseAnnotation();

    std::vector<Annotation> ParseAnnotations();

    Expressions::NewExpression *ParseNewExpression();

    std::vector<std::u32string> ParseNamespacePath();

  private:
    bool IsNativeFunction(const std::vector<Annotation> &annotations);
    bool RequiresDeclaration(FunctionParseKind kind, bool isNative, std::string &errorMessage);
};

}; /* namespace SyntaxAnalysis */
}; /* namespace Cygni */

#endif /* CYGNI_EXPRESSIONS_PARSER_HPP */