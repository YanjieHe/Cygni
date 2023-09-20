#ifndef CYGNI_LEXICAL_ANALYSIS_SOURCE_CODE_FILE_HPP
#define CYGNI_LEXICAL_ANALYSIS_SOURCE_CODE_FILE_HPP

#include <string>

namespace Cygni {
namespace LexicalAnalysis {

class SourceCodeFile {
private:
  std::string fileName;

public:
  SourceCodeFile() : fileName() {}
  SourceCodeFile(const std::string &filePath) : fileName{filePath} {}

  const std::string &FileName() const { return fileName; }
};

}; /* namespace LexicalAnalysis */
}; /* namespace Cygni */

#endif /* CYGNI_LEXICAL_ANALYSIS_SOURCE_CODE_FILE_HPP */