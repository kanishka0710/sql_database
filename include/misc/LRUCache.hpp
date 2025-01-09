#ifndef LRUCACHE_HPP
#define LRUCACHE_HPP

#include <unordered_map>
#include <list>
#include <stdexcept>

namespace ECE141 {

    template<typename KeyT, typename ValueT>
    class LRUCache {
    public:
        LRUCache(size_t aCapacity=200) : capacity(aCapacity) {}

        size_t size() const { return cacheMap.size(); }

        bool contains(const KeyT& key) const {
            return cacheMap.find(key) != cacheMap.end();
        }

        void evictIf() {
            if (cacheMap.size() > capacity) {
                auto last = cacheList.back();
                cacheMap.erase(last.first);
                cacheList.pop_back();
            }
        }

        void add(const KeyT& key, const ValueT& value) {
            evictIf(); // Check and evict if necessary
            cacheList.push_front(std::make_pair(key, value));
            auto result=cacheMap.insert({key, cacheList.begin()});
            if(!result.second)
                throw std::runtime_error("failed add to cache");
        }

        void put(const KeyT& key, const ValueT& value) {
            auto it = cacheMap.find(key);
            if (it != cacheMap.end()) {
                // Remove existing entry from the list and map
                cacheList.erase(it->second);
                cacheMap.erase(it);
            }
            // Add new entry
            add(key, value);
        }

        void update(const KeyT& key, const ValueT& value) {
            put(key, value); // Using put to update the value
        }

        const ValueT& get(const KeyT& key) {
            auto it = cacheMap.find(key);
            if (it == cacheMap.end()) {
                throw std::runtime_error("Key not found");
            }
            // Move accessed item to the front of the list (most recently used)
            cacheList.splice(cacheList.begin(), cacheList, it->second);
            return it->second->second;
        }

        void clear() {
            cacheMap.clear();
            cacheList.clear();
        }

        bool erase(const KeyT &aKey) {
            auto it = cacheMap.find(aKey);
            if (it == cacheMap.end()) return false;
            cacheList.erase(it->second);
            cacheMap.erase(it);
            return true;
        }

        void resize(const size_t newCapacity) {
            capacity = newCapacity;
            while (cacheMap.size() > capacity) {
                evictIf();
            }
        }

    private:
        size_t capacity;
        std::list<std::pair<KeyT, ValueT>> cacheList;
        std::unordered_map<KeyT, typename std::list<std::pair<KeyT, ValueT>>::iterator> cacheMap;
    };

}

#endif // LRUCACHE_HPP
