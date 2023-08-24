#ifndef CYGNI_EXPRESSIONS_NAMESPACE_HPP
#define CYGNI_EXPRESSIONS_NAMESPACE_HPP

#include "Expressions/Expression.hpp"

namespace Cygni {
namespace Expressions {

class Namespace;

class Namespace {
private:
  Namespace *parent;
  std::u32string name;
  std::unordered_map<std::u32string, Namespace *> children;
  std::unordered_map<std::u32string, ParameterExpression *> globalVariables;
  std::unordered_map<std::u32string, LambdaExpression *> functions;

public:
  Namespace(Namespace *parent, std::u32string name) : parent{parent}, name{name} {}

  const Namespace *Parent() { return parent; }
  const std::u32string &Name() const { return name; }
  std::unordered_map<std::u32string, Namespace *> &Children() { return children; }
  std::unordered_map<std::u32string, ParameterExpression *> &GlobalVariables() {
    return globalVariables;
  }
  std::unordered_map<std::u32string, LambdaExpression *> &Functions() {
    return functions;
  }
};

class NamespaceFactory {
private:
  std::vector<Namespace *> namespaces;

public:
  NamespaceFactory() = default;
  ~NamespaceFactory() {
    for (auto ns : namespaces) {
      delete ns;
    }
  }

  Namespace *Create(Namespace *parent, const std::u32string &name) {
    auto ns = new Namespace(parent, name);
    namespaces.push_back(ns);
    return ns;
  }

  void Insert(Namespace* root, const std::vector<std::u32string>& path);
  Namespace* Search(Namespace* root, const std::vector<std::u32string>& path);
};

}; /* namespace Expressions */
}; /* namespace Cygni */

#endif /* CYGNI_EXPRESSIONS_NAMESPACE_HPP */