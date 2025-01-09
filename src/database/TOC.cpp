//
// Created by Matthew Dacosta on 4/28/24.
//

#include "database/TOC.hpp"
#include "storage/BlockIO.hpp"
#include "storage/BinaryBuffer.h"
#include <map>

namespace ECE141 {

    //--------------DataBase TOC ------------------------
    DbTOC::DbTOC(const std::string aName) : name(aName) {
        nextPart = 0;
        tocLength = 0;
        DBtoc.clear();
    }

    DbTOC::DbTOC(const DbTOC &aCopy) {
        name = aCopy.name;
        nextPart = aCopy.nextPart;
        tocLength = aCopy.tocLength;
        DBtoc = aCopy.DBtoc;
    }

    DbTOC::~DbTOC() = default;

    void DbTOC::setNextPart(size_t &aNextBlock) {
        nextPart = aNextBlock;
    }

    void DbTOC::addTable(std::string &aTableName, size_t &aBlockIndex) {
        DBtoc.insert({aTableName, aBlockIndex});
//        DBtoc.insert({hash_fn(aTableName) , aBlockIndex});
        tocLength+=1;
    }

    uint32_t DbTOC::findTable(const std::string &aTableName) {
//        auto hashedTableName = hash_fn(aTableName);
        auto it = DBtoc.find(aTableName);
        if (it != DBtoc.end()) { // If the key is found
            return it->second;
        }else {
            return 0;
        }
    }


    void DbTOC::removeTable(std::string &aTableName) {
//        auto it = DBtoc.find(hash_fn(aTableName));
        auto it = DBtoc.find(aTableName);
        if (it != DBtoc.end()) { // If the key is found
            DBtoc.erase(it); // Remove the pair
        }
        tocLength-=1;
    }

    size_t DbTOC::getBinarySize() const {
        size_t res = 0;
        res += name.size();
        res += sizeof(nextPart);
        res += sizeof(tocLength);
        for (auto &it : DBtoc) {
            res += it.first.size();
            res += sizeof(it.second);
        }
        return res;
    }

    StatusResult DbTOC::encode(BinaryBuffer &output) const  {
        output.writeString(name);
        output.write(nextPart);
        output.write(tocLength);
        for (auto& it : DBtoc) {
            output.writeString(it.first);
            output.write(it.second);
        }
        return Errors::noError;
    }

    StatusResult DbTOC::decode(BinaryBuffer &input)  {
        name = input.readString();
        nextPart = input.read<size_t>();
        tocLength = input.read<size_t>();
        std::string tableName;
        size_t aBlock;
        for (size_t i = 0; i < tocLength; i++) {
//            tableName = input.read<size_t >();
            tableName = input.readString();
            aBlock = input.read<size_t>();
            DBtoc.insert({tableName,aBlock});
        }
        return Errors::noError;
    }

    bool DbTOC::initHeader(Block &aBlock) const {
        aBlock.header.type = static_cast<char>(BlockType::TOC_block);
        return true;
    }

    std::vector<std::string> DbTOC::getTableNames() {
        std::vector<std::string> result;
        for (auto& it : DBtoc) {
            result.emplace_back(it.first);
        }
        return result;
    }




    //--------------Table TOC ------------------------
    TableTOC::TableTOC(const std::string aName) : name(aName), nextRow(0), nextPart(0), tocLength(0) {}

    TableTOC::TableTOC(const TableTOC &aCopy) {
        name = aCopy.name;
        nextPart = aCopy.nextPart;
        nextRow = aCopy.nextRow;
        tocLength = aCopy.tocLength;
        tabletoc = aCopy.tabletoc;
    }

    TableTOC::~TableTOC() = default;

    void TableTOC::setNextPart(size_t &aNextBlock) {
        nextPart = aNextBlock;
    }

    void TableTOC::addRow(const uint32_t aBlockIndex) {
        tabletoc.insert({nextRow, aBlockIndex});
        nextRow+=1;
        tocLength+=1;
    }

    uint32_t TableTOC::findRow(size_t &aRowIndex) {
        auto it = tabletoc.find(aRowIndex);
        if (it != tabletoc.end()) { // If the key is found
            return it->second;
        } else{
            return 0;
        }
    }

    uint32_t TableTOC::findSchema(){
        // the schema SHOULD be saved in Row zero so we can get its block index
        size_t schemaRow = 0;
        return findRow(schemaRow);
    }

    void TableTOC::removeRow(const size_t &aRowIndex) {
        auto it = tabletoc.find(aRowIndex);
        if (it != tabletoc.end()) { // If the key is found
            tabletoc.erase(it); // Remove the pair
        }
        tocLength-=1;
    }

    size_t TableTOC::getBinarySize() const {
        size_t res = 0;
        res += name.size();
        res += sizeof(nextPart);
        res += sizeof(nextRow);
        res += sizeof(tocLength);
        for (auto &it : tabletoc) {
            res += sizeof(it.first);
            res += sizeof(it.second);
        }
        return res;
    }

    StatusResult TableTOC::encode(BinaryBuffer &output) const  {
        output.writeString(name);
        output.write(nextPart);
        output.write(nextRow);
        output.write(tocLength);
        for (auto& it : tabletoc) {
            output.write(it.first);
            output.write(it.second);
        }
        return Errors::noError;
    }

    StatusResult TableTOC::decode(BinaryBuffer &input)  {
        name = input.readString();
        nextPart = input.read<size_t>();
        nextRow = input.read<size_t>();
        tocLength = input.read<size_t>();
        size_t rowNum;
        size_t aBlock;
        for (size_t i = 0; i < tocLength; i++) {
            rowNum = input.read<size_t>();
            aBlock = input.read<size_t>();
            tabletoc.insert({rowNum,aBlock});
        }
        return Errors::noError;
    }

    bool TableTOC::initHeader(Block &aBlock) const {
        aBlock.header.type = static_cast<char>(BlockType::TOC_block);
        return true;
    }
}