//
// Created by Kanishka Roy on 5/6/24.
//

#include "misc/DBQuery.hpp"
#include "misc/Helpers.hpp"

namespace ECE141 {

    StatusResult DBQuery::addSelectField(std::string &fieldName) {
        selectFields.push_back(fieldName);
        return StatusResult{Errors::noError};
    }

    StatusResult DBQuery::addWhereExpression(Expression &aExpression) {
        whereExpressions.push_back(aExpression);
        return StatusResult{Errors::noError};
    }

    StatusResult DBQuery::addOrderByField(std::string fieldName) {
        orderByField = fieldName;
        return StatusResult{Errors::noError};
    }

    StatusResult DBQuery::changeLimit(int newLimit) {
        limit = newLimit;
        isLimitTrue = true;
        return StatusResult{Errors::noError};
    }

    StatusResult DBQuery::changeAllFields(bool val) {
        all = val;
        return StatusResult{Errors::noError};
    }

    StatusResult DBQuery::changeTableName(std::string tableName) {
        this->tableName = tableName;
        return StatusResult{Errors::noError};
    }

    StatusResult DBQuery::changeSetField(std::string aSetField) {
        this->setField = aSetField;
        return StatusResult{Errors::noError};
    }
    StatusResult DBQuery::changeSetValue(std::string aSetValue) {
        this->setValue = aSetValue;
        return StatusResult{Errors::noError};
    }
    StatusResult DBQuery::changeJoinType(Keywords aJoinType) {
        if (std::find(std::begin(gJoinTypes), std::end(gJoinTypes), aJoinType) != std::end(gJoinTypes)) {
            this->joinArgument.joinType = aJoinType;
            return StatusResult{Errors::noError};
        }
        return StatusResult{Errors::joinTypeExpected};
    }

    StatusResult DBQuery::addJoinTable(std::string tableName) {
        this->joinArgument.table = tableName;
        return StatusResult{Errors::noError};
    }

    StatusResult DBQuery::addJoinPair(std::string RtableName, std::string Rattribute, std::string LtableName, std::string Lattribute) {
        this->joinArgument.onLeft = TableField(Rattribute, RtableName);
        this->joinArgument.onRight = TableField(Lattribute, LtableName);
        return StatusResult{Errors::noError};
    }

    StatusResult DBQuery::addCountField(std::string aCount) {
        count = aCount;
        return StatusResult{Errors::noError};
    }
    StatusResult DBQuery::addAsField(std::string aAsField) {
        asField = aAsField;
        return StatusResult{Errors::noError};
    }

    std::string DBQuery::getSetField() {
        return this->setField;
    }
    std::string DBQuery::getSetValue() {
        return this->setValue;
    }

    std::string DBQuery::getTableName() const {
        return tableName;
    }

    std::vector<std::string> DBQuery::getSelectFields() {
        return selectFields;
    }

    std::string DBQuery::getOrderByField() {
        return orderByField;
    }

    Expressions DBQuery::getWhereExpressions() {
        return whereExpressions;
    }


    int DBQuery::getLimit() const {
        return limit;
    }

    bool DBQuery::getIfLimit() const {
        return isLimitTrue;
    }

    bool DBQuery::getIfAllFields() const {
        return all;
    }


    Join DBQuery::getJoin() {
        return this->joinArgument;
    }

    std::vector<std::string> DBQuery::getAllSchemaNamesRequired() {
        std::vector<std::string> tableNames;
        tableNames.emplace_back(this->getTableName());
        if (!this->joinArgument.empty()) {
            tableNames.emplace_back(this->joinArgument.table);
        }
        return tableNames;
    }

    std::string DBQuery::getCount() {
        return count;
    }
    std::string DBQuery::getAsField() {
        return asField;
    }

}