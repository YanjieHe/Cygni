#ifndef CYGNI_ORDER_PRESERVING_MAP_HPP
#define CYGNI_ORDER_PRESERVING_MAP_HPP

#include <unordered_map>
#include <vector>
#include <string>
#include <stdexcept>

namespace Cygni {
namespace Utility {

template <typename TKey, typename TValue> class OrderPreservingMap {
private:
  std::unordered_map<TKey, size_t> indexMap;
  std::vector<TValue> items;

public:
  OrderPreservingMap() {}

  void AddItem(const TKey &key, const TValue &item);

  TValue GetItemByKey(const TKey &key) const;

  TValue GetItemByIndex(size_t index) const;

  size_t GetIndexByKey(const TKey &key) const;

  std::vector<TKey> GetAllKeys() const;

  const std::vector<TValue> &GetAllItems() const;

  bool ContainsKey(const TKey &key) const;
};

template <typename TKey, typename TValue>
void OrderPreservingMap<TKey, TValue>::AddItem(const TKey &key,
                                               const TValue &item) {
  if (indexMap.find(key) != indexMap.end()) {
    throw std::invalid_argument(
        "The name already exists in the OrderPreservingMap.");
  } else {
    indexMap[key] = items.size();
    items.push_back(item);
  }
}

template <typename TKey, typename TValue>
TValue OrderPreservingMap<TKey, TValue>::GetItemByKey(const TKey &key) const {
  if (indexMap.find(key) != indexMap.end()) {
    return items[indexMap.at(key)];
  } else {
    throw std::invalid_argument(
        "The name doesn't exist in the OrderPreservingMap.");
  }
}

template <typename TKey, typename TValue>
TValue OrderPreservingMap<TKey, TValue>::GetItemByIndex(size_t index) const {
  if (index >= 0 && index < items.size()) {
    return items[index];
  } else {
    throw std::invalid_argument("The index is out of bound.");
  }
}

template <typename TKey, typename TValue>
size_t OrderPreservingMap<TKey, TValue>::GetIndexByKey(const TKey &key) const {
  if (indexMap.find(key) != indexMap.end()) {
    return indexMap.at(key);
  } else {
    throw std::invalid_argument(
        "The name doesn't exist in the OrderPreservingMap.");
  }
}

template <typename TKey, typename TValue>
std::vector<TKey> OrderPreservingMap<TKey, TValue>::GetAllKeys() const {
  std::vector<TKey> keys;
  keys.reserve(indexMap.size());

  for (const auto &pair : indexMap) {
    keys.push_back(pair.first);
  }

  return keys;
}

template <typename TKey, typename TValue>
const std::vector<TValue> &
OrderPreservingMap<TKey, TValue>::GetAllItems() const {
  return items;
}

template <typename TKey, typename TValue>
bool OrderPreservingMap<TKey, TValue>::ContainsKey(const TKey &key) const {
  return (indexMap.find(key) != indexMap.end());
}

}; /* namespace Utility */
}; /* namespace Cygni */

#endif /* CYGNI_ORDER_PRESERVING_MAP_HPP */
