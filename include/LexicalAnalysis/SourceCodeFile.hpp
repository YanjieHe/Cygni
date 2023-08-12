#ifndef CYGNI_LEXICAL_ANALYSIS_SOURCE_CODE_FILE_HPP
#define CYGNI_LEXICAL_ANALYSIS_SOURCE_CODE_FILE_HPP

#include <string>

namespace Cygni {
namespace LexicalAnalysis {

class SourceCodeFile {
 public:
  std::string filePath;

  SourceCodeFile() : filePath() {}
  SourceCodeFile(const std::string& filePath) : filePath{filePath} {}
};

}; /* namespace LexicalAnalysis */
}; /* namespace Cygni */

#endif /* CYGNI_LEXICAL_ANALYSIS_SOURCE_CODE_FILE_HPP */