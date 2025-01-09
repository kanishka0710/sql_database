//
// Created by Kanishka Roy on 5/6/24.
//

#ifndef ECE141DB_VALIDATION_H
#define ECE141DB_VALIDATION_H


#include "database/Row.hpp"

namespace ECE141 {
    class Validation {

    public:
        static bool validateDBQuery(DBQuery &aQuery, Schema &aSchema);
        static bool validRow(DBQuery &aQuery, Row &aRow);
        static bool validateWhereExpressions(DBQuery &aQuery, Schema &aSchema);
        static bool validateOrderBy(DBQuery &aQuery, Schema &aSchema);
        static bool validateSelectFields(DBQuery &aQuery, Schema &aSchema);
        static bool validateGroupBy(DBQuery &aQuery, Schema &aSchema);
        static bool validateSetFields(DBQuery &aQuery, Schema &aSchema);
        static bool validateJoinFields(DBQuery &aQuery, Schema &aSchema);
    };
}


#endif //ECE141DB_VALIDATION_H
