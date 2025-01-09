//
//  BlockIO.hpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//

#ifndef BlockIO_hpp
#define BlockIO_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <variant>
#include <functional>
#include "misc/Errors.hpp"
#include <cstring>

namespace ECE141 {

    enum class BlockType {
        data_block = 'D',
        free_block = 'F',
        schema_block = 'S',
        //other types?
        unknown_block = 'U',
        TOC_block = 'T',
    };


    //a small header that describes the block...
    struct BlockHeader {

        BlockHeader(BlockType aType = BlockType::data_block)
                : type(static_cast<char>(aType)), nextBlockNum(-1) {}

        BlockHeader(const BlockHeader &aCopy) {
            type = aCopy.type;
            nextBlockNum = aCopy.nextBlockNum;
        }

        void empty() {
            type = static_cast<char>(BlockType::free_block);
        }

        BlockHeader &operator=(const BlockHeader &aCopy) {
            type = aCopy.type;
            nextBlockNum=aCopy.nextBlockNum;
            return *this;
        }

        char type;     //char version of block type
        int32_t nextBlockNum;
        //other properties?
    };

    const size_t kBlockSize = 1024;
    const size_t kPayloadSize = kBlockSize - sizeof(BlockHeader);

    //block .................
    class Block {
    public:
        Block(BlockType aType = BlockType::data_block);

        Block(const Block &aCopy);

        Block &operator=(const Block &aCopy);

        StatusResult write(std::ostream &aStream);

        BlockHeader header;
        char payload[kPayloadSize]{};
    };

    //------------------------------

    struct CreateFile {
        operator std::ios_base::openmode() {
            return std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc;
        }
    }; //tags for db-open modes...
    struct OpenFile {
        operator std::ios_base::openmode() {
            return std::ios::binary | std::ios::in | std::ios::out;
        }
    };

    using AccessMode = std::variant<CreateFile, OpenFile>;

    class BlockIO {
    public:

        BlockIO(const std::string &aName, AccessMode aMode);

        size_t getBlockCount();

        virtual StatusResult readBlock(uint32_t aBlockNumber, Block &aBlock);

        virtual StatusResult writeBlock(uint32_t aBlockNumber, Block &aBlock);

        bool isOpen() { return stream.is_open();}

    protected:
        std::fstream stream;

        StatusResult getBlock(uint32_t aBlockNumber, Block &aBlock);
    };

}


#endif /* BlockIO_hpp */
