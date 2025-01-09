//
//  Database.cpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//

#include <string>
#include <iostream>
#include <iomanip>
#include <map>
#include "database/Database.hpp"
#include "misc/Config.hpp"


namespace ECE141 {


    Database::Database(const std::string& aName, AccessMode aMode)
            : name(aName), changed(true) {
        std::string thePath = Config::getDBPath(name);
        //create for read/write
        std::ios_base::openmode flags = std::visit([](auto &&arg) -> std::ios_base::openmode {
            return arg; // Implicit conversion to openmode using the conversion operator
        }, aMode);
        databaseFile.open(thePath, flags);
    }

    void Database::setNewDatabaseFile(const std::string& aPath, AccessMode aMode) {
        name = aPath;
        changed = true;
        std::ios_base::openmode flags = std::visit([](auto &&arg) -> std::ios_base::openmode {
            return arg; // Implicit conversion to openmode using the conversion operator
        }, aMode);
        databaseFile.open(Config::getDBPath(aPath), flags);
    }

    Database::Database(const Database& aDatabase) {
        databaseFile.open(aDatabase.name);
        name = aDatabase.name;
        changed = aDatabase.changed;
    }

    Database& Database::operator=(const Database& aDatabase) {
        if (this != &aDatabase) {
            if (databaseFile.is_open()) {
                databaseFile.close();
            }
            name = aDatabase.name;
            changed = aDatabase.changed;
            std::string thePath = Config::getDBPath(name);
            std::ios_base::openmode flags = std::ios_base::in | std::ios_base::out;
            databaseFile.open(thePath, flags);
        }
        return *this;
    }

//    Database::~Database() {
//        if (changed) {
//            //stuff to save?
//        }
//    }

    // USE: Dump command for debug purposes...
    StatusResult Database::dump(std::ostream &anOutput) {
        return StatusResult{Errors::noError};
    }

    std::fstream& Database::getDatabaseFile() {
        return databaseFile;
    }


}
