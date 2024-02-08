#ifndef CYGNI_UTILITY_HASH_PAIR_HPP
#define CYGNI_UTILITY_HASH_PAIR_HPP

#include <utility>
#include <functional>

namespace Cygni {
namespace Utility {

struct HashPair {
  template <class T1, class T2>
  size_t operator()(const std::pair<T1, T2> &p) const {
    size_t hash1 = std::hash<T1>{}(p.first);
    size_t hash2 = std::hash<T2>{}(p.second);

    if (hash1 != hash2) {
      return hash1 ^ hash2;
    } else {
      return hash1;
    }
  }
};

}; /* namespace Utility */
}; /* namespace Cygni */

#endif /* CYGNI_UTILITY_HASH_PAIR_HPP */