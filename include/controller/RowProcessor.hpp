//
// Created by Matthew Dacosta on 5/2/24.
//

#ifndef ECE141DB_ROWPROCESSOR_HPP
#define ECE141DB_ROWPROCESSOR_HPP

#include "CommandProcessor.hpp"
#include "database/TOC.hpp"
#include "parsing/CommandStatement.hpp"
#include "misc/DBQuery.hpp"
#include <algorithm>
#include "string"
//#include "database/Row.hpp"

namespace ECE141 {

    class RowProcessor : public CommandProcessor {

        CommandProcessor *recognises(Tokenizer &aTokenizer) override;

        ECE41::SharedStatement makeStatement(Tokenizer &aTokenizer, StatusResult &aResult) override;

        static void insertRow(std::string& tableName, std::vector<ECE141::Attribute>& attributeList, std::vector<std::vector<AttributeValuePairs>>& values, ViewListener &aViewer);

        static void selectRow(DBQuery &aQuery, ViewListener &aViewer);

        static void updateRow(DBQuery &aQuery, ViewListener &aViewer);

        static void deleteRow(DBQuery &aQuery, ViewListener &aViewer);

        StatusResult run(CommandStatement &aStatement, ViewListener &aViewer) override;


    public:
        RowProcessor(CommandHandler aHandler = nullptr) : CommandProcessor(aHandler) {}

    };

}

#endif //ECE141DB_ROWPROCESSOR_HPP
