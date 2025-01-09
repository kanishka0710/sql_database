//
// Created by kanis on 4/29/2024.
//

#ifndef ECE141DB_TABLEPROCESSOR_HPP
#define ECE141DB_TABLEPROCESSOR_HPP

#include "CommandProcessor.hpp"
#include "database/TOC.hpp"
#include "parsing/CommandStatement.hpp"
//#include "database/Row.hpp"

namespace ECE141 {

    class TableProcessor : public CommandProcessor {

        CommandProcessor *recognises(Tokenizer &aTokenizer) override;

        ECE41::SharedStatement makeStatement(Tokenizer &aTokenizer, StatusResult &aResult) override;

        static void createTable(std::string& tableName, std::vector<ECE141::Attribute>& aList, ViewListener &aViewer);

        static void showTables(ViewListener &aViewer);

        static void dropTable(std::string& tableName, ViewListener &aViewer);

        static void describeTable(const std::string& tableName, ViewListener &aViewer);

        StatusResult run(CommandStatement &aStatement, ViewListener &aViewer) override;

    public:
        TableProcessor(CommandHandler aHandler = nullptr) : CommandProcessor(aHandler) {}

    };

}

#endif //ECE141DB_TABLEPROCESSOR_HPP