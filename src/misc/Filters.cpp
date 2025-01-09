//
//  Filters.cpp
//  Datatabase5
//
//  Created by rick gessner on 3/5/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#include <string>
#include <limits>
#include "misc/Filters.hpp"
#include "tokenizer/keywords.hpp"
#include "misc/Helpers.hpp"
#include "database/Schema.hpp"
#include "database/Attribute.hpp"
#include "misc/ParseHelper.hpp"
//#include "Compare.hpp"

namespace ECE141 {

    using Comparitor = bool (*)(Value &aLHS, Value &aRHS);

    bool equals(Value &aLHS, Value &aRHS) {
        bool theResult = false;

        std::visit([&](auto const &aLeft) {
            std::visit([&](auto const &aRight) {
                //theResult=isEqual(aLeft,aRight);
            }, aRHS);
        }, aLHS);
        return theResult;
    }

    static std::map<Operators, Comparitor> comparitors{
            {Operators::equal_op, equals},
            //STUDENT: Add more for other operators...
    };

    bool Expression::operator()(/* args */) {
        //STUDENT: Add code here to evaluate the expression...
        return false;
    }

    //--------------------------------------------------------------

    Filters::Filters() = default;

    Filters::Filters(const Filters &aCopy) {
        this->expressions.clear();
        for (auto &it: aCopy.expressions) {
            this->expressions.push_back(it);
        }
    }

    Filters::~Filters() = default;

    Filters &Filters::add(Expression *anExpression) {
//        expressions.push_back(std::unique_ptr<Expression>(anExpression));
        return *this;
    }

    //compare expressions to row; return true if matches
    bool Filters::matches(KeyValues &aList) const {

        //STUDENT: You'll need to add code here to deal with
        //         logical combinations (AND, OR, NOT):
        //         like:  WHERE zipcode=92127 AND age>20

//        for (auto &theExpr: expressions) {
//            if (!(*theExpr)(aList)) {
//                return false;
//            }
//        }
        return true;
    }


    //where operand is field, number, string...
    StatusResult parseOperand(Tokenizer &aTokenizer,
                              Schema &aSchema, Operand &anOperand) {
    StatusResult theResult{Errors::noError};
//    Token &theToken = aTokenizer.current();
//    if(TokenType::identifier==theToken.type) {
//      if(auto *theAttr=aSchema.getAttribute(theToken.data)) {
//        anOperand.ttype=theToken.type;
//        anOperand.name=theToken.data; //hang on to name...
//        anOperand.schemaId=aSchema::hashString(theToken.data);
//        anOperand.dtype=theAttr->getType();
//      }
//      else {
//        anOperand.ttype=TokenType::string;
//        anOperand.dtype=DataTypes::varchar_type;
//        anOperand.value=theToken.data;
//      }
//    }
//    else if(TokenType::number==theToken.type) {
//      anOperand.ttype=TokenType::number;
//      anOperand.dtype=DataTypes::int_type;
//      if (theToken.data.find('.')!=std::string::npos) {
//        anOperand.dtype=DataTypes::float_type;
//        anOperand.value=std::stof(theToken.data);
//      }
//      else anOperand.value=std::stoi(theToken.data);
//    }
//    else theResult.error=syntaxError;
//    if(theResult) aTokenizer.next();
//    return theResult;
        return theResult;
    }

    //STUDENT: Add validation here...
    bool validateOperands(Operand &aLHS, Operand &aRHS, Schema &aSchema) {
        if (TokenType::identifier == aLHS.ttype) { //most common case...
            //STUDENT: Add code for validation as necessary
            return true;
        } else if (TokenType::identifier == aRHS.ttype) {
            //STUDENT: Add code for validation as necessary
            return true;
        }
        return false;
    }

    bool isValidOperand(Token &aToken) {
        //identifier, number, string...
        if (aToken.type == TokenType::identifier) return true;
        return false;
    }

    //STUDENT: This starting point code may need adaptation...
//    StatusResult Filters::parse(Tokenizer &aTokenizer, Schema &aSchema) {
//        StatusResult theResult{Errors::noError};
////        ParseHelper theHelper(aTokenizer);
////        while (theResult && (2 < aTokenizer.remaining())) {
////            if (isValidOperand(aTokenizer.current())) {
////                Expression theExpr;
////                if ((theResult = theHelper.parseExpression(aSchema, theExpr))) {
////                    expressions.push_back(theExpr);
////                    //add logic to deal with bool combo logic...
////                }
////            } else break;
////        }
//        return theResult;
//    }

}

