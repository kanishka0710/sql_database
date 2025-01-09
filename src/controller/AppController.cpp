//
//  CommandProcessor.cpp
//  ECEDatabase
//
//  Created by rick gessner on 3/30/23.
//  Copyright © 2018-2023 rick gessner. All rights reserved.
//

#include <iostream>
#include <memory>
#include <map>
#include "controller/AppController.hpp"
#include "tokenizer/Tokenizer.hpp"
#include "controller/RowProcessor.hpp"
#include "controller/TableProcessor.hpp"
#include "controller/DatabaseProcessor.hpp"
#include "controller/BasicCommandProcessor.hpp"

#include "misc/Logger.hpp"

Logger* Logger::logger = nullptr;

namespace ECE141 {

    AppController::AppController() : running{true} {}

    AppController::AppController(std::ostream &anOutput) : running{true} {}

    AppController::~AppController() = default;

    // USE: -----------------------------------------------------

    //build a tokenizer, tokenize input, ask processors to handle...
    StatusResult AppController::handleInput(std::istream &anInput,
                                            ViewListener aViewer) {
        Tokenizer theTokenizer(anInput);
        StatusResult theResult = theTokenizer.tokenize();
        //theTokenizer.dump();

        auto rowProcessor = std::make_shared<RowProcessor>();
        auto tableProcessor = std::make_shared<TableProcessor>(rowProcessor);
        auto databaseProcessor = std::make_shared<DatabaseProcessor>(tableProcessor);
        auto basicCommandProcessor = std::make_shared<BasicCommandProcessor>(databaseProcessor);



        while (theResult && theTokenizer.more()) {
            if (auto theProcessor = basicCommandProcessor->findHandler(theTokenizer)) {
                if (auto theStatement = theProcessor->makeStatement(theTokenizer, theResult)) {
                    if (theResult) theResult = theProcessor->run(**theStatement, aViewer);
                    if (theResult)
                        if(!theTokenizer.skipIf(semicolon)){
                            while(theTokenizer.more())
                                theTokenizer.next();
                                theTokenizer.skipIf(Keywords::semicolon_kw);//TODO: Bandaid
                        }
                } else {
                    if(theResult.error==Errors::noError)
                        theResult = Errors::syntaxError;
                    std::string anError="Error 101: Identifier expected at line 1";
                    TextView view(anError);
                    aViewer(view);
                }
            } else {
                theResult = Errors::unknownCommand;
            }
        }
        return theResult;
    }

    OptString AppController::getError(StatusResult &aResult) const {

        static std::map<ECE141::Errors, std::string_view> theMessages = {
                {Errors::illegalIdentifier, "Illegal identifier"},
                {Errors::unknownIdentifier, "Unknown identifier"},
                {Errors::databaseExists,    "Database exists"},
                {Errors::tableExists,       "Table Exists"},
                {Errors::syntaxError,       "Syntax Error"},
                {Errors::unknownCommand,    "Unknown command"},
                {Errors::unknownDatabase,   "Unknown database"},
                {Errors::unknownTable,      "Unknown table"},
                {Errors::unknownError,      "Unknown error"}
        };

        std::string_view theMessage = "Unknown Error";
        if (theMessages.count(aResult.error)) {
            theMessage = theMessages[aResult.error];
        }
        return theMessage;
    }



}