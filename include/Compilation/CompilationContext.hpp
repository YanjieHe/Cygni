#ifndef CYGNI_COMPILATION_COMPILATION_CONTEXT_HPP
#define CYGNI_COMPILATION_COMPILATION_CONTEXT_HPP
#include "Expressions/Expression.hpp"
#include "Expressions/Namespace.hpp"

namespace Cygni
{
namespace Compilation
{
class CompilationContext
{
  private:
    Expressions::ExpressionFactory expressionFactory;
    Expressions::NamespaceFactory namespaceFactory;
    Expressions::TypeSyntaxFactory typeSyntaxFactory;

  public:
    CompilationContext() : expressionFactory{}, namespaceFactory{}, typeSyntaxFactory{}
    {
    }
    CompilationContext(const CompilationContext &) = delete;
    CompilationContext &operator=(const CompilationContext &) = delete;
    CompilationContext(CompilationContext &&) = delete;
    CompilationContext &operator=(CompilationContext &&) = delete;

    Expressions::ExpressionFactory &GetExpressionFactory()
    {
        return expressionFactory;
    };
    Expressions::NamespaceFactory &GetNamespaceFactory()
    {
        return namespaceFactory;
    }
    Expressions::TypeSyntaxFactory &GetTypeSyntaxFactory()
    {
        return typeSyntaxFactory;
    }
};
}; /* namespace Compilation */
}; /* namespace Cygni */

#endif /* CYGNI_COMPILATION_COMPILATION_CONTEXT_HPP */