#ifndef ECE141DB_TOKENSEQUENCER_HPP
#define ECE141DB_TOKENSEQUENCER_HPP

#include "tokenizer/Tokenizer.hpp"
#include "tokenizer/keywords.hpp"
#include "parsing/CommandStatement.hpp"
#include <initializer_list>
#include <stack>
#include <vector>

namespace ECE141 {
    class StringFunctions {
    public:
        static Value stringToValue(const std::string &aString) {
            auto lowerStr = toLower(aString);

            if (isBool(aString)) {
                return lowerStr == "true";
            }
            if (isNumeric<int>(aString)) {
                return std::stoi(aString);
            }
            if (isNumeric<float>(aString)) {
                return std::stof(aString);
            }
            return aString;
        }

        static std::string toLower(const std::string &str) {
            std::string lowerStr = str;
            std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
            return lowerStr;
        }

        static bool isBool(const std::string &str) {
            auto lowerStr = toLower(str);
            return lowerStr == "true" || lowerStr == "false";
        }

        template<typename T>
        static bool isNumeric(const std::string &str) {
            std::istringstream iss(str);
            T value;
            iss >> std::noskipws >> value;
            return iss.eof() && !iss.fail();
        }
    };

    class TokenSequencer {
    public:

        TokenSequencer(Tokenizer &aTokenizer)
                : tokenizer(aTokenizer), state{true}, index{0} {}

        struct SeqState {
            bool state;
            TokenSequencer &seq;
        };

        bool next() {
            if (tokenizer.more()) {
                ++index;
                return true;
            }
            return false;
        }

        bool nextToken() {
            if (tokenizer.more()) {
                tokenizer.next();
                ++index;
                return true;
            }
            return false;
        }

        Token current() {
            return tokenizer.peekStart(index);
        }

        TokenSequencer &is(const KWList &aList) {
            for (auto keyword : aList) {
                if (tokenizer.peekStart(index).keyword == keyword) {
                    return *this;
                }
            }
            state = false;
            return *this;
        }

        TokenSequencer &is(const TTList &aList) {
            for (auto type : aList) {
                if (tokenizer.peekStart(index).type == type) {
                    return *this;
                }
            }
            state = false;
            return *this;
        }

        TokenSequencer &skipIf(const KWList &aList) {
            for (auto keyword : aList) {
                if (tokenizer.peek(index).keyword == keyword) {
                    next();
                    return *this;
                }
            }
            state = false;
            return *this;
        }

        TokenSequencer &tokenizerSkipIf(const KWList &aList) {
            for (auto keyword : aList) {
                if (tokenizer.peekStart(index).keyword == keyword) {
                    next();
                    tokenizer.next();
                    return *this;
                }
            }
            state = false;
            return *this;
        }

        TokenSequencer &tokenizerSkipIf(const std::string &ch) {
            if (tokenizer.peekStart(index).data == ch) {
                next();
                tokenizer.next();
                return *this;
            } else state = false;
            return *this;
        }

        TokenSequencer &skipIf(const std::string &ch) {
            if (tokenizer.peekStart(index).data == ch) {
                next();
                return *this;
            } else state = false;
            return *this;
        }

        TokenSequencer &consumeIfKeyword(const KWList &aList, CommandStatement &aStatement) {
            for (auto keyword : aList) {
                if (tokenizer.peekStart(index).keyword == keyword) {
                    aStatement.statement.push_back(tokenizer.current());
                    nextToken();
                    return *this;
                }
            }
            state = false;
            return *this;
        }
        static bool isSemicolon(const Token& aToken){
            return ";" == aToken.data;
        }

        TokenSequencer &consumeIntoVector(const KWList &aList, std::vector<Token> &aVector) {
            for (auto keyword : aList) {
                if (tokenizer.peekStart(index).keyword == keyword) {
                    aVector.push_back(tokenizer.current());
                    nextToken();
                } else {
                    state = false;
                    break;
                }
            }
            return *this;
        }

        TokenSequencer &changeTableName(const KWList &aList, CommandStatement &aStatement) {
            for (auto keyword : aList) {
                if (tokenizer.peekStart(index).keyword == keyword) {
                    aStatement.aQuery.changeTableName(current().data);
                    nextToken();
                    return *this;
                }
            }
            state = false;
            return *this;
        }

        bool trackParenthesis(std::stack<std::string> &parenStack) {
            if (current().data == "(") {
                parenStack.push(current().data);
            } else if (current().data == ")") {
                if (parenStack.empty()) {
                    return false;
                }
                parenStack.pop();
            }
            return true;
        }

        bool parseJoinColumns(CommandStatement& aStatement){
            std::string attribute1;
            std::string attribute2;
            std::string table1 = aStatement.aQuery.getTableName();
            std::string table2 = aStatement.aQuery.getJoin().table;
            while(current().data != ";" && current().type==TokenType::identifier){
                skipIf(table1);
                skipIf(table2);
                skipIf(".");
                attribute1 = current().data;
                nextToken();
                skipIf("=");
                skipIf(table1);
                skipIf(table2);
                skipIf(".");
                attribute2 = current().data;
                aStatement.aQuery.addJoinPair(table1, attribute1, table2, attribute2);
                nextToken();
                skipIf(",");
            }
            return true;
        }

        bool parseAttributeOrValue(std::vector<Token> &aRow, std::stack<std::string> &parenStack, StatusResult &aResult) {
            while (current().data != "," && (current().data != ")" || !parenStack.empty())) {
                if (!trackParenthesis(parenStack)) {
                    aResult = Errors::syntaxError;
                    return false;
                }
                if (current().data == ";") {
                    if (!parenStack.empty()) {
                        aResult = Errors::syntaxError;
                        return false;
                    } else {
                        break;
                    }
                }
                aRow.push_back(current());
                if (!nextToken()) break;
            }
            return true;
        }

        bool makeExpression(Expression &anExpression, StatusResult &aResult) {
            if (is({Keywords::identifier_kw})) {
                anExpression.lhs = Operand(current().data, current().type, current().data);
                nextToken();
                if (current().type == TokenType::operators) {
                    anExpression.op = gExpressionOps[current().data];
                    nextToken();
                    if (is({TokenType::number, TokenType::string, TokenType::identifier, TokenType::timedate})) {
                        anExpression.rhs = Operand(current().data, current().type, StringFunctions::stringToValue(current().data));
                        nextToken();
                        return true;
                    }
                }
            }
            aResult = StatusResult{Errors::syntaxError};
            return false;
        }

        operator bool() { return state; }

        TokenSequencer &clear() {
            state = true;
            return *this;
        }

        bool more() {
            return index < tokenizer.size();
        }

        bool resetTokenizer() {
            tokenizer.restart();
            return true;
        }

        TokenSequencer &reset() {
            state = true;
            index = 0;
            return *this;
        }

        TokenSequencer &resetState() {
            state = true;
            return *this;
        }

        Tokenizer &tokenizer;
        bool state;
        int index;
    };

}

#endif //ECE141DB_TOKENSEQUENCER_HPP
