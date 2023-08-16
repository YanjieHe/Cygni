#ifndef CYGNI_LEXICAL_ANALYSIS_LEXER_HPP
#define CYGNI_LEXICAL_ANALYSIS_LEXER_HPP

#include <memory>
#include <string>
#include <vector>

#include "LexicalAnalysis/LexicalException.hpp"
#include "LexicalAnalysis/SourceCodeFile.hpp"
#include "LexicalAnalysis/Token.hpp"
#include "Utility/Format.hpp"
#include "Utility/UTF32Functions.hpp"


namespace Cygni {
namespace LexicalAnalysis {

using Utility::IsDigit;
using Utility::IsLetter;
using Utility::IsWhiteSpace;

class Lexer {
 private:
  std::shared_ptr<SourceCodeFile> sourceCodeFile;
  std::u32string code;
  int line;
  int column;
  int offset;
  std::u32string builder;

  static inline const char32_t SINGLE_QUOTE = U'\'';
  static inline const char32_t DOUBLE_QUOTE = U'\"';
  static inline const char32_t END_LINE = U'\n';
  static inline const char32_t BACKSLASH = U'\\';

 public:
  Lexer(std::shared_ptr<SourceCodeFile> sourceCodeFile,
        const std::u32string& code);

  std::vector<Token> ReadAll();

 private:
  Token ReadInt();

  Token ReadFloat();

  Token ReadExponent();

  void ReadDecimalDigits();

  Token ReadCharacterLiteral();

  void ReadCharacter();

  void ReadSimpleEscapeSequence();

  void ReadHexadecimalEscapeSequence();

  void ReadUnicodeEscapeSequence();

  Token ReadAnnotation();

  bool IsHexDigit();

  Token ReadString();

  char32_t UnescapedChar(char32_t c);

  Token ReadIdentifier();

  Token ReadOperator();

  void SkipWhitespaces();

  void SkipComment(std::vector<Token>& tokens);

  void SkipSingleLineComment();

  inline static bool IsIdentifierChar(char32_t c) {
    return IsLetter(c) || IsDigit(c) || c == U'_';
  }

  inline bool IsEof() const {
    return offset >= static_cast<int32_t>(code.size());
  }

  void Forward() {
    if (code.at(offset) == END_LINE) {
      offset++;
      line++;
      column = 1;
    } else {
      offset++;
      column++;
    }
  }

  inline char32_t Peek() const { return code.at(offset); }

  void Consume() {
    builder.push_back(Peek());
    Forward();
  }

  void Reset() { builder.clear(); }

  inline void Match(char32_t c) {
    if (Peek() == c) {
      Consume();
    } else {
      throw LexicalException(sourceCodeFile, line, column,
                             Format(U"unexpected character '{}'", Peek()));
    }
  }

  inline void Match(char32_t c1, char32_t c2) {
    if (Peek() == c1 || Peek() == c2) {
      Consume();
    } else {
      throw LexicalException(sourceCodeFile, line, column,
                             Format(U"unexpected character '{}'", Peek()));
    }
  }

  inline void MatchAndSkip(char32_t c) {
    if (Peek() == c) {
      Forward();
    } else {
      throw LexicalException(sourceCodeFile, line, column,
                             Format(U"unexpected character '{}'", Peek()));
    }
  }
};

}; /* namespace LexicalAnalysis */
}; /* namespace Cygni */

#endif /* CYGNI_LEXICAL_ANALYSIS_LEXER_HPP */