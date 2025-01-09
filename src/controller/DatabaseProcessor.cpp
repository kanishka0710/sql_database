//
// Created by kanis on 4/29/2024.
//

#include "controller/DatabaseProcessor.hpp"
#include "parsing/CommandStatement.hpp"
#include "database/Row.hpp"
#include "database/TOC.hpp"

namespace ECE141 {

    CommandProcessor* DatabaseProcessor::recognises(Tokenizer &aTokenizer) {
        KWList validList = {Keywords::create_kw, Keywords::drop_kw, Keywords::show_kw, Keywords::use_kw, Keywords::dump_kw, Keywords::backup_kw};
        ECE141::TokenSequencer aSequencer(aTokenizer);
        if (aSequencer.skipIf(validList).skipIf({Keywords::database_kw, Keywords::databases_kw, Keywords::identifier_kw}))
            return this;
        else return nullptr;
    }

    ECE41::SharedStatement DatabaseProcessor::makeStatement(Tokenizer &aTokenizer, StatusResult &aResult) {
        auto aStatement = std::make_shared<CommandStatement>();
        ECE41::DatabaseCommand aCommand(aTokenizer);
        if (!aCommand.createCommand(*aStatement, aResult)) {
            aResult.error = Errors::syntaxError;
            return std::nullopt;
        }
        return aStatement;
    }

    void DatabaseProcessor::createDatabase(const std::string &dbName, ViewListener &aViewer) {
        int count = 0;
        TableView2 aView{};
        Storage aStorage = Storage(dbName, CreateFile());
        if (aStorage.isOpen()) {
            count++;
        }

        DbTOC aDBTOC = DbTOC(dbName);
        aDBTOC.type = 'T';
        aStorage.save(aDBTOC, 0);
        aView.setCount(count);
        aViewer(aView);
    }

    void DatabaseProcessor::dropDatabase(const std::string &dbName, ViewListener &aViewer) {
        TableView2 aView{};
        std::filesystem::path filePath = Config::getDBPath(dbName);
        if (std::filesystem::exists(filePath)) {
            std::filesystem::remove(filePath);
            aView.setCount(0);
            aViewer(aView);
            return;
        }
        else{
            failedOperation(aView, aViewer);
        }
    }

    void DatabaseProcessor::showDatabases(std::vector<std::string> &databaseFiles, ViewListener &aViewer) {
        TableView2 aView{};
        std::vector<std::vector<std::string>> values;
        std::vector<std::string> columns = {"Databases"};
        std::filesystem::path filePath = Config::getStoragePath();
        if (std::filesystem::exists(filePath) && std::filesystem::is_directory(filePath)) {
            for (const auto &entry : std::filesystem::directory_iterator(filePath)) {
                if (entry.path().extension() == ".db") {
                    values.push_back({entry.path().filename().string()});
                }
            }
        }
        aView.setHeaders(columns);
        aView.setData(values);
        aViewer(aView);
    }

    void DatabaseProcessor::useDatabase(const std::string &dbName, ViewListener &aViewer) {
        Timer aTimer = Config::getTimer();
        DatabaseStorageEngine &liveDatabase = DatabaseStorageEngine::getInstance();
        liveDatabase.setLiveDatabase(dbName);
        TextView aView("Database changed (" + std::to_string(aTimer.elapsed()) + " secs)\n");
        aViewer(aView);
    }

    void DatabaseProcessor::dumpDatabase(ViewListener &aViewer, const std::string &dbName) {
        TableView2 aView{};
        std::vector<std::string> columns = {"#", "Type", "Id"};
        Storage aStorage{dbName, OpenFile()};

        std::map<char, std::string> typeMap = {
                {'S', "Schema"},
                {'D', "Data"},
                {'F', "Free"},
                {'U', "Unknown"},
                {'T', "TOC"}
        };

        size_t blockCount = aStorage.getBlockCount();
        Block aBlock{};
        std::vector<std::vector<std::string>> values;
        std::vector<std::string> row;
        for (size_t i = 0; i < blockCount; i++) {
            aStorage.readBlock(i, aBlock);
            row.push_back(std::to_string(i));
            row.push_back(typeMap[aBlock.header.type]);

            if (aBlock.header.type == 'D') {
                Row aRow{};
                aStorage.load(aRow, i);
                row.push_back(std::to_string(aRow.entityId));
                values.push_back(row);
            } else {
                row.push_back(" ");
                values.push_back(row);
            }
            row.clear();
        }
        aView.setHeaders(columns);
        aView.setData(values);
        aViewer(aView);
    }

    void DatabaseProcessor::backupDatabase(const std::string &dbName, ViewListener &aViewer) {
        TableView2 aView{};
        std::string databaseInstance = DatabaseStorageEngine::getInstance().getLiveDatabase();
        Storage aStorage{databaseInstance, OpenFile()};

        std::string filename{Config::getStoragePath() + "/" + dbName + ".sql"};
        std::fstream backupFile{filename, std::ios::in | std::ios::out | std::ios::trunc};
        if (!backupFile.is_open()) {
            failedOperation(aView, aViewer);
            return;
        }

        backupFile << "CREATE DATABASE " << dbName << ";\n";
        backupFile << "USE " << dbName << ";\n";

        DbTOC aDbTOC;
        Schema aSchema;
        TableTOC aTableTOC;
        uint32_t tableBlock;
        uint32_t schemaBlock;

        aStorage.load(aDbTOC, 0);
        auto tableNames = aDbTOC.getTableNames();

        for (auto& tableName : tableNames) {
            backupFile << "CREATE TABLE " << tableName << " (";
            tableBlock = aDbTOC.findTable(tableName);
            aStorage.load(aTableTOC, tableBlock);
            schemaBlock = aTableTOC.findSchema();
            aStorage.load(aSchema, schemaBlock);
            backupFile << aSchema.attributesAsString() << ")\n";
        }

        for (auto& tableName : tableNames) {
            tableBlock = aDbTOC.findTable(tableName);
            aStorage.load(aTableTOC, tableBlock);
            schemaBlock = aTableTOC.findSchema();
            aStorage.load(aSchema, schemaBlock);

            backupFile << "INSERT INTO " << tableName << "(";
            for (auto& it : aSchema.getAllAttributeNames()) {
                backupFile << it << ", ";
            }
            backupFile << ") VALUES ";

            for (const auto &rowAndBlock : aTableTOC.getMap()) {
                backupFile << "(";
                if (rowAndBlock.first > 0) {
                    Row aRow;
                    aRow.setSchema(aSchema);
                    aStorage.load(aRow, rowAndBlock.second);
                    for (auto& val : aRow.getData()) {
                        backupFile << Helpers::variantToString(val.second) << ", ";
                    }
                    backupFile << "), ";
                }
            }
            backupFile << ";\n";
        }
        backupFile.close();
        aViewer(aView);
        return;
    }

    StatusResult DatabaseProcessor::run(CommandStatement &aStatement, ViewListener &aViewer) {
        std::vector<std::string> databaseFiles;
        std::map<std::vector<Keywords>, std::function<void(const std::string &, ViewListener &)>> dispatchMap{
                {{Keywords::create_kw, Keywords::database_kw},   [&](const std::string &str, ViewListener &viewListener) {
                    return createDatabase(str, viewListener);
                }},
                {{Keywords::drop_kw,   Keywords::database_kw},   [&](const std::string &str, ViewListener &viewListener) {
                    return dropDatabase(str, viewListener);
                }},
                {{Keywords::show_kw,   Keywords::databases_kw},  [&](const std::string &, ViewListener &viewListener) {
                    return showDatabases(databaseFiles, viewListener);
                }},
                {{Keywords::use_kw,    Keywords::identifier_kw}, [&](const std::string &str, ViewListener &viewListener) {
                    return useDatabase(str, viewListener);
                }},
                {{Keywords::dump_kw,   Keywords::database_kw},   [&](const std::string &str, ViewListener &viewListener) {
                    return dumpDatabase(viewListener, str);
                }},
                {{Keywords::backup_kw, Keywords::identifier_kw}, [&](const std::string &str, ViewListener &viewListener) {
                    return backupDatabase(str, viewListener);
                }}
        };

        std::vector<Keywords> commandStatement;
        if (aStatement.statement.size() < 2) {
            return Errors::unknownCommand;
        } else if (aStatement.statement.size() >= 2) {
            commandStatement = {aStatement.statement.at(0).keyword,
                                aStatement.statement.at(1).keyword};
        }
        if (dispatchMap.count(commandStatement)) {
            if (aStatement.statement.size() == 3) {
                dispatchMap[commandStatement](aStatement.statement.at(2).data, aViewer);
            } else if (aStatement.statement.size() == 2) {
                if (aStatement.statement.at(1).keyword == Keywords::identifier_kw) {
                    dispatchMap[commandStatement](aStatement.statement.at(1).data, aViewer);
                } else {
                    dispatchMap[commandStatement]("", aViewer);
                }
            }
        } else {
            return Errors::unknownCommand;
        }

        return Errors::noError;
    }
}
