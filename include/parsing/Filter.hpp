//
// Created by aryam on 02-05-2024.
//

#ifndef ECE141DB_FILTER_HPP
#define ECE141DB_FILTER_HPP

#include <iostream>
#include <vector>
#include <string>
#include "tokenizer/Tokenizer.hpp"
#include "database/Attribute.hpp"

namespace ECE141 {

    struct Conditions {
        std::vector<Token> aCondition;
    };

    class Filter {
    public:
        std::string& tableName;
        std::vector<Attribute> selectList;
        std::vector<Conditions> whereConditionList;
        std::vector<Conditions> groupConditionList;
        std::vector<Conditions> havingConditionList;




    };

}
#endif //ECE141DB_FILTER_HPP
