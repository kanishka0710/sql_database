//
// Created by Emin Tunc Kirimlioglu on 3/31/24.
//

#ifndef ECE141DB_TESTUTILITIES_HPP
#define ECE141DB_TESTUTILITIES_HPP

#include <iostream>
#include <string>
#include <optional>
#include <sstream>
#include <map>
#include <functional>
#include <unordered_map>

namespace ECE141 {

    const std::string_view theVersion{"Tester(1.1)"};

    using StringMap   = std::map<std::string_view, std::string_view>;
    using Validate    = std::function<bool(const std::string &aBuffer)>;
    using TestCall    = std::function<bool(std::stringstream &anOutput)>;
    using TestCalls   = std::map<std::string, TestCall>;
    using InputLine   = std::pair<std::string_view, size_t>;
    using LineList    = std::initializer_list<InputLine>;


    class TestUtilities {
    public:

        static bool verifyAgainstMap(const std::string& aString, std::unordered_map<std::string, int32_t> theCounts){
            std::stringstream theInput(aString);
            std::string line;
            while (!theInput.eof()) {
                std::getline(theInput, line);
                std::stringstream theLineInput(line);
                std::string theName;
                while (theLineInput >> theName) {
                    if (theCounts.count(theName)) {
                        theCounts[theName] -= 1;
                    }
                }
            }
            bool theResult = true;
            // check if all the counts are zero
            for (const auto& [prefix, count] : theCounts) {
                theResult &= count == 0;
            }
            return theResult;
        }

        static bool matches(const std::string &aBuffer, const LineList &aLines) {
            std::stringstream theInput(aBuffer);
            std::string theLine;
            for(auto &theItem: aLines) {
                if(std::getline(theInput,theLine)) {
                    if (theLine.find(theItem.first) != std::string::npos) {
                        if(theLine.length()<theItem.second)
                            return false;
                    }
                    else return false;
                }
                else return false;
            }
            return true;
        }
    };
}

#endif //ECE141DB_TESTUTILITIES_HPP
