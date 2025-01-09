//
//  BlockIO.cpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//

#include <cstring>
#include "storage/BlockIO.hpp"
#include "misc/Config.hpp"
#include "storage/DatabaseStorageEngine.hpp"

namespace ECE141 {

    Block::Block(BlockType aType) {
        header.type = static_cast<char>(aType);
        header.nextBlockNum = -1;
        std::memset(payload, 0, kPayloadSize);
    }

    Block::Block(const Block &aCopy)  : header(aCopy.header){
        std::memcpy(payload, aCopy.payload, kPayloadSize);
    }

    Block &Block::operator=(const Block &aCopy) {
        if (this != &aCopy) {
            std::memcpy(payload, aCopy.payload, kPayloadSize);
            header = aCopy.header;
        }
        return *this;
    }

    StatusResult Block::write(std::ostream &aStream) {
        aStream.write((char*)this, kBlockSize);
        return StatusResult{Errors::noError};
    }

    //---------------------------------------------------

    struct modeToInt {
        std::ios::openmode operator()(CreateFile &aVal) { return aVal; }

        std::ios::openmode operator()(OpenFile &aVal) { return aVal; }
    };

    BlockIO::BlockIO(const std::string &aName, AccessMode aMode) {
        std::string thePath = Config::getDBPath(aName);

        auto theMode = std::visit(modeToInt(), aMode);
        stream.clear(); // Clear flag just-in-case...
        stream.open(thePath.c_str(), theMode); //force truncate if...
        stream.close();
        stream.open(thePath.c_str(), theMode);
    }

    // USE: write data a given block (after seek) ---------------------------------------
    StatusResult BlockIO::writeBlock(uint32_t aBlockNum, Block &aBlock) {
        if (aBlockNum > getBlockCount())
            return StatusResult{Errors::invalidArguments};

        stream.clear();
        stream.seekg(static_cast<std::streamoff>(aBlockNum * kBlockSize));
        if (!stream.write(reinterpret_cast<const char*>(&aBlock), kBlockSize)) {
            throw std::runtime_error("failed write to db");
        }

        if (Config::useCache(CacheType::block)) {
            if(DatabaseStorageEngine::getInstance().inCache(aBlockNum))
                DatabaseStorageEngine::getInstance().putInCache(aBlockNum, aBlock);
        }

        return StatusResult{Errors::noError};
    }

    // USE: write data a given block (after seek) ---------------------------------------
    StatusResult BlockIO::getBlock(uint32_t aBlockNumber, Block &aBlock){
        stream.clear();
        stream.seekp(static_cast<std::streamoff>(aBlockNumber * kBlockSize));
        if(!stream.read(reinterpret_cast<char*>(&aBlock), kBlockSize)){
            throw std::runtime_error("failed read to db");
            return StatusResult{Errors::readError};
        }
        return StatusResult{Errors::noError};
    }

    StatusResult BlockIO::readBlock(uint32_t aBlockNumber, Block &aBlock) {
        if(Config::useCache(CacheType::block)){
            if(!DatabaseStorageEngine::getInstance().getFromCache(aBlockNumber,aBlock)){
                getBlock(aBlockNumber, aBlock);
                DatabaseStorageEngine::getInstance().putInCache(aBlockNumber, aBlock);
            }
            else return StatusResult{Errors::noError};
        }
        else {
            if (aBlockNumber >= getBlockCount())
                return StatusResult{Errors::invalidArguments};

            getBlock(aBlockNumber, aBlock);
        }
            return StatusResult{Errors::noError};
    }

    // USE: count blocks in file ---------------------------------------
    size_t BlockIO::getBlockCount() {
        if (!stream.is_open())
            return 0;
        if(!stream.good()){
            stream.clear();
        }
        const auto previousPosition = stream.tellg();
        stream.seekg(0, std::ios::end);
        const auto endPosition = stream.tellg();
        stream.seekg(previousPosition);

        return (size_t)endPosition / kBlockSize;
    }

}
