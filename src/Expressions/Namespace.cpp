#include "Expressions/Namespace.hpp"

namespace Cygni {
namespace Expressions {

void NamespaceFactory::Insert(Namespace *root,
                              const std::vector<std::u32string> &path) {

  for (const std::u32string &name : path) {
    if (root->Children().count(name)) {
      root = root->Children().at(name);
    } else {
      Namespace *ns = Create(root, name);
      root->Children().insert({name, ns});
      root = ns;
    }
  }
}

Namespace* NamespaceFactory::Search(Namespace *root,
                              const std::vector<std::u32string> &path) {
  for (const std::u32string &name : path) {
    if (root->Children().count(name)) {
      root = root->Children().at(name);
    } else {

        return nullptr;
    }
  }

  return root;
}

}; /* namespace Expressions */
}; /* namespace Cygni */