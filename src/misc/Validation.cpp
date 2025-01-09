//
// Created by Kanishka Roy on 5/6/24.
//

#include "misc/Validation.h"

namespace ECE141 {

    bool Validation::validateDBQuery(DBQuery &aQuery, Schema &aSchema) {
        if(validateSelectFields(aQuery, aSchema)) {
            if(validateSetFields(aQuery, aSchema)){
                if (validateWhereExpressions(aQuery, aSchema)) {
                    if (validateOrderBy(aQuery, aSchema)) {
                        if (validateGroupBy(aQuery, aSchema)) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }
    bool Validation::validRow(DBQuery &aQuery, Row &aRow) {
        auto keyValPairs = aRow.getData();
        bool compResult;
        Value left, right;
        for(const auto& exp : aQuery.getWhereExpressions()) {//loop where conditions
            // -------------------------------------------------------
            auto finder = keyValPairs.find(exp.lhs.name);
            if (finder != keyValPairs.end()) {
                left = keyValPairs[exp.lhs.name];
            } else if(exp.lhs.name=="id"){
                left = static_cast<int>(aRow.entityId);
            }
            else {
                return false;
            }

            if(exp.rhs.ttype==TokenType::identifier){
                finder = keyValPairs.find(exp.rhs.name);
                if (finder != keyValPairs.end()) {
                    right = keyValPairs[exp.rhs.name];
                } else {
                    return false;
                }
            }else{right = std::stoi(exp.rhs.name);}
            switch (exp.op) {
                case Operators::equal_op :
                    compResult = (left==right);
                    break;
                case Operators::notequal_op :
                    compResult = (left!=right);
                    break;
                case Operators::lt_op :
                    compResult = (left<right);
                    break;
                case Operators::lte_op :
                    compResult = (left<=right);
                    break;
                case Operators::gt_op :
                    compResult = (left>right);
                    break;
                case Operators::gte_op :
                    compResult = (left>=right);
                    break;
                default:
                    compResult = false;
            }
            if(!compResult){return false;}

            // don't know how to handle logical ops OR/AND
        }

        return true;
    }

    bool Validation::validateWhereExpressions(DBQuery &aQuery, Schema &aSchema){
        if(aQuery.getWhereExpressions().empty()){return true;}
        bool validLeft, validRight;
        DataTypes expressionType;
        for(auto exp : aQuery.getWhereExpressions()){//loop where conditions
            validLeft=false, validRight = false;
            //check left hand side
            if (exp.lhs.ttype==TokenType::identifier) {// if lhs isn't an id it's gonna fail
                for (auto att: aSchema.getAttributeList()) {
                    //check that name matches some attribute from schema and save the dataType
                    if (exp.lhs.name == att.getName()) {
                        expressionType = att.getType();
                        validLeft = true;

                    }
                }
            }
            //check right hand side
            if (exp.rhs.ttype==TokenType::identifier){
                for(auto att : aSchema.getAttributeList()){
                    //check if name matches some attribute from schema and if dataTypes match
                    if(exp.rhs.name==att.getName() && att.getType()==expressionType){//comparing two columns
                        validRight=true;
                    }
                    else{
                        //the rhs is either a bool or a string
                        if(exp.rhs.name=="true"||exp.rhs.name=="false"){
                            exp.rhs.dtype=DataTypes::bool_type;
                        }
                        else{
                            exp.rhs.ttype=TokenType::string;
                            exp.rhs.dtype=DataTypes::varchar_type;
                        }
                    }
                }
            }// a number is also valid entry
            else if(exp.rhs.ttype==TokenType::number){
                if(expressionType==DataTypes::int_type || expressionType==DataTypes::float_type){
                    validRight=true;
                }
            }
            //if either is bad, don't check the rest of the expressions
            if(!validLeft || !validRight){return false;}
        }
        // if you make it to the end it's a valid query
        return true;
    }

    bool Validation::validateSetFields(ECE141::DBQuery &aQuery, ECE141::Schema &aSchema) {
        if(aQuery.getSetField().empty()){return true;}
        auto exp = aQuery.getSetField();
        for(auto att : aSchema.getAttributeList()) {
            if(exp == att.getName()){
                return true;
            }
        }
        return false;
    }

    bool Validation::validateSelectFields(DBQuery &aQuery, Schema &aSchema) {
        bool badSelect = true;
        if(aQuery.getIfAllFields()){return true;}
        for(auto id : aQuery.getSelectFields()) {//loop where conditions
            for(auto att : aSchema.getAttributeList()) {
                if(id == att.getName()){
                    badSelect = false;
                }
            }
            if(badSelect){return false;}
        }
        return true;
    }

    bool Validation::validateOrderBy(DBQuery &aQuery, Schema &aSchema) {
        // if the order by field is a valid Identifier then it passes
        if(aQuery.getOrderByField().empty()){return true;}
        for(auto att : aSchema.getAttributeList()) {
            if(aQuery.getOrderByField()==att.getName()){
                return true;
            }
        }
        return false;
    }

    bool Validation::validateGroupBy(DBQuery &aQuery, Schema &aSchema) {
        return true;
    }

    bool Validation::validateJoinFields(ECE141::DBQuery &aQuery, ECE141::Schema &aSchema) {
        return true;
    }
}