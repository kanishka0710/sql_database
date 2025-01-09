//
//  DBQuery.hpp
//  PA5
//
//  Created by rick gessner on 4/7/23.
//

#ifndef DBQuery_h
#define DBQuery_h

#include "misc/Filters.hpp"
#include "misc/TableField.h"

namespace ECE141 {

    // State held from a fancy select statement

    struct Property {
        Property(std::string aName, uint32_t aTableId = 0) : name(aName), tableId(aTableId), desc(true) {}

        std::string name;
        uint32_t tableId;
        bool desc;

        bool operator==(const Property &other) const {
            return name == other.name && tableId == other.tableId && desc == other.desc;
        }
    };

    using PropertyList = std::vector<Property>;

    struct Join  {
        Join() = default;

        Join(const std::string &aTable, Keywords aType, const std::string &aLHS, const std::string &aRHS)
                : table(aTable), joinType(aType), onLeft(aLHS), onRight(aRHS) {}

        bool operator==(const Join &other) const {
            return joinType == other.joinType && table == other.table && onLeft == other.onLeft && onRight == other.onRight;
        }

        bool empty() {
            return table.empty();
        }

        Keywords    joinType;
        std::string table;
        TableField  onLeft;
        TableField  onRight;
    };

    //--------------------------

    class DBQuery {
    public:

//        DBQuery(Schema *aSchema = nullptr, bool allFields = true)
//                : fromTable(aSchema), all(allFields) {}
//
//        DBQuery(const DBQuery &aQuery) : fromTable(aQuery.fromTable) {}

        DBQuery(std::string newTableName = "", bool useAllFields = true, int newLimit = 0)
                : tableName(newTableName), all(useAllFields), limit(newLimit),
                  selectFields({}), orderByField(""), whereExpressions({}) {}

        DBQuery(const DBQuery &aQuery)
                : tableName(aQuery.tableName), all(aQuery.all), limit(aQuery.limit),
                  selectFields(aQuery.selectFields), orderByField(aQuery.orderByField),
                  whereExpressions(aQuery.whereExpressions) {}

        bool operator==(const DBQuery &other) const {
            return tableName == other.tableName &&
                   selectFields == other.selectFields &&
                   orderByField == other.orderByField &&
                   setField == other.setField &&
                   setValue == other.setValue &&
                   whereExpressions == other.whereExpressions &&
                   joinArgument == joinArgument &&
                   limit == other.limit &&
                   count == other.count &&
                   asField == other.asField &&
                   all == other.all &&
                   descending == other.descending;
        }

        StatusResult addSelectField(std::string &fieldName);
        StatusResult addWhereExpression(Expression &aExpression);
        StatusResult addOrderByField(std::string fieldName);
        StatusResult addOrder(std::string fieldName);
        StatusResult changeLimit(int newLimit);
        StatusResult changeAllFields(bool val);
        StatusResult changeTableName(std::string tableName);
        StatusResult changeSetField(std::string aSetField);
        StatusResult changeSetValue(std::string aSetValue);
        StatusResult changeJoinType(Keywords aJoinType);
        StatusResult addJoinTable(std::string tableName);
        StatusResult addJoinPair(std::string RtableName, std::string Rattribute, std::string LtableName, std::string Lattribute);
        StatusResult addCountField(std::string aCount);
        StatusResult addAsField(std::string aAsField);

        std::string getTableName() const;
        std::vector<std::string> getSelectFields();
        std::string getOrderByField();
        Expressions getWhereExpressions();
        int getLimit() const;
        bool getIfLimit() const;
        bool getIfAllFields() const;
        std::string getSetField();
        std::string getSetValue();
        Join getJoin();
        std::string getCount();
        std::string getAsField();

        std::vector<std::string> getAllSchemaNamesRequired();
        bool descending;//asc or desc

    protected:

        std::string tableName;
        Filters filters; // unused
        StringVector selectFields;
        std::string orderByField;
        std::string setField;
        std::string setValue;
        Expressions whereExpressions;
        Join joinArgument;
        int limit;
        bool isLimitTrue = false;
        std::string count;
        std::string asField;
        bool all; //if user used SELECT * FROM...
    };

}

#endif /* DBQuery_h */


