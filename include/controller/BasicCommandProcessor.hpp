//
// Created by kanis on 4/29/2024.
//

#ifndef ECE141DB_BASICCOMMANDPROCESSOR_HPP
#define ECE141DB_BASICCOMMANDPROCESSOR_HPP


#include "CommandProcessor.hpp"
#include "parsing/CommandStatement.hpp"

namespace ECE141 {

    class BasicCommandProcessor : public CommandProcessor {

    public:

        std::vector<std::string> helpDescriptions = {
                std::string{"insert: insert into {table-name} (attribute-name [optional comma]..more attribute-names separated"
                            " by commas) value (value [optional comma]..more values)"},
                std::string{"create table: create table {table-name} (attribute [optional comma]..more attributes separated by"
                            " commas...); create a table in the current database"},
                std::string{"show tables: shows all tables in the current database"},
                std::string{"drop table: drop table {table-name}; removes the table if it exists"},
                std::string{"describe table: describe {table-name}; shows the info for the table requested"},
                std::string{"create database: create database {name}; make a new database in the storage folder"},
                std::string{"drop database: drop database {name}; delete a known database from the storage folder"},
                std::string{"show databases: list databases in the storage folder"},
                std::string{"use: use database {name}; load a known database for use"},
                std::string{"about: Display the authors of the program, usage: about;"},
                std::string{"version: Display the version of the program, usage: version;"},
                std::string{"quit: Quit the program, usage: quit;"},
                std::string{"help: Display this list of commands, usage: help;"},
                std::string{"dump database: dump database {name}; dump the contents of the database to a file"},
                std::string{"select: select {attributes} from {table-name} [where condition]; retrieve data from the table"},
                std::string{"update: update {table-name} set {attribute=value} [where condition]; update data in the table"},
                std::string{"delete: delete from {table-name} [where condition]; delete data from the table"}
        };



        BasicCommandProcessor(CommandHandler aHandler = nullptr) : CommandProcessor(aHandler) {}

        CommandProcessor *recognises(Tokenizer &aTokenizer) override;

        ECE41::SharedStatement makeStatement(Tokenizer &aTokenizer, StatusResult &aResult) override;

        static std::string handleAbout();

        static std::string handleVersion();

        static std::string handleQuit();

        std::string handleHelp();

        StatusResult run(CommandStatement &aStatement, ViewListener &aViewer) override;

    };
}

#endif //ECE141DB_BASICCOMMANDPROCESSOR_HPP
