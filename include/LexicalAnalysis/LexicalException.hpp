#ifndef CYGNI_LEXICAL_ANALYSIS_LEXICAL_EXCEPTION_HPP
#define CYGNI_LEXICAL_ANALYSIS_LEXICAL_EXCEPTION_HPP

#include <memory>
#include <string>

#include "LexicalAnalysis/SourceCodeFile.hpp"
#include "Utility/Format.hpp"
#include "Utility/UTF32Functions.hpp"

namespace Cygni {
namespace LexicalAnalysis {

using Utility::Format;

class LexicalException {
 public:
  std::shared_ptr<SourceCodeFile> sourceCodeFile;
  int line;
  int column;
  std::u32string message;

  LexicalException(std::shared_ptr<SourceCodeFile> sourceCodeFile, int line,
                   int column, const std::u32string &message)
      : sourceCodeFile{sourceCodeFile},
        line{line},
        column{column},
        message{message} {}

  std::u32string FormattedErrorMessage() const {
    auto actualLine = std::to_string(line + 1);
    auto actualColumn = std::to_string(column + 1);
    return Format(U"File: {} Syntax Error: ({}, {}) {} ",
                  sourceCodeFile->filePath, actualLine, actualColumn, message);
  }
};

}; /* namespace LexicalAnalysis */
}; /* namespace Cygni */

#endif /* CYGNI_LEXICAL_ANALYSIS_LEXICAL_EXCEPTION_HPP */