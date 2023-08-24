#ifndef CYGNI_EXPRESSIONS_PARSER_HPP
#define CYGNI_EXPRESSIONS_PARSER_HPP

#include "Expressions/TreeException.hpp"
#include "Expressions/Expression.hpp"
#include "Expressions/SourceRange.hpp"
#include "LexicalAnalysis/Token.hpp"

namespace Cygni {
namespace SyntaxAnalysis {

using LexicalAnalysis::Token;

using ExpPtr = Expressions::Expression *;
using TypePtr = Expressions::Type*;

class Parser {
private:
  std::vector<Token> tokens;
  std::shared_ptr<LexicalAnalysis::SourceCodeFile> document;
  int offset;
  Expressions::ExpressionFactory expressionFactory;
  Expressions::TypeFactory typeFactory;

public:
  Parser(std::vector<Token> tokens,
         std::shared_ptr<LexicalAnalysis::SourceCodeFile> document);

  inline bool IsEof() const {
    return Look().tag == LexicalAnalysis::TokenTag::Eof;
  }

  inline const Token &Look() const { return tokens[offset]; }

  inline void Advance() { offset++; }

  inline void Back() { offset--; }

  const Token &Match(LexicalAnalysis::TokenTag tag);

  inline Expressions::SourceRange Pos(const Token &token) const {
    return Expressions::SourceRange{document, token.line, Look().line,
                                    token.column, Look().column};
  }

  ExpPtr Statement();

  ExpPtr ParseAssign();

  ExpPtr ParseOr();

  ExpPtr ParseAnd();

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

  std::vector<ExpPtr> ParseArguments();

  ExpPtr ParseArgument();
};

}; /* namespace SyntaxAnalysis */
}; /* namespace Cygni */

#endif /* CYGNI_EXPRESSIONS_PARSER_HPP */