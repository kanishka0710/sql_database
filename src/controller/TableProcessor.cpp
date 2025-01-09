//
// Created by kanis on 4/29/2024.
//

#include "controller/TableProcessor.hpp"
#include "parsing/CommandStatement.hpp"


namespace ECE141 {

    CommandProcessor *TableProcessor::recognises(Tokenizer &aTokenizer) {
        KWList validList = {Keywords::create_kw, Keywords::drop_kw, Keywords::describe_kw, Keywords::show_kw};
        ECE141::TokenSequencer aSequencer(aTokenizer);
        if (aSequencer.skipIf(validList).skipIf(
                {Keywords::table_kw, Keywords::tables_kw, Keywords::identifier_kw}))
            return this;
        else {
            return nullptr;
        }
    }

    ECE41::SharedStatement TableProcessor::makeStatement(Tokenizer &aTokenizer, StatusResult &aResult) {
        auto aStatement = std::make_shared<CommandStatement>();
        ECE41::TableCommand aCommand(aTokenizer);
        if (!aCommand.createCommand(*aStatement, aResult)) {
            //aResult.error=Errors::syntaxError;
            return std::nullopt;
        }
        return aStatement;
    }

    void TableProcessor::createTable(std::string &tableName, std::vector<ECE141::Attribute> &aList, ViewListener &aViewer) {
        // create the schema object, save that to a database, and return query ok
        TableView2 aView{};
        Schema aSchema = Schema(tableName);

        std::vector<Attribute> aAttributeList;
        StatusResult aResult;
        for (const auto &attribute: aList) {
            aAttributeList.push_back(attribute);
        }
        aSchema.setAttributes(aAttributeList);

        std::string databaseInstance = DatabaseStorageEngine::getInstance().getLiveDatabase();
        Storage aStorage{databaseInstance, OpenFile()};

        DbTOC adbToc;
        aStorage.load(adbToc, 0);

        size_t blockCount = aStorage.getFreeBlock();
        adbToc.addTable(tableName, blockCount);

        TableTOC aTableToc(tableName);
        aTableToc.addRow(blockCount+1);

        aStorage.save(adbToc, 0);
        aStorage.save(aTableToc, blockCount);
        aStorage.save(aSchema, blockCount+1);
        aView.setCount(1);
        aViewer(aView);
    }

    void TableProcessor::showTables(ViewListener &aViewer) {
        TableView2 aView2{};
        std::vector<std::vector<std::string>> values;
        std::vector<std::string> row;
        std::vector<std::string> columns;
        std::string databaseInstance = DatabaseStorageEngine::getInstance().getLiveDatabase();
        columns.push_back("Tables in: " + databaseInstance);
        Storage aStorage{databaseInstance, OpenFile()};
        DbTOC aDbTOC;
        aStorage.load(aDbTOC, 0);
        auto tableNames = aDbTOC.getTableNames();
        for (auto &name: tableNames) {
            row.push_back(name);
            values.push_back(row);
            row.clear();
        }
        
        aView2.setHeaders(columns);
        aView2.setData(values);
        aView2.setIfTable(true);
        aViewer(aView2);
    }

    void TableProcessor::dropTable(std::string &tableName, ViewListener &aViewer) {
        TableView2 aView2{};
        std::string databaseInstance = DatabaseStorageEngine::getInstance().getLiveDatabase();
        DbTOC aDbTOC = DbTOC(databaseInstance);
        TableTOC aTableTOC;
        Schema aSchema;
        size_t count = 0;

        Storage aStorage{databaseInstance, OpenFile()};
        // Check if the table exists in the TOC
        aStorage.load(aDbTOC, 0);
        size_t tableIndex = aDbTOC.findTable(tableName);
        if (!tableIndex || aDbTOC.getTOClength() == 0) {
            return;
        }
        aStorage.load(aTableTOC, tableIndex);
        uint32_t schemaBlock = aTableTOC.findSchema();
        aStorage.load(aSchema, schemaBlock);

        for (const auto& rowAndBlock : aTableTOC.getMap()) {
            if (rowAndBlock.first > 0) {
                Row aRow;
                aRow.setSchema(aSchema);
                aStorage.load(aRow, rowAndBlock.second);
                aStorage.save(aRow, rowAndBlock.second, true);
                count++;
            }
        }
        aStorage.save(aSchema, schemaBlock, true);
        aStorage.save(aTableTOC, tableIndex, true);
        aDbTOC.removeTable(tableName);
        aStorage.save(aDbTOC, 0);
        aView2.setCount(0);
        aViewer(aView2);
    }

    void TableProcessor::describeTable(const std::string &tableName, ViewListener &aViewer) {
        TableView2 aView2{};
        std::vector<std::vector<std::string>> values;
        std::vector<std::string> row;
        std::vector<std::string> columns={"Field", "Type", "Null", "Key", "Default", "Extra"};

        std::string databaseInstance = DatabaseStorageEngine::getInstance().getLiveDatabase();
        Storage aStorage{databaseInstance, OpenFile()};

        std::map<DataTypes, std::string> dataTypeMap = {
                {DataTypes::no_type,       "none"},
                {DataTypes::bool_type,     "boolean"},
                {DataTypes::datetime_type, "datetime"},
                {DataTypes::float_type,    "float"},
                {DataTypes::int_type,      "int"},
                {DataTypes::varchar_type,  "varchar"}
        };
        std::map<bool, std::string> yesNoMap = {
                {false, "NO"},
                {true,  "YES"}
        };

        DbTOC aDbTOC = DbTOC(databaseInstance);
        aStorage.load(aDbTOC, 0);
        auto tableNames = aDbTOC.getTableNames();
        Schema aSchema;
        TableTOC aTableTOC;
        uint32_t tableBlock;
        uint32_t schemaBlock;

        for (auto& name : tableNames) {
            tableBlock = aDbTOC.findTable(tableName);
            aStorage.load(aTableTOC, tableBlock);
            schemaBlock = aTableTOC.findSchema();
            aStorage.load(aSchema, schemaBlock);
            if (aSchema.getName() == tableName) {
                for (auto &it: aSchema.getAttributeList()) {
                    row.push_back(it.getName());
                    row.push_back(dataTypeMap[it.getType()]);
                    row.push_back(yesNoMap[it.isNullable()]);
                    row.push_back(yesNoMap[it.isPrimaryKey()]);
                    row.push_back(it.getDefaultValue());
                    row.push_back(yesNoMap[it.isAutoIncrement()]);
                    values.push_back(row);
                    row.clear();
                }
                break;
            }
        }
        aView2.setIfTable(true);
        aView2.setData(values);
        aView2.setHeaders(columns);
        aViewer(aView2);
    }

    StatusResult TableProcessor::run(CommandStatement &aStatement, ViewListener &aViewer) {

        if (aStatement.statement.size() > 1) {
            if (Keywords::create_kw == aStatement.statement.at(0).keyword) {
                createTable(aStatement.statement.at(2).data, aStatement.attributeList, aViewer);
            } else if (Keywords::show_kw == aStatement.statement.at(0).keyword) {
                showTables(aViewer);
            } else if (Keywords::describe_kw == aStatement.statement.at(0).keyword) {
                describeTable(aStatement.statement.at(1).data, aViewer);
            } else if (Keywords::drop_kw == aStatement.statement.at(0).keyword) {
                dropTable(aStatement.statement.at(2).data, aViewer);
            }
        }
        return Errors::noError;
    }

}