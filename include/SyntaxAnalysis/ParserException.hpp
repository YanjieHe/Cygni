#ifndef CYGNI_SYNTAX_ANALYSIS_PARSER_EXCEPTION_HPP
#define CYGNI_SYNTAX_ANALYSIS_PARSER_EXCEPTION_HPP

#include "Utility/Exception.hpp"
#include "Expressions/SourceRange.hpp"

namespace Cygni {
namespace SyntaxAnalysis {

class ParserException : public Utility::Exception {
private:
  Expressions::SourceRange sourceRange;

public:
  ParserException(std::string source, int line,
                  Expressions::SourceRange sourceRange,
                  const std::string &message,
                  const std::exception *innerException)
      : Exception(source, line, message, innerException),
        sourceRange{sourceRange} {}

  Expressions::SourceRange GetSourceRange() { return sourceRange; }
};

}; /* namespace SyntaxAnalysis */
}; /* namespace Cygni */

#endif /* CYGNI_SYNTAX_ANALYSIS_PARSER_EXCEPTION_HPP */