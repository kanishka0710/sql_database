//
// Created by kanis on 6/6/2024.
//

#include "controller/ProcessorHelpers.hpp"
#include "database/Row.hpp"
#include "storage/DatabaseStorageEngine.hpp"
#include "misc/Helpers.hpp"
#include "misc/Logger.hpp"
#include "database/TOC.hpp"
#include <cstdio>

namespace ECE141 {

    std::vector<StringVector>
    ProcessorHelpers::handleJoin(DBQuery &aQuery, std::map<std::string, Schema> &schemasRequired,
                                 const std::map<std::string, StringVector> &headers,
                                 const std::map<std::string, std::multimap<Value, size_t>> &validRows) {
        std::vector<StringVector> content;
        switch (aQuery.getJoin().joinType) {
            case Keywords::left_kw:
                content = ProcessorHelpers::leftJoin(aQuery, schemasRequired, headers, validRows);
                break;
            case Keywords::right_kw:
                content = ProcessorHelpers::rightJoin(aQuery, schemasRequired, headers, validRows);
                break;
            default:
                break;
        }
        return content;
    }

    std::vector<StringVector>
    ProcessorHelpers::leftJoin(DBQuery &aQuery, std::map<std::string, Schema> &schemasRequired,
                               std::map<std::string, StringVector> headers,
                               std::map<std::string, std::multimap<Value, size_t>> validRows) {

//        Logger::getInstance().setStream(std::cout);
//        Logger::log(LogLevel::Info, "Starting Left Join");

        std::vector<StringVector> content;
        // Initialize Storage
        std::string databaseInstance = DatabaseStorageEngine::getInstance().getLiveDatabase();
        Storage aStorage{databaseInstance, OpenFile()};

        // Load up Join Query
        Join aJoin = aQuery.getJoin();
        std::string primaryKey = aJoin.onLeft.table;
        std::string secondaryKey = aJoin.onRight.table;

        // Load up all the right hand rows for comparison
        std::unordered_multimap<std::string, std::map<std::string, Value>> matchingContent;
        for (const auto &goodIndex: validRows[secondaryKey]) {
            Row aRow;
            aRow.setSchema(schemasRequired[secondaryKey]);
            aStorage.load(aRow, goodIndex.second);
            std::map<std::string, Value> rowContent;
            processRowData(aRow.getData(), headers, rowContent, secondaryKey);
            // Place the content in the map with the join attribute as the key
            auto keyToMatch = findKeyToMatch(aRow, headers, secondaryKey, aJoin.onRight.fieldName);
            matchingContent.emplace(std::pair(keyToMatch, rowContent));
        }

        for (const auto &goodIndex: validRows[primaryKey]) {
            Row aRow;
            aRow.setSchema(schemasRequired[primaryKey]);
            aStorage.load(aRow, goodIndex.second);

            StringVector rowContent;
            auto keyToMatch = findKeyToMatch(aRow, headers, primaryKey, aJoin.onLeft.fieldName);
            auto range = matchingContent.equal_range(keyToMatch);
            if (range.first == range.second) {
                processRowData(aRow.getData(), headers, rowContent, primaryKey);
                for (size_t i = 0; i < headers[secondaryKey].size(); i++) {
                    rowContent.emplace_back("NULL");
                }
                content.push_back(rowContent);
            } else {
                for (auto it = range.first; it != range.second; ++it) {
                    processRowData(aRow.getData(), headers, rowContent, primaryKey);
                    for (auto& fieldName : headers[secondaryKey]) {
                        if (it->second.find(fieldName) != it->second.end()) {
                            rowContent.push_back(Helpers::variantToString(it->second[fieldName]));
                        }
                    }
                    content.push_back(rowContent);
                    rowContent.clear();
                }
            }
        }
//        Logger::log(LogLevel::Info, "Finished Left Join");
        return content;
    }


    std::vector<StringVector>
    ProcessorHelpers::rightJoin(DBQuery &aQuery, std::map<std::string, Schema> &schemasRequired,
                                std::map<std::string, StringVector> headers,
                                std::map<std::string, std::multimap<Value, size_t>> validRows) {

        std::vector<StringVector> content;

        // Initialize Storage
        std::string databaseInstance = DatabaseStorageEngine::getInstance().getLiveDatabase();
        Storage aStorage{databaseInstance, OpenFile()};

        // Load up Join Query
        Join aJoin = aQuery.getJoin();
        std::string primaryKey = aJoin.onRight.table;
        std::string secondaryKey = aJoin.onLeft.table;

        // Load up all the right hand rows for comparison
        std::unordered_multimap<std::string, std::map<std::string, Value>> matchingContent;
        for (const auto &goodIndex: validRows[secondaryKey]) {
            Row aRow;
            aRow.setSchema(schemasRequired[secondaryKey]);
            aStorage.load(aRow, goodIndex.second);
            std::map<std::string, Value> rowContent;
            processRowData(aRow.getData(), headers, rowContent, secondaryKey);
            // Place the content in the map with the join attribute as the key
            auto keyToMatch = findKeyToMatch(aRow, headers, secondaryKey, aJoin.onLeft.fieldName);
            matchingContent.emplace(std::pair(keyToMatch, rowContent));
        }

        for (const auto &goodIndex: validRows[primaryKey]) {
            Row aRow;
            aRow.setSchema(schemasRequired[primaryKey]);
            aStorage.load(aRow, goodIndex.second);

            StringVector rowContent;
            auto keyToMatch = findKeyToMatch(aRow, headers, primaryKey, aJoin.onRight.fieldName);
            auto range = matchingContent.equal_range(keyToMatch);
            if (range.first == range.second) {
                for (size_t i = 0; i < headers[secondaryKey].size(); i++) {
                    rowContent.emplace_back("NULL");
                }
                processRowData(aRow.getData(), headers, rowContent, primaryKey);
                content.push_back(rowContent);
            } else {
                for (auto it = range.first; it != range.second; ++it) {
                    for (auto& fieldName : headers[secondaryKey]) {
                        if (it->second.find(fieldName) != it->second.end()) {
                            rowContent.push_back(Helpers::variantToString(it->second[fieldName]));
                        }
                    }
                    processRowData(aRow.getData(), headers, rowContent, primaryKey);
                    content.push_back(rowContent);
                    rowContent.clear();
                }
            }
        }
        return content;
    }

    template <typename RowContentType>
    void ProcessorHelpers::processRowData(RowKeyValues &rowData, std::map<std::string, StringVector> &headers,
                                     RowContentType &rowContent, std::string &key) {
        for (const auto &pair: rowData) {
            if (std::find(headers[key].begin(), headers[key].end(), pair.first) != headers[key].end()) {
                if constexpr (std::is_same_v<RowContentType, StringVector>) {
                    rowContent.push_back(Helpers::variantToString(pair.second));
                } else if constexpr (std::is_same_v<RowContentType, std::map<std::string, Value>>) {
                    rowContent.emplace(pair.first, Helpers::variantToString(pair.second));
                }
            }
        }
    }

    std::string ProcessorHelpers::findKeyToMatch(Row &aRow, std::map<std::string, StringVector> &headers, std::string &key, std::string &fieldName) {
        std::string keyToMatch;
        if (aRow.getData().find(fieldName) != aRow.getData().end()) {
            keyToMatch = Helpers::variantToString(aRow.getData()[fieldName]);
        } else if (std::find(headers[key].begin(), headers[key].end(), fieldName) !=
            headers[key].end()) {
            keyToMatch = Helpers::variantToString(aRow.getData()[fieldName]);
        } else if ("id" == fieldName) {
            keyToMatch = std::to_string(aRow.entityId);
        }
        return keyToMatch;
    }

    size_t ProcessorHelpers::getAndVerifyTable(std::string &tableName) {
        std::string databaseInstance = DatabaseStorageEngine::getInstance().getLiveDatabase();
        Storage aStorage{databaseInstance, OpenFile()};

        DbTOC aDbTOC = DbTOC(databaseInstance);
        // Check if the table exists in the TOC
        aStorage.load(aDbTOC, 0);
        size_t tableIndex = aDbTOC.findTable(tableName);
        if (!tableIndex || aDbTOC.getTOClength() == 0) {
            return -1;
        }
        return tableIndex;
    }


}