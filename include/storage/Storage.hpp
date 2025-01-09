//
//  Storage.hpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//

#ifndef Storage_hpp
#define Storage_hpp

#include <string>
#include <fstream>
#include <iostream>
#include <deque>
#include <stack>
#include <optional>
#include <functional>
#include "BlockIO.hpp"
#include "misc/Errors.hpp"
#include "storage/BinaryBuffer.h"
#include "misc/LRUCache.hpp"
//#include "RowCache.hpp"

namespace ECE141 {
    class Row;
    class RowCache;
    class Storable;
}

namespace ECE141 {

    const int32_t kNewBlock = 0;

    class Storable {
    public:
        virtual StatusResult encode(BinaryBuffer &output) const = 0;

        virtual StatusResult decode(BinaryBuffer &input) = 0;

        virtual bool initHeader(Block &aBlock) const = 0;

        virtual size_t getBinarySize() const = 0;

        char type;


    };

    using BlockVisitor = std::function<bool(const Block &, uint32_t)>;
    using BlockList = std::deque<uint32_t>;

    // USE: Our storage manager class...
    class Storage : public BlockIO {
    public:

        Storage(const std::string &aName, AccessMode aMode) : BlockIO(aName, aMode) {}

        ~Storage() = default;

        bool each(const BlockVisitor &aVisitor);

        StatusResult clean(uint32_t aStartBlockNum);

        std::vector<int> getRequiredBlockNums(const Storable &aStorable);

        StatusResult save(const Storable &aStorable, int32_t aStartPos=kNewBlock, bool free=false);
        StatusResult load(Storable& aStorable, uint32_t aStartBlockNum);

        //StatusResult markBlockAsFree(uint32_t aPos); maybe?
        int32_t getFreeBlock();

    };

}


#endif /* Storage_hpp */
