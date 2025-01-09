//
// Created by aryam on 11-04-2024.
//

#ifndef ECE141DB_COMMANDPROCESSOR_HPP
#define ECE141DB_COMMANDPROCESSOR_HPP

#include <cstdio>
#include <map>
#include <string>
#include <utility>
#include <functional>
#include <fstream>
#include "misc/Config.hpp"
#include "misc/Errors.hpp"
#include "view/View.hpp"
#include "tokenizer/Tokenizer.hpp"
#include "misc/Helpers.hpp"
#include "storage/BlockIO.hpp"
#include "tokenizer/keywords.hpp"
#include "database/Database.hpp"
#include "tokenizer/TokenSequencer.hpp"
#include "parsing/Command.hpp"
#include "database/Attribute.hpp"
#include "database/Schema.hpp"
#include "database/TOC.hpp"
#include "storage/DatabaseStorageEngine.hpp"
#include "database/Row.hpp"

namespace ECE141 {

    class CommandProcessor : public std::enable_shared_from_this<CommandProcessor> {
    public:
        using CommandHandler = std::shared_ptr<CommandProcessor>;

        CommandProcessor(CommandHandler aHandler = nullptr) : nextHandler(aHandler){}

        virtual ~CommandProcessor() = default;

        virtual CommandHandler findHandler(Tokenizer &aTokenizer) {
            if (recognises(aTokenizer)) {
                return shared_from_this();
            } else if (nextHandler) {
                return nextHandler->findHandler(aTokenizer);
            }
            return nullptr;
        }

        virtual CommandProcessor *recognises(Tokenizer &aTokenizer) = 0;

        virtual ECE41::SharedStatement makeStatement(Tokenizer &aTokenizer, StatusResult &aResult) = 0;

        virtual StatusResult run(CommandStatement &aStatement, ViewListener &aViewer) = 0;

        CommandHandler nextHandler;

    protected:
        static void failedOperation(TableView2 &aView, ViewListener &aViewer){
            aView.setCount(0);
            aView.setSuccess(false);
            aViewer(aView);
        }
    };
}

#endif //ECE141DB_COMMANDPROCESSOR_HPP