//
// Created by kanis on 6/6/2024.
//

#include "misc/DBQuery.hpp"
#include "database/Schema.hpp"
#include "misc/Logger.hpp"

#ifndef ECE141DB_SELECTPROCESSOR_HPP
#define ECE141DB_SELECTPROCESSOR_HPP

#endif //ECE141DB_SELECTPROCESSOR_HPP

namespace ECE141 {

    class ProcessorHelpers {
    public:

        static std::vector<StringVector> handleJoin(DBQuery &aQuery, std::map<std::string, Schema> &schemasRequired,
                                                    const std::map<std::string, StringVector>& headers, const std::map<std::string, std::multimap<Value, size_t>>& validRows);
        static std::vector<StringVector> leftJoin(DBQuery &aQuery, std::map<std::string, Schema> &schemasRequired,
                                                  std::map<std::string, StringVector> headers, std::map<std::string, std::multimap<Value, size_t>> validRows);
        static std::vector<StringVector> rightJoin(DBQuery &aQuery, std::map<std::string, Schema> &schemasRequired,
                                                   std::map<std::string, StringVector> headers, std::map<std::string, std::multimap<Value, size_t>> validRows);

        // Can add more join types below similar to the ones above


        // Helper function to process row data
        template <typename RowContentType>
        static void processRowData(RowKeyValues &rowData, std::map<std::string, StringVector> &headers, RowContentType &rowContent, std::string &key);
        static std::string findKeyToMatch(Row &aRow, std::map<std::string, StringVector> &headers, std::string &key, std::string &fieldName);

        // Helper Functions to Load Common Items (TOC, Schema)
        static size_t getAndVerifyTable(std::string &tableName);
    };

}