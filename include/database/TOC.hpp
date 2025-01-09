//
// Created by Matthew Dacosta on 4/28/24.
//

#ifndef ECE141DB_TOC_HPP
#define ECE141DB_TOC_HPP

#include <memory>
#include <string>
#include <map>
#include <unordered_map>

#include "Attribute.hpp"
#include "misc/Errors.hpp"
#include "storage/BinaryBuffer.h"
#include "storage/Storage.hpp"


namespace ECE141 {


    class DbTOC : public Storable {
        using tableMap = std::unordered_map<std::string, size_t>;
        std::hash<std::string> hash_fn;
    public:
        DbTOC() = default;
        DbTOC(const std::string aName);

        DbTOC(const DbTOC &aCopy);

        ~DbTOC();

        const std::string &getName() const { return name; }

        void setNextPart(size_t &aNextBlock);

        void addTable(std::string &aTableName, size_t &aBlockIndex);

        void removeTable(std::string &aTableName);

        uint32_t findTable(const std::string &aTableName);

        //uint32_t findSchema(const std::string &aTableName);

        size_t getNextPart() {return nextPart;}

        size_t getTOClength() {return tocLength;}

        size_t getBinarySize() const override;

        StatusResult encode(BinaryBuffer &output) const override;

        StatusResult decode(BinaryBuffer &input) override;

        bool initHeader(Block &aBlock) const override;

        std::vector<std::string> getTableNames();

    protected:
        size_t nextPart; // next page of TOC
        size_t tocLength; // number of tables stored
        tableMap DBtoc; // hashed table name and BlockIndex
        std::string name; // name of DB?
    };

    class TableTOC : public Storable {
        using rowMap = std::map<size_t , size_t>;
    public:
        TableTOC() = default;
        TableTOC(const std::string aName);

        TableTOC(const TableTOC &aCopy);

        ~TableTOC();

        const std::string &getName() const { return name; } //holds table name?

        void setNextPart(size_t &aNextBlock); // sets next TOC page

        void addRow(const uint32_t aBlockIndex);

        void removeRow(const size_t &aRowIndex); // remove from TOC

        uint32_t findRow(size_t &aRowIndex);

        uint32_t findSchema();

        size_t getNextPart() {return nextPart;}

        size_t getNextRow() {return nextRow;} // keeps track of next row index

        size_t getTOClength() {return tocLength;}

        rowMap getMap() {return tabletoc;}

        size_t getBinarySize() const override;

        StatusResult encode(BinaryBuffer &output) const override;

        StatusResult decode(BinaryBuffer &input) override;

        bool initHeader(Block &aBlock) const override;


    protected:
        size_t nextPart; // next page of TOC
        size_t tocLength; // number of rows stored
        size_t nextRow; // next row index to be used
        rowMap tabletoc; // row index and BlockIndex
        std::string name; // name of table?
    };

}


#endif //ECE141DB_TOC_HPP
