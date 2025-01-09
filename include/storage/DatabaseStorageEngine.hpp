#ifndef ECE141DB_DATABASESTORAGEENGINE_HPP
#define ECE141DB_DATABASESTORAGEENGINE_HPP

#include <string>
#include "database/Database.hpp"
#include "misc/Config.hpp"
#include "misc/LRUCache.hpp"

namespace ECE141 {

    class DatabaseStorageEngine {
    public:
        static DatabaseStorageEngine& getInstance() {
            static DatabaseStorageEngine instance;
            return instance;
        }

        // Method to update the singleton object
        void setLiveDatabase(const std::string& aPath) {
            databaseName = aPath;
        }

        // Method to access the data of the singleton object
        std::string getLiveDatabase() const {
            return databaseName;
        }

        void setCacheSize(size_t aSize=Config::getCacheSize(CacheType::block)) {
            cacheSize = aSize;
            cache.resize(cacheSize);
        }

        size_t getCacheSize() const {
            return cacheSize;
        }

        bool getFromCache(uint32_t key, Block& block) {
            if (cache.contains(key)) {
                block = cache.get(key);
                return true;
            }
            return false;
        }

        void putInCache(uint32_t key, const Block& block) {
            cache.put(key, block);
        }

        bool inCache(uint32_t key){
            return cache.contains(key);
        }

    protected:
        LRUCache<uint32_t, Block> cache;
        size_t cacheSize;

    private:
        DatabaseStorageEngine()
                : cacheSize(Config::getCacheSize(CacheType::block)), cache(Config::getCacheSize(CacheType::block)) {}

        // Private copy constructor and assignment operator to prevent cloning
        DatabaseStorageEngine(const DatabaseStorageEngine&) = delete;
        DatabaseStorageEngine& operator=(const DatabaseStorageEngine&) = delete;

        std::string databaseName;
    };

}

#endif // ECE141DB_DATABASESTORAGEENGINE_HPP
