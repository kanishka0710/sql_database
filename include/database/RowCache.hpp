//
// Created by aryam on 25-05-2024.
//

#ifndef ECE141DB_ROWCACHE_HPP
#define ECE141DB_ROWCACHE_HPP
#include <iostream>
#include "misc/LRUCache.hpp"
#include "database/Row.hpp"
#include "misc/Config.hpp"

namespace ECE141 {

    class RowCache {
    public:
        static RowCache& getInstance() {
            static RowCache instance;
            return instance;
        }

        RowCache(const RowCache&) = delete;
        RowCache& operator=(const RowCache&) = delete;

        void setCacheSize(size_t newSize) {
            cacheSize = newSize;
            cache.resize(newSize);
        }

        size_t getCacheSize() const {
            return cacheSize;
        }

        void addToCache(uint32_t key, const Row& value) {
            cache.put(key, value);
        }

        bool removeFromCache(uint32_t key) {
            return cache.erase(key);
        }

        bool getFromCache(uint32_t key, Row& value) {
            if (cache.contains(key)) {
                value = cache.get(key);
                return true;
            }
            return false;
        }

        bool inCache(uint32_t key){
            return cache.contains(key);
        }

    private:
        RowCache() : cacheSize(Config::getCacheSize(CacheType::rows)), cache(cacheSize) {}

        LRUCache<uint32_t, Row> cache;
        size_t cacheSize;
    };

}

#endif //ECE141DB_ROWCACHE_HPP
