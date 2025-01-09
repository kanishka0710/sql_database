//
// Created by aryam on 21-04-2024.
//

#ifndef ECE141DB_COMMAND_HPP
#define ECE141DB_COMMAND_HPP

#include <stdexcept>
#include <cstdio>
#include <map>
#include <string>
#include <utility>
#include <functional>
#include <stack>
#include "misc/Config.hpp"
#include "misc/Errors.hpp"
#include "view/View.hpp"
#include "tokenizer/Tokenizer.hpp"
#include "misc/Helpers.hpp"
#include "tokenizer/keywords.hpp"
#include "tokenizer/TokenSequencer.hpp"
#include "database/Attribute.hpp"
#include "SQLParser.hpp"
#include "CommandStatement.hpp"

namespace ECE41 {

    using SharedStatement = std::optional<std::shared_ptr<ECE141::CommandStatement>>;

    class Command {
    public:
        Command(ECE141::Tokenizer &aTokenizer) : tokenizer{aTokenizer} {
            tokenizer.restart();
        }

        virtual bool createCommand(ECE141::CommandStatement &aStatement, ECE141::StatusResult &aResult) = 0;

    protected:
        ECE141::Tokenizer &tokenizer;
    };

    class AppCommand : public Command {
    public:
        AppCommand(ECE141::Tokenizer &aTokenizer) : Command(aTokenizer) {}

        bool createCommand(ECE141::CommandStatement &aStatement, ECE141::StatusResult &aResult) override {
            ECE141::KWList validList = {ECE141::Keywords::about_kw, ECE141::Keywords::version_kw,
                                        ECE141::Keywords::quit_kw, ECE141::Keywords::help_kw};
            ECE141::TokenSequencer aSequencer(tokenizer);
            if (aSequencer.is(validList)) {
                aStatement.statement.push_back(aSequencer.current());
                tokenizer.next();
                return true;
            }
            return false;
        }
    };

    class DatabaseCommand : public Command {
    public:
        DatabaseCommand(ECE141::Tokenizer &aTokenizer) : Command(aTokenizer) {}

        bool createCommand(ECE141::CommandStatement &aStatement, ECE141::StatusResult &aResult) override {
            ECE141::KWList validList = {ECE141::Keywords::create_kw, ECE141::Keywords::drop_kw,
                                        ECE141::Keywords::show_kw, ECE141::Keywords::use_kw,
                                        ECE141::Keywords::dump_kw, ECE141::Keywords::backup_kw};
            ECE141::TokenSequencer aSequencer(tokenizer);
            if (aSequencer.skipIf(validList)
                    .skipIf({ECE141::Keywords::database_kw, ECE141::Keywords::databases_kw})
                    .skipIf({ECE141::Keywords::identifier_kw})) {
                for (int i = 0; i < 3; i++) {
                    aStatement.statement.push_back(tokenizer.current());
                    tokenizer.next();
                }
                return true;
            }
            if (aSequencer.reset().skipIf(validList)
                    .skipIf({ECE141::Keywords::database_kw, ECE141::Keywords::databases_kw})) {
                for (int i = 0; i < 2; i++) {
                    aStatement.statement.push_back(tokenizer.current());
                    tokenizer.next();
                }
                return true;
            }
            if (aSequencer.reset().skipIf(validList)
                    .skipIf({ECE141::Keywords::identifier_kw})) {
                for (int i = 0; i < 2; i++) {
                    aStatement.statement.push_back(tokenizer.current());
                    tokenizer.next();
                }
                return true;
            }
            return false;
        }
    };

    class TableCommand : public Command {
    public:
        TableCommand(ECE141::Tokenizer &tokenizer) : Command(tokenizer) {}

        bool createCommand(ECE141::CommandStatement &aStatement, ECE141::StatusResult &aResult) override {
            ECE141::SQLParser aParser(aStatement, tokenizer, aResult);
            return aParser.parse();
        }
    };

}  // namespace ECE41

#endif // ECE141DB_COMMAND_HPP
