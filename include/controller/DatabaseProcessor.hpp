//
// Created by kanis on 4/29/2024.
//

#ifndef ECE141DB_DATABASEPROCESSOR_HPP
#define ECE141DB_DATABASEPROCESSOR_HPP


#include "CommandProcessor.hpp"
#include "parsing/CommandStatement.hpp"
#include <filesystem>


namespace ECE141 {
    class DatabaseProcessor : public CommandProcessor {

    public:

        DatabaseProcessor(CommandHandler aHandler = nullptr) : CommandProcessor(aHandler) {}


        CommandProcessor *recognises(Tokenizer &aTokenizer) override;

        ECE41::SharedStatement makeStatement(Tokenizer &aTokenizer, StatusResult &aResult) override;

        static void showDatabases(std::vector<std::string> &databaseFiles, ViewListener &aViewer);

        static void dumpDatabase(ViewListener &aViewer, const std::string& string);

        StatusResult run(CommandStatement &aStatement, ViewListener &aViewer) override;

        void createDatabase(const std::string &dbName, ViewListener &aViewer);

        void dropDatabase(const std::string &dbName, ViewListener &aViewer);

        void useDatabase(const std::string &dbName, ViewListener &aViewer);

        void backupDatabase(const std::string &dbName, ViewListener &aViewer);
    };
}


#endif //ECE141DB_DATABASEPROCESSOR_HPP
