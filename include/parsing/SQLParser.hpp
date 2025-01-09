//
// Created by aryam on 28-04-2024.
//

#ifndef ECE141DB_SQLPARSER_HPP
#define ECE141DB_SQLPARSER_HPP

#include <stdexcept>
#include <cstdio>
#include <map>
#include <string>
#include <utility>
#include <functional>
#include <stack>
#include <memory>
#include <unordered_set>
#include "misc/Config.hpp"
#include "misc/Errors.hpp"
#include "view/View.hpp"
#include "tokenizer/Tokenizer.hpp"
#include "misc/Helpers.hpp"
#include "tokenizer/keywords.hpp"
#include "tokenizer/TokenSequencer.hpp"
#include "CommandStatement.hpp"
#include "misc/DBQuery.hpp"

namespace ECE141 {

    class SQLCommandInterface {
    public:
        virtual bool parse(ECE141::CommandStatement &aStatement, Tokenizer &aTokenizer, StatusResult &aResult) = 0;
        virtual ~SQLCommandInterface() = default;

        static bool hasOnlyLetters(const Token &token) {
            return std::all_of(token.data.begin(), token.data.end(), ::isalpha);
        }


        bool checkTableName(const Token &token) {
            // List of SQL reserved keywords (a small subset for example purposes)
            std::unordered_set<std::string> reservedWords = {
                    "SELECT", "INSERT", "UPDATE", "DELETE", "CREATE", "DROP", "ALTER", "TABLE"
            };

            // Check if the name is empty or starts with a digit
            if (token.data.empty() || std::isdigit(token.data[0])) {
                return false;
            }

            // Check if the name is a reserved word
            if (reservedWords.find(token.data) != reservedWords.end()) {
                return false;
            }

            // Check for invalid characters
            for (char ch : token.data) {
                if (!std::isalnum(ch) && ch != '_') {
                    return false;
                }
            }

            // Passed all checks
            return true;
        }
        static StatusResult validateTableName(const Token &token) {
            if (hasOnlyLetters(token)) {
                return StatusResult{Errors::noError};
            } else {
                return StatusResult{Errors::invalidTableName};
            }
        }
    };

    class SQLCommand : public SQLCommandInterface {
    public:
        virtual ~SQLCommand() = default;

    protected:
        static bool parseWhereClause(CommandStatement &aStatement, StatusResult &aResult, TokenSequencer &aSequencer) {
            Expression aExpression{};
            if (aSequencer.makeExpression(aExpression, aResult)) {
                aStatement.aQuery.addWhereExpression(aExpression);
                if (aSequencer.tokenizerSkipIf({Keywords::and_kw})) {
                    return parseWhereClause(aStatement, aResult, aSequencer);
                }
                return true;
            }
            aResult = StatusResult{Errors::syntaxError};
            return false;
        }

        static bool parseCountClause(CommandStatement &aStatement, StatusResult &aResult, TokenSequencer &aSequencer) {
            if (aSequencer.is({Keywords::count_kw})) {
                aSequencer.skipIf("(");
                aStatement.aQuery.addCountField(aSequencer.current().data);
                return true;
            } else {
                aResult = StatusResult{Errors::syntaxError};
                return false;
            }
        }

        static bool parseAsClause(CommandStatement &aStatement, StatusResult &aResult, TokenSequencer &aSequencer) {
            if (aSequencer.is({Keywords::as_kw})) {
                aStatement.aQuery.addAsField(aSequencer.current().data);
                return true;
            } else {
                aResult = StatusResult{Errors::syntaxError};
                return false;
            }
        }

        static bool parseOrderByClause(CommandStatement &aStatement, StatusResult &aResult, TokenSequencer &aSequencer) {
            if (aSequencer.is({Keywords::identifier_kw})) {
                aStatement.aQuery.addOrderByField(aSequencer.current().data);
                aSequencer.nextToken();
                aStatement.aQuery.descending = aSequencer.is({Keywords::desc_kw});
                return true;
            }
            aResult = StatusResult{Errors::syntaxError};
            return false;
        }

        static bool parseLimitClause(CommandStatement &aStatement, StatusResult &aResult, TokenSequencer &aSequencer) {
            if (aSequencer.is({TokenType::number})) {
                try {
                    aStatement.aQuery.changeLimit(std::stoi(aSequencer.current().data));
                    return true;
                } catch (const std::invalid_argument &) {
                    aResult = StatusResult{Errors::syntaxError};
                }
            }
            return false;
        }

        static bool parseJoinClause(CommandStatement &aStatement, StatusResult &aResult, TokenSequencer &aSequencer) {
            aStatement.aQuery.changeJoinType(aSequencer.current().keyword);
            aSequencer.nextToken();
            if(aSequencer.resetState().tokenizerSkipIf({Keywords::join_kw})){
                aStatement.aQuery.addJoinTable(aSequencer.current().data);
                aSequencer.nextToken();
            }
            aSequencer.resetState().tokenizerSkipIf({Keywords::on_kw});
            if(aSequencer.parseJoinColumns(aStatement)) return true;

            return false;
        }

        static bool parseCommonClauses(CommandStatement &aStatement, TokenSequencer &aSequencer, StatusResult &aResult) {
            while (aSequencer.more() && aSequencer.current().data != ";") {
                if (aSequencer.resetState().tokenizerSkipIf({Keywords::where_kw})) {
                    if (!parseWhereClause(aStatement, aResult, aSequencer)) return false;
                } else if (aSequencer.resetState().tokenizerSkipIf({Keywords::order_kw}).tokenizerSkipIf({Keywords::by_kw})) {
                    if (!parseOrderByClause(aStatement, aResult, aSequencer)) return false;
                } else if (aSequencer.resetState().tokenizerSkipIf({Keywords::limit_kw})) {
                    if (!parseLimitClause(aStatement, aResult, aSequencer)) return false;
                } else if (aSequencer.resetState().tokenizerSkipIf({Keywords::count_kw})) {
                    if (!parseCountClause(aStatement, aResult, aSequencer)) return false;
                } else if (aSequencer.resetState().tokenizerSkipIf({Keywords::as_kw})) {
                    if (!parseAsClause(aStatement, aResult, aSequencer)) return false;
                } else if(aSequencer.resetState().is({Keywords::left_kw, Keywords::right_kw, Keywords::inner_kw, Keywords::outer_kw, Keywords::full_kw, Keywords::self_kw})) {
                    if(!parseJoinClause(aStatement, aResult, aSequencer)) return false;
                } else {
                    aSequencer.nextToken();
                }
            }
            return true;
        }
    };

    class DeleteCommand : public SQLCommand {
    public:
        static bool parseExecutionStatement(CommandStatement &aStatement, Tokenizer &tokenizer, StatusResult &aResult,
                                            TokenSequencer &aSequencer) {
            aStatement.aQuery = DBQuery();
            if (aSequencer.consumeIntoVector({Keywords::delete_kw},aStatement.statement)) {
                if (aSequencer.tokenizerSkipIf({Keywords::from_kw})) {
                    if (aSequencer.changeTableName({Keywords::identifier_kw}, aStatement)) {
                        if (!parseCommonClauses(aStatement, aSequencer, aResult)) return false;
                    }
                } else {
                    aResult = Errors::syntaxError;
                    return false;
                }
            } else {
                aResult = Errors::syntaxError;
                return false;
            }
            aSequencer.nextToken();
            return true;
        }

        bool parse(ECE141::CommandStatement &aStatement, Tokenizer &aTokenizer, StatusResult &aResult) override {
            TokenSequencer aSequencer(aTokenizer);
            if (!parseExecutionStatement(aStatement, aTokenizer, aResult, aSequencer)) return false;
            return true;
        }
    };

    class UpdateCommand : public SQLCommand {
    public:
        static bool parseExecutionStatement(CommandStatement &aStatement, Tokenizer &tokenizer, StatusResult &aResult,
                                            TokenSequencer &aSequencer) {
            aStatement.aQuery = DBQuery();
            if (aSequencer.consumeIntoVector({Keywords::update_kw},aStatement.statement)) {
                if (aSequencer.changeTableName({Keywords::identifier_kw},aStatement)) {
                    if (aSequencer.tokenizerSkipIf({Keywords::set_kw})) {
                        aStatement.aQuery.changeSetField(aSequencer.current().data);
                        aSequencer.nextToken();
                        if (aSequencer.current().type == TokenType::operators) {
                            aSequencer.nextToken();
                            aStatement.aQuery.changeSetValue(aSequencer.current().data);
                            aSequencer.nextToken();
                            if (!parseCommonClauses(aStatement, aSequencer, aResult)) return false;
                        } else {
                            aResult = Errors::syntaxError;
                            return false;
                        }
                    } else {
                        aResult = Errors::syntaxError;
                        return false;
                    }
                } else {
                    aResult = Errors::syntaxError;
                    return false;
                }
            } else {
                aResult = Errors::syntaxError;
                return false;
            }
            return true;
        }

        bool parse(ECE141::CommandStatement &aStatement, Tokenizer &aTokenizer, StatusResult &aResult) override {
            TokenSequencer aSequencer(aTokenizer);
            return parseExecutionStatement(aStatement, aTokenizer, aResult, aSequencer);
        }
    };

    class SelectCommand : public SQLCommand {
    public:
        static bool getTableFieldNames(CommandStatement &aStatement, StatusResult &aResult, TokenSequencer &aSequencer) {
            if (aSequencer.tokenizerSkipIf("*")) {
                aStatement.aQuery.changeAllFields(true);
            } else {
                aStatement.aQuery.changeAllFields(false);
                while (!aSequencer.is({Keywords::from_kw})) {
                    if (aSequencer.resetState().is({Keywords::identifier_kw})) {
                        aStatement.aQuery.addSelectField(aSequencer.tokenizer.current().data);
                        aSequencer.nextToken();
                    } else if (aSequencer.current().data == ",") {
                        aSequencer.nextToken();
                    } else {
                        break;
                    }
                }
            }
            return true;
        }

        static bool parseExecutionStatement(CommandStatement &aStatement, Tokenizer &tokenizer, StatusResult &aResult, TokenSequencer &aSequencer) {
            aStatement.aQuery = DBQuery();
            if (aSequencer.consumeIntoVector({Keywords::select_kw},aStatement.statement)) {
                getTableFieldNames(aStatement, aResult, aSequencer);
                if (aSequencer.resetState().tokenizerSkipIf({Keywords::from_kw})) {
                    if(!aSequencer.changeTableName({Keywords::identifier_kw},aStatement)) return false;
                    if (!parseCommonClauses(aStatement, aSequencer, aResult)) return false;
                } else {
                    aResult = Errors::syntaxError;
                    return false;
                }
            } else {
                aResult = Errors::syntaxError;
                return false;
            }
            return true;
        }

        bool parse(ECE141::CommandStatement &aStatement, Tokenizer &aTokenizer, StatusResult &aResult) override {
            TokenSequencer aSequencer(aTokenizer);
            return parseExecutionStatement(aStatement, aTokenizer, aResult, aSequencer);
        }
    };

    class BasicCommand : public SQLCommandInterface {
    public:
        bool parse(ECE141::CommandStatement &aStatement, Tokenizer &aTokenizer, StatusResult &aResult) override {
            TokenSequencer aSequencer(aTokenizer);
            if (aSequencer.consumeIntoVector({Keywords::show_kw, Keywords::tables_kw},aStatement.statement)) {
                return true;
            } else if (aSequencer.reset().consumeIntoVector({Keywords::drop_kw, Keywords::table_kw, Keywords::identifier_kw},aStatement.statement)) {
                if (!validateTableName(aStatement.statement[2])) return false;
                return true;
            } else if (aSequencer.reset().consumeIntoVector({Keywords::describe_kw, Keywords::identifier_kw},aStatement.statement)) {
                if (!validateTableName(aStatement.statement[1])) return false;
                return true;
            } else {
                return false;
            }
        }
    };

    class CreateTableCommand : public SQLCommandInterface {
    public:
        static bool makeAttributeList(std::vector<std::vector<Token>> &attributes, CommandStatement &aStatement, StatusResult &aResult) {
            for (auto &attribute : attributes) {
                Attribute anAttribute;
                if (anAttribute.setAttributes(attribute, aResult)) {
                    aStatement.attributeList.push_back(anAttribute);
                } else {
                    return false;
                }
            }
            return true;
        }

        static bool handleAttributes(TokenSequencer &aSequencer, CommandStatement &aStatement, StatusResult &aResult) {
            std::vector<std::vector<Token>> attributes;
            std::stack<std::string> parenStack;
            std::vector<Token> aRow;

            while (aSequencer.current().data != ")" && aSequencer.more() && aSequencer.current().data != ";") {
                if (!aSequencer.parseAttributeOrValue(aRow, parenStack, aResult)) return false;
                attributes.push_back(aRow);
                if (aSequencer.current().data == ",") {
                    if (!aSequencer.nextToken()) break;
                    aRow.clear();
                }
            }
            if (aSequencer.current().data != ")" || !parenStack.empty()) {
                aResult = Errors::syntaxError;
                return false;
            }
            if (!aSequencer.nextToken() || aSequencer.tokenizer.current().data != ";") {
                aResult = Errors::syntaxError;
                return false;
            }
            return makeAttributeList(attributes, aStatement, aResult);
        }

        bool parseExecutionStatement(CommandStatement &aStatement, Tokenizer &tokenizer, StatusResult &aResult, TokenSequencer &aSequencer) {
            if (aSequencer.skipIf({Keywords::create_kw}).skipIf({Keywords::table_kw}).skipIf({Keywords::identifier_kw}).skipIf("(")) {
                for (int i = 0; i < 3; i++) {
                    aStatement.statement.push_back(tokenizer.current());
                    tokenizer.next();
                }
                tokenizer.next(); // skip "(" in the tokenizer

                if (!checkTableName(aStatement.statement[2])) {
                    aResult = Errors::invalidTableName;
                    return false;
                }
            }
            return true;
        }

        bool parse(CommandStatement &aStatement, Tokenizer &aTokenizer, StatusResult &aResult) override {
            TokenSequencer aSequencer(aTokenizer);
            if (!parseExecutionStatement(aStatement, aTokenizer, aResult, aSequencer)) return false;
            return handleAttributes(aSequencer, aStatement, aResult);
        }
    };


    class InsertCommand : public SQLCommandInterface {
    public:
        bool parseExecutionStatement(CommandStatement &aStatement, Tokenizer &tokenizer, StatusResult &aResult, TokenSequencer &aSequencer) {
            if (aSequencer.skipIf({Keywords::insert_kw}).skipIf({Keywords::into_kw}).skipIf({Keywords::identifier_kw}).skipIf("(")) {
                for (int i = 0; i < 3; i++) {
                    aStatement.statement.push_back(tokenizer.current());
                    tokenizer.next();
                }
                tokenizer.next(); // skip "(" in the tokenizer

                if (!checkTableName(aStatement.statement[2])) {
                    aResult = Errors::invalidTableName;
                    return false;
                }
            } else {
                aResult = Errors::syntaxError;
                return false;
            }
            return true;
        }

        static bool handleAttributes(TokenSequencer &aSequencer, CommandStatement &aStatement, StatusResult &aResult) {
            std::stack<std::string> parenStack;
            while (aSequencer.current().data != ")" && aSequencer.more() && aSequencer.current().data != ";") {
                while (aSequencer.current().data != "," && (aSequencer.current().data != ")" || !parenStack.empty())) {
                    if (!aSequencer.trackParenthesis(parenStack)) {
                        aResult = Errors::syntaxError;
                        return false;
                    }
                    if (aSequencer.current().data == ";") {
                        if (!parenStack.empty()) {
                            aResult = Errors::syntaxError;
                            return false;
                        } else {
                            break;
                        }
                    }
                    Attribute anAttribute;
                    if (!anAttribute.setName(aSequencer.tokenizer.current(), aResult)) {
                        return false;
                    }
                    aStatement.attributeList.push_back(anAttribute);
                    if (!aSequencer.nextToken()) break;
                }
                if (aSequencer.current().data != ")") {
                    if (!aSequencer.nextToken()) break;
                } else {
                    break;
                }
            }

            if (aSequencer.current().data != ")" || !parenStack.empty()) {
                aResult = Errors::syntaxError;
                return false;
            }
            aSequencer.nextToken();
            return true;
        }


        static bool makeValueList(std::vector<std::vector<Token>> &values, CommandStatement &aStatement, StatusResult &aResult) {
            if (values.empty() || aStatement.attributeList.empty()) {
                aResult = Errors::invalidAttribute;
                return false;
            }
            std::vector<AttributeValuePairs> aRow;

            for (const auto &row : values) {
                size_t attributeCount = 0;
                for (const auto &element : row) {
                    AttributeValuePairs aPair;
                    if (attributeCount < aStatement.attributeList.size()) {
                        aPair.anAttribute = aStatement.attributeList[attributeCount];
                        aPair.value = element.data;
                        attributeCount++;
                    } else {
                        break;
                    }
                    aRow.push_back(aPair);
                }
                if (aRow.size() != aStatement.attributeList.size()) {
                    aResult = Errors::invalidCommand;
                    return false;
                }
                aStatement.values.push_back(aRow);
                aRow.clear();
            }
            return true;
        }

        static bool handleValues(TokenSequencer &aSequencer, CommandStatement &aStatement, StatusResult &aResult) {
            std::vector<std::vector<Token>> values;
            std::stack<std::string> parenStack;
            std::vector<Token> aRow;

            if (!aSequencer.is({Keywords::values_kw})) {
                aResult = Errors::invalidCommand;
                return false;
            }
            if (!aSequencer.nextToken()) {
                aResult = Errors::invalidCommand;
                return false;
            }
            while (aSequencer.more() && aSequencer.tokenizer.more() && aSequencer.current().data != ";") {
                while (aSequencer.current().data != ",") {
                    while (aSequencer.current().data != ")" || !parenStack.empty()) {
                        if (aSequencer.current().data == "(") {
                            parenStack.push(aSequencer.current().data);
                            if (!aSequencer.nextToken()) break;
                            continue;
                        }
                        if (aSequencer.current().data == ")") {
                            parenStack.pop();
                            aSequencer.nextToken();
                            break;
                        }
                        if (aSequencer.current().type != TokenType::punctuation) {
                            aRow.push_back(aSequencer.current());
                        }
                        if (!aSequencer.nextToken()) break;
                    }
                    values.push_back(aRow);
                    if (aSequencer.current().data == ",") {
                        if (!aSequencer.nextToken()) break;
                        aRow.clear();
                    } else {
                        break;
                    }
                }
            }

            if (!parenStack.empty()) {
                aResult = Errors::syntaxError;
                return false;
            }
            if (aSequencer.tokenizer.current().data != ";") {
                aResult = Errors::syntaxError;
                return false;
            }
            return makeValueList(values, aStatement, aResult);
        }

        bool parse(CommandStatement &aStatement, Tokenizer &aTokenizer, StatusResult &aResult) override {
            TokenSequencer aSequencer(aTokenizer);
            if (!parseExecutionStatement(aStatement, aTokenizer, aResult, aSequencer)) return false;
            if (!handleAttributes(aSequencer, aStatement, aResult)) return false;
            if (!handleValues(aSequencer, aStatement, aResult)) return false;
            return true;
        }
    };

    class BackupCommand : public SQLCommandInterface {
    public:
        bool parse(ECE141::CommandStatement &aStatement, Tokenizer &aTokenizer, StatusResult &aResult) override {
            TokenSequencer aSequencer(aTokenizer);
            if (aSequencer.consumeIntoVector({Keywords::backup_kw, Keywords::identifier_kw},aStatement.statement)) {
                return true;
            } else {
                return false;
            }
        }
    };

    using sharedSQLCommand = std::shared_ptr<SQLCommandInterface>;

    class SQLCommandFactory {
    public:
        static sharedSQLCommand createCommand(CommandStatement &aStatement, Tokenizer &aTokenizer, StatusResult &aResult) {
            Token &token = aTokenizer.current();
            switch (token.keyword) {
                case Keywords::create_kw:
                    return std::make_shared<CreateTableCommand>();
                case Keywords::insert_kw:
                    return std::make_shared<InsertCommand>();
                case Keywords::select_kw:
                    return std::make_shared<SelectCommand>();
                case Keywords::update_kw:
                    return std::make_shared<UpdateCommand>();
                case Keywords::delete_kw:
                    return std::make_shared<DeleteCommand>();
                default:
                    return std::make_shared<BasicCommand>();
            }
        }
    };

    class SQLParser {
    public:
        SQLParser(CommandStatement &aStatement, Tokenizer &tokenizer, StatusResult &result)
                : statement(aStatement), tokenizer(tokenizer), result(result) {}

        bool parse() {
            sharedSQLCommand aCommand = SQLCommandFactory::createCommand(statement, tokenizer, result);
            if (aCommand) {
                return aCommand->parse(statement, tokenizer, result);
            }
            return false;
        }

    private:
        Tokenizer &tokenizer;
        StatusResult &result;
        CommandStatement &statement;
    };

} // namespace ECE141

#endif //ECE141DB_SQLPARSER_HPP