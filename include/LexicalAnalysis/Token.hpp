#ifndef CYGNI_LEXICAL_ANALYSIS_TOKEN_HPP
#define CYGNI_LEXICAL_ANALYSIS_TOKEN_HPP

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

#include "SourceCodeFile.hpp"


namespace Cygni {
namespace LexicalAnalysis {

enum class TokenTag {
  Identifier,
  Integer,
  Float,
  Character,
  String,

  Add,
  Subtract,
  Multiply,
  Divide,
  Modulo,

  Equal,
  NotEqual,
  GreaterThan,
  GreaterThanOrEqual,
  LessThan,
  LessThanOrEqual,

  And,
  Or,
  Not,

  Var,
  Func,
  Assign,

  LeftBrace,
  RightBrace,
  LeftBracket,
  RightBracket,
  LeftParenthesis,
  RightParenthesis,

  Dot,
  Comma,
  Semicolon,
  Colon,
  At,

  True,
  False,

  If,
  Else,
  While,

  For,
  Break,
  Return,

  Import,
  Uses,
  Require,
  Rename,
  GoesTo,

  Structure,
  Module,
  Package,
  Interface,
  To,
  ScopeResolutionOperator,
  Eof
};

using Json = nlohmann::json;

class Token {
 public:
  std::shared_ptr<SourceCodeFile> sourceCodeFile;
  int line;
  int column;
  TokenTag tag;
  std::u32string text;

  Token() : sourceCodeFile(), line{0}, column{0}, tag{TokenTag::Eof}, text() {}
  Token(std::shared_ptr<SourceCodeFile> sourceCodeFile, int line, int column,
        TokenTag tag, const std::u32string& text)
      : sourceCodeFile{sourceCodeFile},
        line{line},
        column{column},
        tag{tag},
        text{text} {}

  static TokenTag IdentifyKeyword(TokenTag tag, const std::u32string& text);
  static std::unordered_map<std::u32string, TokenTag> keywords;
  static std::unordered_map<std::u32string, TokenTag> operators;
  Json ToJson() const;
};

}; /* namespace LexicalAnalysis */
}; /* namespace Cygni */

#endif /* CYGNI_LEXICAL_ANALYSIS_TOKEN_HPP */