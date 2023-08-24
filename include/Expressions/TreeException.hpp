#ifndef CYGNI_EXPRESSIONS_TREE_EXCEPTION_HPP
#define CYGNI_EXPRESSIONS_TREE_EXCEPTION_HPP

#include "Utility/Exception.hpp"
#include "Expressions/Expression.hpp"

namespace Cygni {
namespace Expressions {

class TreeException : public Utility::Exception {
private:
  const Expression *tree;

public:
  TreeException(std::string source, int line, std::string message,
                const Expression *tree, const std::exception *innerException)
      : Exception(source, line, message, innerException), tree{tree} {}

  const Expression *Tree() const { return tree; }
};

}; /* namespace Expressions */
}; /* namespace Cygni */

#endif /* CYGNI_EXPRESSIONS_TREE_EXCEPTION_HPP */