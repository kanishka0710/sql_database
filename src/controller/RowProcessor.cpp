//
// Created by Matthew Dacosta on 5/2/24.
//

#include "controller/TableProcessor.hpp"
#include "parsing/CommandStatement.hpp"
#include "controller/ProcessorHelpers.hpp"
#include "misc/BasicTypes.hpp"

#include "controller/RowProcessor.hpp"
#include "misc/Validation.h"
#include <algorithm>

namespace ECE141 {

    CommandProcessor* RowProcessor::recognises(Tokenizer &aTokenizer) {
        KWList validList = {Keywords::insert_kw, Keywords::select_kw, Keywords::update_kw, Keywords::delete_kw};
        ECE141::TokenSequencer aSequencer(aTokenizer);
        return aSequencer.skipIf(validList) ? this : nullptr;
    }

    ECE41::SharedStatement RowProcessor::makeStatement(Tokenizer &aTokenizer, StatusResult &aResult) {
        auto aStatement = std::make_shared<CommandStatement>();
        ECE41::TableCommand aCommand(aTokenizer);
        if (!aCommand.createCommand(*aStatement, aResult)) {
            return std::nullopt;
        }
        return aStatement;
    }


    void RowProcessor::insertRow(std::string &tableName, std::vector<ECE141::Attribute> &attributeList, std::vector<std::vector<AttributeValuePairs>> &values, ViewListener &aViewer) {

        TableView2 aView{};
        std::string databaseInstance = DatabaseStorageEngine::getInstance().getLiveDatabase();
        Storage aStorage{databaseInstance, OpenFile()};

        size_t tableIndex = ProcessorHelpers::getAndVerifyTable(tableName);
        if (-1 == tableIndex) {
            failedOperation(aView, aViewer);
            return;
        }

        TableTOC aTableTOC{};
        aStorage.load(aTableTOC, tableIndex);
        Schema aSchema{};
        uint32_t schemaBlock = aTableTOC.findSchema();
        aStorage.load(aSchema, schemaBlock);
        if (!aSchema.validateAttributes(attributeList)) {
            failedOperation(aView,aViewer);
            return;
        }

        Row newRow{};
        newRow.setSchema(aSchema);
        for (auto &list : values) {
            newRow = Row(aTableTOC.getNextRow());
            newRow.blockNumber = aStorage.getFreeBlock();
            for (auto &it : list) {
                newRow.set(it.anAttribute.getName(), it.value);
            }
            aTableTOC.addRow(newRow.blockNumber);
            aStorage.save(newRow, newRow.blockNumber);
        }
        aStorage.save(aTableTOC, tableIndex);
        TableTOC aTest{}; //test added
        aStorage.load(aTest, tableIndex);
        aView.setIfTable(false);
        aView.setCount(values.size());
        aView.setSuccess(true);
        aViewer(aView);
    }

    void RowProcessor::selectRow(DBQuery &aQuery, ViewListener &aViewer) {

        TableView2 aView{};
        std::string databaseInstance = DatabaseStorageEngine::getInstance().getLiveDatabase();
        Storage aStorage{databaseInstance, OpenFile()};

        auto tableNames = aQuery.getAllSchemaNamesRequired();
        std::string orderByColumn = aQuery.getOrderByField();

        std::vector<StringVector> content;

        std::map<std::string, Schema> schemasRequired;
        std::map<std::string, TableTOC> tableMap;

        // Load the Database TOC
        DbTOC aDbTOC = DbTOC(databaseInstance);
        aStorage.load(aDbTOC, 0);

        // Load the TableMaps and Schemas Required
        for (auto& schemaName : tableNames) {
            // Check if the table exists in the TOC
            size_t tableIndex = aDbTOC.findTable(schemaName);
            if (!tableIndex || aDbTOC.getTOClength() == 0) {
                failedOperation(aView,aViewer);
                return;
            }

            TableTOC aTableTOC;
            aStorage.load(aTableTOC, tableIndex);

            Schema aSchema;
            aStorage.load(aSchema, aTableTOC.findSchema());

//            if (!ECE141::Validation::validateDBQuery(aQuery, aSchema)) {
//                 failedOperation(aView,aViewer);
//                 return;
//            }

            tableMap.emplace(std::pair<std::string, TableTOC>(schemaName, aTableTOC));
            schemasRequired.emplace(std::pair<std::string, Schema>(schemaName, aSchema));
        }

        // Gets all the attributes required
        std::map<std::string, StringVector> headers; // key = table name, value is the attributes
        for (auto& schema : schemasRequired) {
            StringVector attributes = schema.second.getAttributeNames(aQuery);
            headers.emplace(std::pair<std::string, StringVector>(schema.second.getName(), attributes));
        }

        Value sortValue;
        std::map<std::string, std::multimap<Value, size_t>> validRows;
        for (auto& table : tableMap) {
            for (const auto& rowAndBlock : table.second.getMap()) {
                if (rowAndBlock.first > 0) {
                    Row aRow{};
                    aRow.setSchema(schemasRequired[table.first]);
                    aStorage.load(aRow, rowAndBlock.second);
                    if (Validation::validRow(aQuery, aRow)) {
                        if (!orderByColumn.empty()) {
                            for (const auto &pair : aRow.getData()) {
                                if (pair.first == orderByColumn) {
                                    sortValue = pair.second;
                                }
                            }
                        } else {
                            sortValue = static_cast<int>(aRow.entityId);
                        }
                        validRows[table.first].insert({sortValue, rowAndBlock.second});
                    }
                }
            }
        }

        if (!aQuery.getJoin().empty()) {
            content = ProcessorHelpers::handleJoin(aQuery, schemasRequired, headers, validRows);
            if (content.empty()) {
                failedOperation(aView, aViewer);
                return;
            }

        } else {
            for (auto &table : tableMap) {
                for (const auto &goodIndex : validRows[table.first]) {
                    Row aRow;
                    aRow.setSchema(schemasRequired[table.first]);
                    aStorage.load(aRow, goodIndex.second);
                    StringVector rowContent;
                    if (std::find(headers[table.first].begin(), headers[table.first].end(), "id") != headers[table.first].end()) {
                        rowContent.push_back(std::to_string(aRow.entityId));
                    }

                    auto rowData = aRow.getData();
                    for (const auto& headerName : headers[table.first]) {
                        if (rowData.find(headerName) != rowData.end()) {
                            rowContent.push_back(Helpers::variantToString(aRow.getData()[headerName]));
                        } else if ("id" == headerName) {
                            continue;
                        } else {
                            rowContent.push_back("");
                        }

                    }

//                    for (const auto &pair : aRow.getData()) {
//                        if (std::find(headers[table.first].begin(), headers[table.first].end(), pair.first) != headers[table.first].end()) {
//                            rowContent.push_back(variantToString(pair.second));
//                        }
//                    }
                    content.push_back(rowContent);
                }
            }
        }


        // Check for descending order
        if (aQuery.descending) {
            std::reverse(content.begin(), content.end());
        }

        // Apply limit
        int numrows = aQuery.getLimit();
        std::vector<std::vector<std::string>> limitContent;
        if (aQuery.getIfLimit()) {
            limitContent.insert(limitContent.end(), content.begin(), content.begin() + numrows);
        } else {
            limitContent = content;
        }

        // Remove all rows containing elements with value "0"
        limitContent.erase(
                std::remove_if(limitContent.begin(), limitContent.end(), [](const std::vector<std::string> &row) {
                    return std::any_of(row.begin(), row.end(), [](const std::string &element) {
                        return element == "0";
                    });
                }),
                limitContent.end()
        );

        if (limitContent.empty()) {
            aView.setSuccess(true);
            aView.setCount(0);
            aViewer(aView);
            return;
        }
        if (!aQuery.getAsField().empty()) {
            aView.setHeaders({"Count(" + aQuery.getAsField() + ")"});
        } else if (!aQuery.getCount().empty()) {
            aView.setHeaders({"Count(" + aQuery.getCount() + ")"});
        } else if (!aQuery.getJoin().empty()) {
            aView.setHeaders(aQuery.getSelectFields());
        } else {
            aView.setHeaders(headers[headers.begin()->first]);
        }

        if (aQuery.getCount().empty()) {
            aView.setData({{std::to_string(limitContent.size())}});
        } else {
            aView.setData(limitContent);
        }
        aViewer(aView);
    }

    void RowProcessor::updateRow(DBQuery &aQuery, ViewListener &aViewer) {
        TableView2 aView{};
        Validation aValidator;
        std::string tableName = aQuery.getTableName();
        std::string setField = aQuery.getSetField();
        Value val;
        std::string databaseInstance = DatabaseStorageEngine::getInstance().getLiveDatabase();
        Storage aStorage{databaseInstance, OpenFile()};
        size_t count = 0;

        size_t tableIndex = ProcessorHelpers::getAndVerifyTable(tableName);
        if (-1 == tableIndex) {
            failedOperation(aView, aViewer);
            return;
        }

        TableTOC aTableTOC;
        aStorage.load(aTableTOC, tableIndex);
        Schema aSchema;
        uint32_t schemaBlock = aTableTOC.findSchema();
        aStorage.load(aSchema, schemaBlock);
        if (!aValidator.validateDBQuery(aQuery, aSchema)) {
            failedOperation(aView,aViewer);
            return;
        }

        switch (aSchema.findAttributeByName(setField)->getType()) {
            case DataTypes::bool_type:
                aQuery.getSetValue() == "true" ? val = true : val = false;
                break;
            case DataTypes::float_type:
                val = std::stof(aQuery.getSetValue());
                break;
            case DataTypes::int_type:
                val = std::stoi(aQuery.getSetValue());
                break;
            case DataTypes::varchar_type:
                val = aQuery.getSetValue();
                break;
            default:
                break;
        }

        for (const auto &rowAndBlock : aTableTOC.getMap()) {
            if (rowAndBlock.first > 0) {
                Row aRow;
                Row cleanRow;
                aRow.setSchema(aSchema);
                aStorage.load(aRow, rowAndBlock.second);
                if (!aQuery.getWhereExpressions().empty()) {
                    if (aValidator.validRow(aQuery, aRow)) {
                        aRow.set(setField, val);
                        aStorage.save(cleanRow, rowAndBlock.second);
                        aStorage.save(aRow, rowAndBlock.second);
                        count++;
                    }
                } else {
                    aRow.setField(setField, val);
                    aStorage.save(cleanRow, rowAndBlock.second);
                    aStorage.save(aRow, rowAndBlock.second);
                    count++;
                }
            }
        }
        aView.setCount(count);
        aView.setSuccess(true);
        aViewer(aView);
    }

    void RowProcessor::deleteRow(ECE141::DBQuery &aQuery, ECE141::ViewListener &aViewer) {
        TableView2 aView{};
        Validation aValidator;
        std::string tableName = aQuery.getTableName();
        std::string databaseInstance = DatabaseStorageEngine::getInstance().getLiveDatabase();
        Storage aStorage{databaseInstance, OpenFile()};
        size_t count = 0;
        TableTOC aTableTOC;
        Schema aSchema;
        Block aBlock;

        size_t tableIndex = ProcessorHelpers::getAndVerifyTable(tableName);
        if (-1 == tableIndex) {
            failedOperation(aView, aViewer);
            return;
        }

        aStorage.load(aTableTOC, tableIndex);
        uint32_t schemaBlock = aTableTOC.findSchema();
        aStorage.load(aSchema, schemaBlock);
        if (!Validation::validateDBQuery(aQuery, aSchema)) {
            failedOperation(aView,aViewer);
            return;
        }

        for (const auto &rowAndBlock : aTableTOC.getMap()) {
            if (rowAndBlock.first > 0) {
                Row aRow;
                aRow.setSchema(aSchema);
                aStorage.load(aRow, rowAndBlock.second);
                if (!aQuery.getWhereExpressions().empty()) {
                    if (aValidator.validRow(aQuery, aRow)) {
                        aStorage.save(aRow, rowAndBlock.second, true);
                        aTableTOC.removeRow(rowAndBlock.first);
                        count++;
                    }
                } else {
                    aStorage.save(aRow, rowAndBlock.second, true);
                    aTableTOC.removeRow(rowAndBlock.first);
                    count++;
                }
            }
        }

        aView.setCount(count);
        aView.setSuccess(true);
        aViewer(aView);
    }

    StatusResult RowProcessor::run(CommandStatement &aStatement, ViewListener &aViewer) {
        if (aStatement.statement.size() >= 1) {
            if (Keywords::insert_kw == aStatement.statement.at(0).keyword) {
                insertRow(aStatement.statement.at(2).data, aStatement.attributeList, aStatement.values, aViewer);
            } else if (Keywords::select_kw == aStatement.statement.at(0).keyword) {
                selectRow(aStatement.aQuery, aViewer);
            } else if (Keywords::update_kw == aStatement.statement.at(0).keyword) {
                updateRow(aStatement.aQuery, aViewer);
            } else if (Keywords::delete_kw == aStatement.statement.at(0).keyword) {
                deleteRow(aStatement.aQuery, aViewer);
            }
        }
        return Errors::noError;
    }

}
