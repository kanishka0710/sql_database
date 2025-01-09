//
// Created by aryam on 28-05-2024.
//

#ifndef ECE141DB_StorableCache_HPP
#define ECE141DB_StorableCache_HPP

#include <iostream>
#include "misc/LRUCache.hpp"
#include "misc/Config.hpp"
#include "Schema.hpp"
#include "TOC.hpp"

namespace ECE141 {
    using Storables=std::variant<Schema, DbTOC, TableTOC>;
    class StorableCache {
    public:
        static StorableCache& getInstance() {
            static StorableCache instance;
            return instance;
        }

        StorableCache(const StorableCache&) = delete;
        StorableCache& operator=(const StorableCache&) = delete;

        void setCacheSize(const size_t newSize) {
            cacheSize = newSize;
            cache.resize(newSize);
        }

        size_t getCacheSize() const {
            return cacheSize;
        }

        void addToCache(uint32_t key, const Storables& value) {
            cache.put(key, value);
        }

        bool removeFromCache(const uint32_t key) {
            return cache.erase(key);
        }

        bool getFromCache(uint32_t key, Storables& value) {
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
        StorableCache() : cacheSize(Config::getCacheSize(CacheType::storable)), cache(cacheSize) {}
        
        LRUCache<uint32_t, Storables> cache;
        size_t cacheSize;
    };

}

#endif //ECE141DB_StorableCache_HPP
