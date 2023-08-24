#ifndef CYGNI_EXPRESSIONS_SOURCE_RANGE_HPP
#define CYGNI_EXPRESSIONS_SOURCE_RANGE_HPP

#include <string>
#include <memory>

#include "LexicalAnalysis/SourceCodeFile.hpp"

namespace Cygni {
namespace Expressions {

class SourceRange {
private:
  std::shared_ptr<LexicalAnalysis::SourceCodeFile> codeFile;
  int startLine;
  int endLine;
  int startColumn;
  int endColumn;

public:
  SourceRange(std::shared_ptr<LexicalAnalysis::SourceCodeFile> codeFile, int startLine, int endLine,
              int startColumn, int endColumn)
      : codeFile{codeFile}, startLine{startLine}, endLine{endLine},
        startColumn{startColumn}, endColumn{endColumn} {}

  const std::shared_ptr<LexicalAnalysis::SourceCodeFile> CodeFile() const { return codeFile; }
  const int StartLine() const { return startLine; }
  const int EndLine() const { return endLine; }
  const int StartColumn() const { return startColumn; }
  const int EndColumn() const { return endColumn; }
};

}; /* namespace Cygni */
}; /* namespace Expressions */

#endif /* CYGNI_EXPRESSIONS_SOURCE_RANGE_HPP */