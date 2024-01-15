#include "Expressions/Namespace.hpp"

namespace Cygni {
namespace Expressions {

void NamespaceFactory::Insert(Namespace *predecessor,
                              const std::vector<std::u32string> &path) {

  for (const std::u32string &name : path) {
    if (predecessor->Children().ContainsKey(name)) {
      predecessor = predecessor->Children().GetItemByKey(name);
    } else {
      Namespace *ns = Create(predecessor, name);
      predecessor->Children().AddItem(name, ns);
      predecessor = ns;
    }
  }
}

Namespace* NamespaceFactory::Search(Namespace *predecessor,
                              const std::vector<std::u32string> &path) {
  for (const std::u32string &name : path) {
    if (predecessor->Children().ContainsKey(name)) {
      predecessor = predecessor->Children().GetItemByKey(name);
    } else {

        return nullptr;
    }
  }

  return predecessor;
}

}; /* namespace Expressions */
}; /* namespace Cygni */