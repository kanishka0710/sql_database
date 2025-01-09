//
// Created by kanis on 6/6/2024.
//

#include <string>

#ifndef ECE141DB_TABLEFIELD_H
#define ECE141DB_TABLEFIELD_H

#endif //ECE141DB_TABLEFIELD_H

namespace ECE141 {
    struct TableName {
        TableName(const std::string &aTableName, const std::string &anAlias="")
                : table(aTableName), alias(anAlias) {}
        TableName(const TableName &aCopy) : table(aCopy.table), alias(aCopy.alias) {}
        TableName& operator=(const std::string &aName) {
            table=aName;
            return *this;
        }
        operator const std::string() {return table;}
        std::string table;
        std::string alias;
    };
    struct TableField {
        TableField(const std::string &aName="", const std::string &aTable="")
                : fieldName(aName), table(aTable) {}
        TableField(const TableField &aCopy) : fieldName(aCopy.fieldName), table(aCopy.table) {}
        bool operator==(const TableField &other) const {
            return fieldName == other.fieldName && table == other.table;
        }
        std::string fieldName;
        std::string table;
    };
}