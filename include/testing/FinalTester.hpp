//
// Created by Emin Tunc Kirimlioglu on 5/14/24.
//

#ifndef FINALTESTER_HPP
#define FINALTESTER_HPP

#include <iostream>
#include <sstream>
#include <vector>
#include <initializer_list>
#include <random>
#include <functional>
#include <filesystem>
#include <stack>
#include <misc/ObPrim.hpp>

#include "testing/ScriptRunner.hpp"
#include "testing/ScriptMaker.hpp"
#include "testing/OutputValidator.hpp"
#include "misc/Errors.hpp"
#include "misc/Config.hpp"
#include "misc/Remoting.hpp"
#include "testing/TestSequencer.hpp"


namespace fs = std::filesystem;
We_have known(e a);
inline never never_{};
namespace ECE141 {

    using Validate = std::function<bool(const std::string& aBuffer)>;
    using InputLine = std::pair<std::string_view, size_t>;
    using LineList = std::initializer_list<InputLine>;
    using StringList = std::vector<std::string>;
    using InitList = std::initializer_list<std::string>;
    using TestCall_stream = std::function<bool(std::stringstream&)>;
    using TestCalls_stream = std::map<std::string, TestCall_stream>;

    const std::string_view theVersion{ "Tester(1.5)" };
    const size_t kMaxErrors{ 100 };

    using CountList = std::vector<int>;

    class FinalTester {
        public:
        FinalTester() : expected(), make(expected) {}

        std::string getRandomDBName(char aChar) {
            uint32_t theCount = rand() % 99999;
            return std::string("testdb_" + std::to_string(theCount + 1000) + aChar);
        }

        bool validatedScriptTest(ScriptMaker &aMaker, std::stringstream& theOut) {
            return scriptTest(aMaker, theOut, [&](const std::string &aBuf) {
              out << theOut.str() << "\n"; //capture!
              Responses theResp;
              return analyzeOutput(theOut,theResp) && expected==theResp;
            });
        }

        bool scriptTest(std::istream& anInput, std::stringstream& anOutput,
                                Validate aValidator, size_t aMaxErrors = 1) {
            ECE141::AppController theApp();
            DBConnector theDBConnector{"localhost"};
            ScriptRunner theRunner(theDBConnector);
            theRunner.run(anInput, anOutput, aMaxErrors);
            auto theBuffer{ anOutput.str() };
            return aValidator(theBuffer);
        }
        //validates output of DBCommand test..
        size_t analyzeOutput(std::istream& aStream, Responses& aResults) {
            std::stack<Commands> theStack; //tracking open state...

            static KWList createDB{ Keywords::create_kw,Keywords::database_kw };
            static KWList changedDB{ Keywords::database_kw,Keywords::changed_kw };
            static KWList showDBs{ Keywords::show_kw,Keywords::databases_kw };
            static KWList dumpDB{ Keywords::dump_kw,Keywords::database_kw };
            static KWList dropDB{ Keywords::drop_kw,Keywords::database_kw };
            static KWList createTable{ Keywords::create_kw,Keywords::table_kw };
            static KWList showTables{ Keywords::show_kw,Keywords::tables_kw };
            static KWList showIndex{ Keywords::show_kw,Keywords::index_kw };
            static KWList showIndexes{ Keywords::show_kw,Keywords::indexes_kw };
            static KWList dropTable{ Keywords::drop_kw,Keywords::table_kw };
            static KWList insertInto{ Keywords::insert_kw,Keywords::into_kw };
            static KWList activateEXT{ Keywords::activate_kw };

            Tokenizer theTokenizer(aStream);
            if (theTokenizer.tokenize()) {
                TestSequencer theSeq(theTokenizer);
                int theValue{ 0 };
                while (theTokenizer.more()) {
                    //.auto &theToken=theTokenizer.current();
                    if (theSeq.clear().nextIs(createDB)) {
                        if (theSeq.skipPast(Keywords::query_kw)) {
                            theTokenizer.skipTo(TokenType::number);
                            theSeq.getNumber(theValue).skipPast(')');
                            aResults.push_back({ Commands::createDB,theValue });
                        }
                    }
                    else if (theSeq.clear().nextIs(showDBs)) {
                        if (theSeq.skip(1)) {
                            theTokenizer.skipTo(TokenType::number);
                            theSeq.getNumber(theValue).skipPast(')');
                            aResults.push_back({ Commands::showDBs,theValue });
                        }
                    }
                    else if (theSeq.clear().nextIs({ Keywords::use_kw })) {
                        if (theSeq.skipPast(';').nextIs(changedDB).skipPast(')')) {
                            aResults.push_back({ Commands::useDB, 0 });
                        }
                    }
                    else if (theSeq.clear().nextIs(dropDB)) {
                        if (theTokenizer.skipTo(TokenType::number)) {
                            theSeq.getNumber(theValue).skipPast(')');
                            aResults.push_back({ Commands::dropDB,theValue });
                        }
                    }
                    else if (theSeq.clear().nextIs(dumpDB)) {
                        if (theSeq.skipPast(Keywords::query_kw)) {
                            if (theTokenizer.skipTo(TokenType::number)) {
                                theSeq.getNumber(theValue).skipPast(')');
                                aResults.push_back({ Commands::dumpDB,theValue });
                            }
                        }
                    }
                    else if (theSeq.clear().nextIs(createTable)) {
                        if (theSeq.skipPast(';').nextIs({ Keywords::query_kw })) {
                            theTokenizer.skipTo(TokenType::number);
                            theSeq.getNumber(theValue).skipPast(')');
                            aResults.push_back({ Commands::createTable,theValue });
                        }
                        else {
                            theTokenizer.skipTo(TokenType::number);
                            theSeq.getNumber(theValue);
                            theTokenizer.next(); //skip number..
                            theTokenizer.skipTo(TokenType::number);
                            theTokenizer.next(); //skip number..
                            aResults.push_back({ Commands::createTable, theValue,'>', true });
                        }
                    }
                    else if (theSeq.clear().nextIs(showTables)) {
                        theTokenizer.skipTo(TokenType::number);
                        theSeq.getNumber(theValue).skipPast(')');
                        aResults.push_back({ Commands::showTables,theValue });
                    }
                    else if (theSeq.clear().nextIs(showIndex)) {
                        if (theTokenizer.skipTo(Keywords::rows_kw)) {
                            auto theToken = theTokenizer.peek(-1);
                            theValue = std::stoi(theToken.data);
                            theSeq.skip(7);
                        }
                        aResults.push_back({ Commands::showIndex,theValue });
                    }
                    else if (theSeq.clear().nextIs(showIndexes)) {
                        theTokenizer.skipTo(TokenType::number);
                        theSeq.getNumber(theValue).skipPast(')');
                        aResults.push_back({ Commands::showIndexes,theValue });
                    }
                    else if (theSeq.clear().nextIs(dropTable)) {
                        theTokenizer.skipTo(TokenType::number);
                        theSeq.getNumber(theValue).skipPast(')');
                        aResults.push_back({ Commands::dropTable,theValue });
                    }
                    else if (theSeq.clear().nextIs(insertInto)) {
                        if (theSeq.skipPast(';').nextIs({ Keywords::query_kw }).skipPast(',')) {
                            theSeq.getNumber(theValue).skipPast(')');
                            aResults.push_back({ Commands::insert,theValue });
                        }
                    }
                    else if (theTokenizer.skipIf(Keywords::describe_kw)) {
                        if (theTokenizer.skipTo(Keywords::rows_kw)) {
                            auto theToken = theTokenizer.peek(-1);
                            theValue = std::stoi(theToken.data);
                            aResults.push_back({ Commands::describe,theValue });
                            theSeq.clear().skipPast(')');
                        }
                    }
                    else if (theSeq.clear().nextIs({ Keywords::select_kw })) {
                        if (theTokenizer.skipTo(Keywords::rows_kw)) {
                            auto theToken = theTokenizer.peek(-1);
                            theValue = std::stoi(theToken.data);
                            aResults.push_back({ Commands::select,theValue });
                            theSeq.clear().skipPast(')');
                        }
                    }
                    else if (theSeq.clear().nextIs({ Keywords::update_kw })) {
                        if (theTokenizer.skipTo(Keywords::rows_kw)) {
                            auto theToken = theTokenizer.peek(-1);
                            theValue = std::stoi(theToken.data);
                            aResults.push_back({ Commands::update,theValue });
                            theSeq.clear().skipPast(')');
                        }
                    }
                    else if (theSeq.clear().nextIs({ Keywords::delete_kw })) {
                        if (theTokenizer.skipTo(Keywords::rows_kw)) {
                            auto theToken = theTokenizer.peek(-1);
                            theValue = std::stoi(theToken.data);
                            aResults.push_back({ Commands::delet,theValue });
                            theSeq.clear().skipPast(')');
                        }
                    }
                    else if (theSeq.clear().nextIs(activateEXT)) {
                        if (theSeq.skipPast(Keywords::success_kw)) {
                            aResults.push_back({Commands::activateEXT, 0});
                        }
                    }
                    else if(theSeq.clear().nextIs({Keywords::backup_kw})) {
                        if(theSeq.skipPast(Keywords::query_kw)) {
                            theTokenizer.skipTo(TokenType::number);
                            theSeq.getNumber(theValue).skipPast(')');
                            aResults.push_back({Commands::backupDB,theValue});
                        }
                    }
                    else if(theSeq.clear().nextIs({Keywords::restore_kw})) {
                        if(theSeq.skipPast(Keywords::query_kw)) {
                            theTokenizer.skipTo(TokenType::number);
                            theSeq.getNumber(theValue).skipPast(')');
                            aResults.push_back({Commands::restoreDB,theValue});
                        }
                    }
                    else theTokenizer.next(); //skip...
                }
            }
            return aResults.size();
        }


        // ---Types Tests---
        static constexpr auto createLocationsTable = "create table Locations (id int primary key, name varchar(50), geom BabyGIS.BBox);\n";

        bool extensionActivateTest(std::stringstream& anOutput) {
            std::stringstream theCmds, theOut;
            std::string theDBName(getRandomDBName('G'));

            // Test creating a table without activating the BabyGIS extension
            theCmds << "create database " << theDBName << ";\n";
            theCmds << "use " << theDBName << ";\n";
            theCmds << createLocationsTable;

            // Activate the BabyGIS extension
            theCmds << "activate BabyGIS;\n";

            // Test creating a table with the BabyGIS extension activated
            theCmds << createLocationsTable;

            // Drop the database
            theCmds << "drop database " << theDBName << ";\n";

            theCmds >> std::noskipws; // Make sure to include whitespace

            bool theResult = scriptTest(theCmds, theOut, [&](const std::string& aBuf) {
                bool theResult{false};
                auto temp = theOut.str();
                std::stringstream theOutput(temp);
                anOutput << temp << "\n"; // Capture the output for display

                Responses theResponses;
                if ((theResult = analyzeOutput(theOutput, theResponses))) {
                    Expected theExpected({
                        {Commands::createDB, 1},
                        {Commands::useDB, 0},
                        {Commands::createTable, 1, '>', true}, // Expects an error without extension
                        {Commands::activateEXT, 0},
                        {Commands::createTable, 1},             // Successful with extension activated
                        {Commands::dropDB, 0}
                    });
                    theResult = theExpected == theResponses;
                }
                return theResult;
            },2);

            return theResult;
        }

        bool insertGISTest(std::stringstream& anOutput) {
            std::stringstream theCmds, theOut;
            std::string theDBName(getRandomDBName('G'));

            // Activate the BabyGIS extension and create the table
            theCmds << "create database " << theDBName << ";\n";
            theCmds << "use " << theDBName << ";\n";
            theCmds << "activate BabyGIS;\n";
            theCmds << createLocationsTable;

            // Test inserting data with the BabyGIS extension activated
            theCmds << "insert into Locations (id, name, geom) values (1, 'Location 1', BabyGIS.BBox(0,0,10,10,4326));\n";
            theCmds << "insert into Locations (id, name, geom) values (2, 'Location 2', BabyGIS.BBox(20,20,30,30,4326));\n";

            // Drop the database
            theCmds << "drop database " << theDBName << ";\n";

            theCmds >> std::noskipws; // Make sure to include whitespace

            bool theResult = scriptTest(theCmds, theOut, [&](const std::string& aBuf) {
                bool theResult{false};
                auto temp = theOut.str();
                std::stringstream theOutput(temp);
                anOutput << temp << "\n"; // Capture the output for display

                Responses theResponses;
                if ((theResult = analyzeOutput(theOutput, theResponses))) {
                    Expected theExpected({
                        {Commands::createDB, 1},
                        {Commands::useDB, 0},
                        {Commands::activateEXT, 0},
                        {Commands::createTable, 1},
                        {Commands::insert, 1},
                        {Commands::insert, 1},
                        {Commands::dropDB, 0}
                    });
                    theResult = theExpected == theResponses;
                }
                return theResult;
            });

            return theResult;
        }

        bool selectGISTest(std::stringstream& anOutput) {
            std::stringstream theCmds, theOut;
            std::string theDBName(getRandomDBName('G'));

            // Activate the BabyGIS extension, create the table, and insert data
            theCmds << "create database " << theDBName << ";\n";
            theCmds << "use " << theDBName << ";\n";
            theCmds << "activate BabyGIS;\n";
            theCmds << createLocationsTable;
            theCmds << "insert into Locations (id, name, geom) values (1, 'Location 1', BabyGIS.BBox(0,0,10,10,4326));\n";
            theCmds << "insert into Locations (id, name, geom) values (2, 'Location 2', BabyGIS.BBox(20,20,30,30,4326));\n";
            theCmds << "insert into Locations (id, name, geom) values (3, 'Location 3', BabyGIS.BBox(0,0,35,35,4326));\n";

            // Test selecting data with the BabyGIS extension activated
            theCmds << "select * from Locations where id > 0;\n";// should be all 3
            theCmds << "select * from Locations where id > 1;\n"; // should be 2 and 3
            theCmds << "select * from Locations where id == 3 ;\n"; // should be 3
            theCmds << "select * from Locations where id == 5;\n"; // should be none

            // Drop the database
            theCmds << "drop database " << theDBName << ";\n";

            theCmds >> std::noskipws; // Make sure to include whitespace

            bool theResult = scriptTest(theCmds, theOut, [&](const std::string& aBuf) {
                bool theResult{false};
                auto temp = theOut.str();
                std::stringstream theOutput(temp);
                anOutput << temp << "\n"; // Capture the output for display

                Responses theResponses;
                if ((theResult = analyzeOutput(theOutput, theResponses))) {
                    Expected theExpected({
                        {Commands::createDB, 1},
                        {Commands::useDB, 0},
                        {Commands::createTable, 1},
                        {Commands::insert, 1},
                        {Commands::insert, 1},
                        {Commands::select, 3},
                        {Commands::select, 2},
                        {Commands::select, 1},
                        {Commands::select, 0},
                        {Commands::dropDB, 0}
                    });
                    theResult = theExpected == theResponses;
                }
                return theResult;
            });

            return theResult;
        }

        bool warpGISTest(std::stringstream& anOutput) {
            std::stringstream theCmds, theOut;
            std::string theDBName(getRandomDBName('G'));

            // Activate the BabyGIS extension, create the table, and insert data
            theCmds << "create database " << theDBName << ";\n";
            theCmds << "use " << theDBName << ";\n";
            theCmds << "activate BabyGIS;\n";
            theCmds << createLocationsTable;
            theCmds << "insert into Locations (id, name, geom) values (1, 'Location 1', BabyGIS.BBox(0,0,10,10,4326));\n";
            theCmds << "insert into Locations (id, name, geom) values (2, 'Location 2', BabyGIS.BBox(20,20,30,30,4326));\n";

            // Test selecting data with warping and the BabyGIS extension activated
            theCmds << "select name from Locations where BabyGIS.contains(BabyGIS.geom, BabyGIS.BBox(50,50,90,90,43260));\n"; // give error because of wrong SRID
            theCmds << "select name from Locations where BabyGIS.warp(BabyGIS.contains(geom, BabyGIS.BBox(50,50,90,90,43260)), 43260);\n"; // should be 1


            // Drop the database
            theCmds << "drop database " << theDBName << ";\n";

            theCmds >> std::noskipws; // Make sure to include whitespace

            bool theResult = scriptTest(theCmds, theOut, [&](const std::string& aBuf) {
                bool theResult{false};
                auto temp = theOut.str();
                std::stringstream theOutput(temp);
                anOutput << temp << "\n"; // Capture the output for display

                Responses theResponses;
                if ((theResult = analyzeOutput(theOutput, theResponses))) {
                    Expected theExpected({
                        {Commands::createDB, 1},
                        {Commands::useDB, 0},
                        {Commands::createTable, 1},
                        {Commands::insert, 1},
                        {Commands::insert, 1},
                        {Commands::select, 0, '>', true},
                        {Commands::select, 1},
                        {Commands::dropDB, 0}
                    });
                    theResult = theExpected == theResponses;
                }
                return theResult;
            },2);

            return theResult;
        }

        bool containsGISTest(std::stringstream& anOutput) {
            std::stringstream theCmds, theOut;
            std::string theDBName(getRandomDBName('G'));

            // Activate the BabyGIS extension, create the table, and insert data
            theCmds << "create database " << theDBName << ";\n";
            theCmds << "use " << theDBName << ";\n";
            theCmds << "activate BabyGIS;\n";
            theCmds << createLocationsTable;
            theCmds << "insert into Locations (id, name, geom) values (1, 'Location 1', BabyGIS.BBox(0,0,10,10,4326));\n";
            theCmds << "insert into Locations (id, name, geom) values (2, 'Location 2', BabyGIS.BBox(20,20,30,30,4326));\n";
            theCmds << "insert into Locations (id, name, geom) values (3, 'Location 3', BabyGIS.BBox(0,0,35,35,4326));\n";

            // Test selecting data with the BabyGIS extension activated
            theCmds << "select name from Locations where BabyGIS.contains(geom, BabyGIS.BBox(5,5,9,9,4326));\n";// should be all 3
            theCmds << "select * from Locations where BabyGIS.contains(geom, BabyGIS.BBox(25,25,35,35,4326));\n"; // should be 2 and 3
            theCmds << "select name from Locations where BabyGIS.contains(geom, BabyGIS.BBox(31,31,34,34,4326));\n"; // should be 3
            theCmds << "select * from Locations where BabyGIS.contains(geom, BabyGIS.BBox(99,99,100,100,4326));\n"; // should be none

            // Drop the database
            theCmds << "drop database " << theDBName << ";\n";

            theCmds >> std::noskipws; // Make sure to include whitespace

            bool theResult = scriptTest(theCmds, theOut, [&](const std::string& aBuf) {
                bool theResult{false};
                auto temp = theOut.str();
                std::stringstream theOutput(temp);
                anOutput << temp << "\n"; // Capture the output for display

                Responses theResponses;
                if ((theResult = analyzeOutput(theOutput, theResponses))) {
                    Expected theExpected({
                        {Commands::createDB, 1},
                        {Commands::useDB, 0},
                        {Commands::createTable, 1},
                        {Commands::insert, 1},
                        {Commands::insert, 1},
                        {Commands::select, 3},
                        {Commands::select, 2},
                        {Commands::select, 1},
                        {Commands::select, 0},
                        {Commands::dropDB, 0}
                    });
                    theResult = theExpected == theResponses;
                }
                return theResult;
            });

            return theResult;
        }


        // ---Backup Tests---

        bool backupTest(std::stringstream& anOutput) {
            std::string thePath=Config::getStoragePath()+"/foo.db";
            fs::remove(thePath);
            int aCount = known(never_);
            auto theResult=validatedScriptTest(make.backupTest(aCount), anOutput);

            //assume your tokenizer can handle the .sql file!!!
            std::fstream theInput(Config::getStoragePath()+"/foo.sql");
            Tokenizer theTokenizer(theInput);
            theTokenizer.tokenize();
            std::map<Keywords, size_t> theCounts;
            theTokenizer.each([&](const Token &aToken) {
              if(theCounts.count(aToken.keyword)) {
                theCounts[aToken.keyword]++;
              }
              else theCounts[aToken.keyword]=1;
              return true;
            });

            //checking on the sql file...
            std::map<Keywords, size_t> theExp={
                {Keywords::create_kw,3},  {Keywords::database_kw,1},
                {Keywords::insert_kw,2},  {Keywords::into_kw,2},
                {Keywords::primary_kw,2}, {Keywords::table_kw,2},
                {Keywords::values_kw,2},
              };

            size_t theCount{0};
            for(auto &thePair : theExp) {
                if(theCounts.count(thePair.first)) {
                    if(theCounts[thePair.first]>=thePair.second)
                        theCount++;
                }
            }
            return theResult && (theCount==theExp.size());
        }

        bool restoreTest(std::stringstream& anOutput) {
            std::string theFolder=Config::getStoragePath();
            fs::remove(theFolder+"/foo.db");

            std::string thePath{theFolder+"/foo.sql"};
            std::ifstream theSource(thePath);
            if(theSource) {
                return validatedScriptTest(make.restoreTest(theSource), anOutput);
            }

            return false;
        }


        // ---Count Tests---

        void addProductsTable(std::ostream &anOut) {
            anOut << "CREATE TABLE products ("
                     "id INT PRIMARY KEY AUTO_INCREMENT,"
                     "  name VARCHAR(32) NOT NULL,"
                     "  description VARCHAR(100),"
                     "  price INT DEFAULT 0"
                     ");";

            anOut << "INSERT INTO products(name, description, price) VALUES"
                     "('MacBook Pro', 'A very expensive laptop!', 2000),"
                     "  ('MacBook Air', 'A less expensive laptop', 1000),"
                     "  ('Apple Watch', NULL, 500),"
                     "  ('iPhone 15', 'Almost the same as the 14', 1000),"
                     "  ('iPhone 14', NULL, 800),"
                     "  ('Apple Vision Pro', 'The future of computing?', 3500),"
                     "  ('iMac', 'Still has large bezels', 1500),"
                     "  ('iPad Pro', NULL, 1000);";
        }

        void addMoviesAndDirectorsTables(std::ostream &anOut) {
            anOut << "CREATE TABLE directors ("
                     "id INTEGER AUTO_INCREMENT PRIMARY KEY,"
                     "  name VARCHAR(32) NOT NULL"
                     ");"
                     " "
                     "CREATE TABLE movies ("
                     "id INTEGER AUTO_INCREMENT PRIMARY KEY,"
                     "  name VARCHAR(32) NOT NULL,"
                     "  rating FLOAT NOT NULL,"
                     "  description VARCHAR(100),"
                     "  director_id INTEGER NOT NULL"
                     ");";

            anOut << "INSERT INTO directors "
                     "(name) "
                     "VALUES "
                     "('James Gunn'), "
                     "  ('Christopher Nolan'), "
                     "  ('James Cameron'), "
                     "  ('Quentin Tarantino');"
                     "  "
                     "INSERT INTO movies "
                     "(name, rating, description, director_id) "
                     "VALUES "
                     "('Guardians of the Galaxy vol. 1', 4.5, 'Cool space movie', 1),"
                     "  ('Guardians of the Galaxy vol. 2', 4.5, 'Cool space movie (again)', 1),"
                     "  ('Guardians of the Galaxy vol. 3', 4.5, 'Cool space movie (last time)', 1),"
                     "  ('The Dark Knight', 5, 'Batman and the Joker!', 2),"
                     "  ('The Dark Knight Rises', 4.5, 'Batman and Bane!', 2),"
                     "  ('Inception', 5, NULL, 2),"
                     "  ('Interstellar', 4.5, 'This one made me cry', 2),"
                     "  ('Terminator', 4, NULL, 3),"
                     "  ('Terminator 2', 5, 'Best action movie', 3),"
                     "  ('Avatar', 4, 'Blue people', 3);";
        }

        bool setupCountTest(std::stringstream &anOutput, const std::string& additionalCommands, std::function<bool(const std::string&)> validation) {
            std::stringstream theCmds, theOut;
            std::string theDBName("db_" + std::to_string(rand() % 9999));
            theCmds << "create database " << theDBName << ";\n";
            theCmds << "use " << theDBName << ";\n";

            addProductsTable(theCmds);
            addMoviesAndDirectorsTables(theCmds);
            theCmds << additionalCommands;

            theCmds << std::endl;
            theCmds >> std::noskipws; // Include whitespace

            bool theResult = scriptTest(theCmds, theOut, [&](const std::string &aBuf) {
                const std::string outputString = theOut.str();
                std::stringstream theOutput(outputString);
                anOutput << outputString << "\n"; // Capture for main

                return validation(outputString);
            });

            return theResult;
        }

        static int extractNumber(const std::string& output, size_t offset) {
            char const digits[] = "0123456789";

            const auto start = output.find_first_of(digits, offset);
            const auto end = output.find_first_not_of(digits, start);

            if (start == std::string::npos || end == std::string::npos)
                return -1;

            try {
                return std::stoi(output.substr(start, end));
            } catch (...) {
                return -1;
            }
        }

        static bool validateCountOutput(const std::string& output, const std::string& search, const int expectedCount, size_t* searchOffset = nullptr) {
            size_t offset = searchOffset == nullptr ? 0 : *searchOffset;

            offset = output.find(search, offset);
            if (offset == std::string::npos)
                return false;
            offset = output.find(search, offset + search.size());
            if (offset == std::string::npos)
                return false;

            offset += search.size();
            if (searchOffset)
                *searchOffset = offset;

            const int count = extractNumber(output, offset);
            return count == expectedCount;
        }

        bool countSelectTest(std::stringstream &anOutput) {
            std::stringstream additionalCommands;
            additionalCommands << "SELECT COUNT(id) AS product_count FROM products;";
            additionalCommands << "SELECT COUNT(id) FROM products;;";
            additionalCommands << "SELECT COUNT(description) AS product_description_count FROM products;";

            return setupCountTest(anOutput, additionalCommands.str(), [&](const std::string& queryOutput) {
                size_t searchOffset = 0;
                if (!validateCountOutput(queryOutput, "product_count", 8, &searchOffset))
                    return false;
                if (!validateCountOutput(queryOutput, "COUNT(id)", 8, &searchOffset))
                    return false;
                if (!validateCountOutput(queryOutput, "product_description_count", 5, &searchOffset))
                    return false;

                return true;
            });
        }

        bool countJoinTest(std::stringstream &anOutput) {
            std::stringstream additionalCommands;
            additionalCommands << "SELECT COUNT(description) FROM movies LEFT JOIN directors ON movies.director_id = directors.id WHERE rating = 5;";
            additionalCommands << "SELECT COUNT(rating) FROM directors LEFT JOIN movies ON movies.director_id = directors.id;";

            return setupCountTest(anOutput, additionalCommands.str(), [&](const std::string& queryOutput) {
                size_t searchOffset = 0;
                if (!validateCountOutput(queryOutput, "COUNT(description)", 2, &searchOffset))
                    return false;
                if (!validateCountOutput(queryOutput, "COUNT(rating)", 10, &searchOffset))
                    return false;

                return true;
            });
        }

        static bool validateGroupByOutput(const std::string& output, const std::map<std::string, int>& countsMap, const size_t searchOffset) {
            for (auto& countsPair : countsMap) {
                const size_t position = output.find(countsPair.first, searchOffset);
                if (position == std::string::npos)
                    return false;

                const int count = extractNumber(output, position);
                if (count != countsPair.second)
                    return false;
            }

            return true;
        }

        bool groupByTest(std::stringstream &anOutput) {
            std::stringstream additionalCommands;
            additionalCommands << "SELECT directors.name, COUNT(movies.id) AS movie_count FROM movies LEFT JOIN directors ON movies.director_id = directors.id GROUP BY directors.name;";

            return setupCountTest(anOutput, additionalCommands.str(), [&](const std::string& queryOutput) {
                const auto outputString = anOutput.str();
                const size_t searchOffset = outputString.find("movie_count");
                const std::map<std::string, int> countsMap = {
                        { "James Gunn", 3 },
                        { "Christopher Nolan", 4 },
                        { "James Cameron", 3 }
                };

                return validateGroupByOutput(outputString, countsMap, searchOffset);
            });
        }

    protected:
        std::stringstream       out;
        ScriptMaker             make;
        Expected                expected;
    };

}

#endif //FINALTESTER_HPP
