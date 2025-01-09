//
//  FolderReader.hpp
//  Database5
//
//  Created by rick gessner on 4/4/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef FolderReader_h
#define FolderReader_h

#include <string>
#include <filesystem>
#include <fstream>
#include <algorithm>

namespace fs = std::filesystem;

namespace ECE141 {

    using FileVisitor = std::function<bool(const std::string &)>;

    class FolderReader {
    public:
        FolderReader(const char *aPath) : path(aPath) {}

        virtual ~FolderReader() = default;

        virtual bool exists(const std::string &aFilename) {
            std::ifstream theStream(aFilename);
            return !!theStream;
        }

        virtual void each(const std::string &anExt, const FileVisitor &aVisitor) const {
            std::filesystem::path aPath{path};
            std::filesystem::directory_iterator directoryIterator{aPath};
            std::for_each(begin(directoryIterator), end(directoryIterator),
                    [&aVisitor](const auto& dir_entry) { aVisitor(dir_entry.path().string()); });
        };

        std::string path;
    };

}

#endif /* FolderReader_h */
