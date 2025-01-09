//
//  Database.hpp
//  PA2
//
//  Created by rick gessner on 2/27/23.
//

#ifndef Database_hpp
#define Database_hpp

#include <cstdio>
#include <fstream> 
#include "storage/Storage.hpp"


namespace ECE141 {
    class Database {
    public:
        Database(const std::string& aPath, AccessMode);
        Database() : changed(false) {}
        Database(const Database& aDatabase);
        Database& operator=(const Database& aDatabase);
        virtual ~Database() = default;

        StatusResult    dump(std::ostream &anOutput); //debug...
        std::fstream& getDatabaseFile();
        std::string getDatabaseName() {return name;}

        void setNewDatabaseFile(const std::string& aPath, AccessMode aMode);

    protected:
        std::string     name;
        bool            changed;
        std::fstream    databaseFile;


    };
}

#endif /* Database_hpp */
