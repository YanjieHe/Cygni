#ifndef CYGNI_EXPRESSIONS_TREE_EXCEPTION_HPP
#define CYGNI_EXPRESSIONS_TREE_EXCEPTION_HPP

#include "Expressions/Expression.hpp"
#include "Utility/Exception.hpp"

namespace Cygni
{
namespace Expressions
{

class TreeException : public Utility::Exception
{
  private:
    SourceRange sourceRange;

  public:
    TreeException(std::string source, int line, std::string message, const Expression *tree,
                  const std::exception *innerException)
        : Exception(source, line, message, innerException),
          sourceRange{tree ? tree->GetSourceRange() : SourceRange(nullptr, 0, 0, 0, 0)}
    {
    }

    const SourceRange &GetSourceRange() const
    {
        return sourceRange;
    }
};

}; /* namespace Expressions */
}; /* namespace Cygni */

#endif /* CYGNI_EXPRESSIONS_TREE_EXCEPTION_HPP */