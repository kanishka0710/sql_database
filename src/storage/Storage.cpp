#include <sstream>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <cstring>
#include <iostream>
#include "storage/Storage.hpp"
#include "misc/Config.hpp"
#include "database/RowCache.hpp"
#include "database/StorableCache.hpp"
#include "misc/Logger.hpp"

namespace ECE141 {
    using Storables = std::variant<Schema, DbTOC, TableTOC>;

    bool Storage::each(const BlockVisitor &aVisitor) {
        Logger::log(LogLevel::Debug, "Storage::each - Visiting blocks");
        for (size_t i = 0; i < this->getBlockCount(); ++i) {
            Block aBlock;
            readBlock(i, aBlock);
            if (!aVisitor(aBlock, i)) {
                return false;
            }
        }
        return true;
    }

    std::vector<int> Storage::getRequiredBlockNums(const Storable &aStorable) {
        Logger::log(LogLevel::Debug, "Storage::getRequiredBlockNums - Calculating required block numbers");
        std::vector<int> res;
        int blockNums = std::ceil((float)(aStorable.getBinarySize()) / (float)(kPayloadSize));
        for (int i = 0; i < blockNums; i++) {
            res.push_back(this->getFreeBlock());
        }
        return res;
    }

    StatusResult Storage::save(const Storable &aStorable, int32_t aStartPos, bool free) {
        Logger::log(LogLevel::Debug, "Storage::save - Saving storable at position: ", aStartPos);
        BinaryBuffer binaryBuffer{};
        aStorable.encode(binaryBuffer);
        int blockNums = std::ceil((float)(binaryBuffer.getSize()) / (float)(kPayloadSize));
        if (blockNums > 1) {
            for (int i = 0; i < blockNums; i++) {
                auto block = Block();
                if(!free){aStorable.initHeader(block);}
                else{block.header.type = static_cast<char>(BlockType::free_block);}
                size_t bufferSize = 0;
                if (blockNums == i + 1) {
                    bufferSize = binaryBuffer.getSize() - i * kPayloadSize;
                } else {
                    Block checkBlock{};
                    BlockIO::readBlock(aStartPos, checkBlock);
                    if(checkBlock.header.nextBlockNum == -1)
                        block.header.nextBlockNum = getFreeBlock();
                    else
                        block.header.nextBlockNum = checkBlock.header.nextBlockNum;
                    bufferSize = kPayloadSize;
                }

                std::memcpy(block.payload, binaryBuffer.getBuffer().data() + (i * kPayloadSize), bufferSize);

                if (auto statusResult = BlockIO::writeBlock(aStartPos, block); !statusResult)
                    return statusResult;
                aStartPos = block.header.nextBlockNum;
            }
        } else {
            auto block = Block();
            if(!free){aStorable.initHeader(block);}
            else{block.header.type = static_cast<char>(BlockType::free_block);}
            block.header.nextBlockNum = -1;
            std::memcpy(block.payload, binaryBuffer.getBuffer().data(), binaryBuffer.getSize());
            if (auto statusResult = BlockIO::writeBlock(aStartPos, block); !statusResult) {
                return statusResult;
            }
        }

        // If the storable is a Row or schema and cache is enabled, update the cache
        if (Config::useCache(CacheType::rows)) {
            if (const Row* rowPtr = dynamic_cast<const Row*>(&aStorable)) {
                if (RowCache::getInstance().inCache(aStartPos)) {
                    RowCache::getInstance().addToCache(aStartPos, *rowPtr);
                    Logger::log(LogLevel::Debug, "Storage::save - Row added to cache at position: ", aStartPos);
                }
            }
        }

        // If the storable is a Schema, DbTOC, or TableTOC and cache is enabled, update the StorableCache
        if (Config::useCache(CacheType::storable)) {
            Storables storableVariant;
            if (const Schema* schemaPtr = dynamic_cast<const Schema*>(&aStorable)) {
                storableVariant = *schemaPtr;
            } else if (const DbTOC* dbTOCPtr = dynamic_cast<const DbTOC*>(&aStorable)) {
                storableVariant = *dbTOCPtr;
            } else if (const TableTOC* tableTOCPtr = dynamic_cast<const TableTOC*>(&aStorable)) {
                storableVariant = *tableTOCPtr;
            }

            if (std::holds_alternative<Schema>(storableVariant) ||
                std::holds_alternative<DbTOC>(storableVariant) ||
                std::holds_alternative<TableTOC>(storableVariant)) {
                if (StorableCache::getInstance().inCache(aStartPos)) {
                    StorableCache::getInstance().addToCache(aStartPos, storableVariant);
                    Logger::log(LogLevel::Debug, "Storage::save - Storable added to cache at position: ", aStartPos);
                }
            }
        }

        return Errors::noError;
    }

    StatusResult Storage::load(Storable& aStorable, uint32_t aStartBlockNum) {
        Logger::log(LogLevel::Debug, "Storage::load - Loading storable at position: ", aStartBlockNum);
        BinaryBuffer binaryBuffer;
        Block block;

        // Check cache first if the storable is a Row and cache is enabled
        if (Config::useCache(CacheType::rows)) {
            if (auto *rowPtr = dynamic_cast<Row *>(&aStorable)) {
                if (RowCache::getInstance().getFromCache(aStartBlockNum, *rowPtr)) {
                    Logger::log(LogLevel::Debug, "Storage::load - Row loaded from cache at position: ", aStartBlockNum);
                    return Errors::noError;
                }
            }
        }

        // Check StorableCache first if the storable is a Schema, DbTOC, or TableTOC and cache is enabled
        if (Config::useCache(CacheType::storable)) {
            Storables storableVariant;
            if (StorableCache::getInstance().getFromCache(aStartBlockNum, storableVariant)) {
                if (auto *schemaPtr = std::get_if<Schema>(&storableVariant)) {
                    aStorable = *schemaPtr;
                } else if (auto *dbTOCPtr = std::get_if<DbTOC>(&storableVariant)) {
                    aStorable = *dbTOCPtr;
                } else if (auto *tableTOCPtr = std::get_if<TableTOC>(&storableVariant)) {
                    aStorable = *tableTOCPtr;
                }
                Logger::log(LogLevel::Debug, "Storage::load - Storable loaded from cache at position: ", aStartBlockNum);
                return Errors::noError;
            }
        }

        BlockIO::readBlock(aStartBlockNum, block);
        size_t numBlocks = 0;
        binaryBuffer.getBuffer().resize(kPayloadSize);
        if (block.header.type != 'F') {
            std::memcpy(binaryBuffer.getBuffer().data(), block.payload, kPayloadSize);
        }

        while (block.header.nextBlockNum != -1) {
            BlockIO::readBlock(block.header.nextBlockNum, block);
            numBlocks++;
            binaryBuffer.getBuffer().resize(kPayloadSize * (numBlocks + 1));
            std::memcpy(binaryBuffer.getBuffer().data() + numBlocks * kPayloadSize, block.payload, kPayloadSize);
        }

        if (block.header.type != 'F') {
            aStorable.decode(binaryBuffer);

            // If the storable is a Row and cache is enabled, update the RowCache
            if (Config::useCache(CacheType::rows)) {
                if (Row *rowPtr = dynamic_cast<Row *>(&aStorable)) {
                    RowCache::getInstance().addToCache(aStartBlockNum, *rowPtr);
                    Logger::log(LogLevel::Debug, "Storage::load - Row added to cache at position: ", aStartBlockNum);
                }
            }

            // If the storable is a Schema, DbTOC, or TableTOC and cache is enabled, update the StorableCache
            if (Config::useCache(CacheType::storable)) {
                Storables storableVariant;
                if (Schema *schemaPtr = dynamic_cast<Schema *>(&aStorable)) {
                    storableVariant = *schemaPtr;
                } else if (DbTOC *dbTOCPtr = dynamic_cast<DbTOC *>(&aStorable)) {
                    storableVariant = *dbTOCPtr;
                } else if (TableTOC *tableTOCPtr = dynamic_cast<TableTOC *>(&aStorable)) {
                    storableVariant = *tableTOCPtr;
                }

                if (std::holds_alternative<Schema>(storableVariant) ||
                    std::holds_alternative<DbTOC>(storableVariant) ||
                    std::holds_alternative<TableTOC>(storableVariant)) {
                    StorableCache::getInstance().addToCache(aStartBlockNum, storableVariant);
                    Logger::log(LogLevel::Debug, "Storage::load - Storable added to cache at position: ", aStartBlockNum);
                }
            }
        }

        return Errors::noError;
    }

    int32_t Storage::getFreeBlock() {
        Logger::log(LogLevel::Debug, "Storage::getFreeBlock - Retrieving free block count");
        return BlockIO::getBlockCount();
    }

    StatusResult Storage::clean(uint32_t aStartBlockNum) {
        Logger::log(LogLevel::Debug, "Storage::clean - Cleaning block at position: ", aStartBlockNum);
        BinaryBuffer aBuffer{};
        auto aBlock = Block();

        if (auto statusResult = this->writeBlock(aStartBlockNum, aBlock); !statusResult) {
            return statusResult;
        }
        return Errors::noError;
    }

}
