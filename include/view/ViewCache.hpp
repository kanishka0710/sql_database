//
// Created by aryam on 25-05-2024.
//

#ifndef ECE141DB_VIEWCACHE_HPP
#define ECE141DB_VIEWCACHE_HPP
#include <iostream>
#include "misc/LRUCache.hpp"
#include "View.hpp"
#include "misc/Config.hpp"
#include "parsing/CommandStatement.hpp"

namespace ECE141 {

    class ViewCache {
    public:
        static ViewCache& getInstance() {
            static ViewCache instance;
            return instance;
        }

        // Delete copy constructor and assignment operator to prevent copies
        ViewCache(const ViewCache&) = delete;
        ViewCache& operator=(const ViewCache&) = delete;

        // Method to set cache size
        void setCacheSize(size_t newSize) {
            cacheSize = newSize;
            cache.resize(newSize);
        }

        // Method to get cache size
        size_t getCacheSize() const {
            return cacheSize;
        }

        // Method to add an element to the cache
        void addToCache(const CommandStatement& key, const View& value) {
            cache.put(key, value);
        }

        // Method to remove an element from the cache
        bool removeFromCache(const CommandStatement& key) {
            return cache.erase(key);
        }

        // Method to get an element from the cache
        bool getFromCache(const CommandStatement& key, View& value) {
            if (cache.contains(key)) {
                value = cache.get(key);
                return true;
            }
            return false;
        }

    private:
        // Private constructor for singleton pattern
        ViewCache() : cacheSize(Config::getCacheSize(CacheType::views)), cache(cacheSize) {}

        LRUCache<CommandStatement, View> cache;
        size_t cacheSize;
    };

}

#endif // ECE141DB_VIEWCACHE_HPP

