#ifndef CYGNI_VISITORS_COMPILATION_EXCEPTION_HPP
#define CYGNI_VISITORS_COMPILATION_EXCEPTION_HPP

#include "Expressions/Expression.hpp"
#include "Utility/Exception.hpp"


namespace Cygni
{
namespace Visitors
{

class CompilationException : public Utility::Exception
{
  private:
    const Expressions::Expression *tree;

  public:
    CompilationException(std::string source, int line, std::string message, const Expressions::Expression *tree,
                         const std::exception *innerException)
        : Exception(source, line, message, innerException), tree{tree}
    {
    }

    const Expressions::Expression *Tree() const
    {
        return tree;
    }
};

}; /* namespace Visitors */
}; /* namespace Cygni */

#endif /* CYGNI_VISITORS_COMPILATION_EXCEPTION_HPP */