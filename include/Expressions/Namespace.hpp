#ifndef CYGNI_EXPRESSIONS_NAMESPACE_HPP
#define CYGNI_EXPRESSIONS_NAMESPACE_HPP

#include "Expressions/Expression.hpp"
#include "Utility/OrderPreservingMap.hpp"

namespace Cygni {
namespace Expressions {

class Namespace;

class Namespace {
private:
  Namespace *parent;
  std::u32string name;
  Utility::OrderPreservingMap<std::u32string, Namespace *> children;
  Utility::OrderPreservingMap<std::u32string, VariableDeclarationExpression *>
      globalVariables;
  Utility::OrderPreservingMap<std::u32string, LambdaExpression *> functions;

public:
  Namespace(Namespace *parent, std::u32string name)
      : parent{parent}, name{name} {}

  const Namespace *Parent() { return parent; }
  const std::u32string &Name() const { return name; }
  Utility::OrderPreservingMap<std::u32string, Namespace *> &Children() {
    return children;
  }
  Utility::OrderPreservingMap<std::u32string, VariableDeclarationExpression *> &
  GlobalVariables() {
    return globalVariables;
  }
  Utility::OrderPreservingMap<std::u32string, LambdaExpression *> &Functions() {
    return functions;
  }
};

class NamespaceFactory {
private:
  std::vector<Namespace *> namespaces;
  Namespace *root;

public:
  NamespaceFactory() : root{nullptr} {}
  NamespaceFactory(const NamespaceFactory &) = delete;
  ~NamespaceFactory() {
    for (auto ns : namespaces) {
      delete ns;
    }
  }

  Namespace *GetRoot() {
    if (root) {
      return root;
    } else {
      root = Create(nullptr, U"");

      return root;
    }
  }

  Namespace *Create(Namespace *parent, const std::u32string &name) {
    auto ns = new Namespace(parent, name);
    namespaces.push_back(ns);
    return ns;
  }

  void Insert(Namespace *predecessor, const std::vector<std::u32string> &path);
  Namespace *Search(Namespace *predecessor,
                    const std::vector<std::u32string> &path);
};

}; /* namespace Expressions */
}; /* namespace Cygni */

#endif /* CYGNI_EXPRESSIONS_NAMESPACE_HPP */