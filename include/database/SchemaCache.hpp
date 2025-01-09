//
// Created by aryam on 28-05-2024.
//

#ifndef ECE141DB_SCHEMACACHE_HPP
#define ECE141DB_SCHEMACACHE_HPP
#include <iostream>
#include "misc/LRUCache.hpp"
#include "Schema.hpp"
#include "misc/Config.hpp"

namespace ECE141 {

    class SchemaCache {
    public:
        static SchemaCache& getInstance() {
            static SchemaCache instance;
            return instance;
        }

        SchemaCache(const SchemaCache&) = delete;
        SchemaCache& operator=(const SchemaCache&) = delete;

        void setCacheSize(size_t newSize) {
            cacheSize = newSize;
            cache.resize(newSize);
        }

        size_t getCacheSize() const {
            return cacheSize;
        }

        void addToCache(uint32_t key, const Schema& value) {
            cache.put(key, value);
        }

        bool removeFromCache(uint32_t key) {
            return cache.erase(key);
        }

        bool getFromCache(uint32_t key, Schema& value) {
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
        SchemaCache() : cacheSize(Config::getCacheSize(CacheType::schema)), cache(cacheSize) {}

        LRUCache<uint32_t, Schema> cache;
        size_t cacheSize;
    };

}
#endif //ECE141DB_SCHEMACACHE_HPP
