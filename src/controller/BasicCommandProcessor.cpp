//
// Created by kanis on 4/29/2024.
//

#include "controller/BasicCommandProcessor.hpp"
#include "parsing/CommandStatement.hpp"
namespace ECE141 {

    CommandProcessor* BasicCommandProcessor::recognises(Tokenizer &aTokenizer) {
        KWList validList = {ECE141::Keywords::about_kw, ECE141::Keywords::version_kw,
                            ECE141::Keywords::quit_kw, ECE141::Keywords::help_kw};
        TokenSequencer aSequencer(aTokenizer);
        if (aSequencer.skipIf(validList)) return this;
        else return nullptr;
    }

    ECE41::SharedStatement BasicCommandProcessor::makeStatement(Tokenizer &aTokenizer, StatusResult &aResult) {
        auto aStatement = std::make_shared<CommandStatement>();
        ECE41::AppCommand aCommand(aTokenizer);
        if (!aCommand.createCommand(*aStatement, aResult)) {
            aResult.error = Errors::invalidCommand;
            return std::nullopt;
        }
        return aStatement;
    }

    std::string BasicCommandProcessor::handleAbout() {
        return "Authors: " + ECE141::Config::getMembers();
    }

    std::string BasicCommandProcessor::handleVersion() {
        return "Version: " + ECE141::Config::getVersion();
    }

    std::string BasicCommandProcessor::handleQuit() {
        return "DB::141 is shutting down";
    }

    std::string BasicCommandProcessor::handleHelp() {
        std::string returnValue = "Available commands:\n";
        for (size_t i = 0; i < helpDescriptions.size(); i++) {
            returnValue.append(helpDescriptions[i]);
            if (helpDescriptions.size() != (i + 1)) {
                returnValue.append("\n");
            }
        }
        return returnValue;
    }

    StatusResult BasicCommandProcessor::run(CommandStatement &aStatement, ViewListener &aViewer) {
        std::map<Keywords, std::function<std::string()>> dispatchMap{
                {Keywords::about_kw,   [&]() { return handleAbout(); }},
                {Keywords::version_kw, [&]() { return handleVersion(); }},
                {Keywords::help_kw,    [&]() { return handleHelp(); }},
                {Keywords::quit_kw,    [&]() { return handleQuit(); }}
        };

        for (const auto &token: aStatement.statement) {
            if (dispatchMap.count(token.keyword)) {
                std::string output = dispatchMap[token.keyword]();
                TextView view(output);
                aViewer(view);
            } else {
                return Errors::unknownCommand;
            }
        }
        return Errors::noError;
    }
}
