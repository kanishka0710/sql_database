cmake_minimum_required(VERSION 3.10)
project(ECE141db)


set(CMAKE_CXX_STANDARD 17)


include_directories(include)


file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

set(SOURCES ${SOURCES} ${HEADERS}
        src/misc/DBQuery.cpp
        src/misc/Validation.cpp
        include/misc/Validation.h
        include/parsing/Filter.hpp
        include/parsing/Filter.hpp
        include/misc/LRUCache.hpp
        include/database/RowCache.hpp
        include/view/ViewCache.hpp
        include/database/SchemaCache.hpp
        include/database/StorableCache.hpp
        include/controller/ProcessorHelpers.hpp
        src/controller/ProcessorHelpers.cpp
        include/misc/TableField.h
)


add_executable(ECE141db ${SOURCES})
