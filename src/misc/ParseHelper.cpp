//
//  ParseHelpers.cpp
//  RGAssignment4
//
//  Created by rick gessner on 4/18/21.
//

#include "misc/ParseHelper.hpp"
#include "misc/Helpers.hpp"
#include "database/Schema.hpp"
#include "database/Database.hpp"

namespace ECE141 {

// USE: parse table name (identifier) with (optional) alias...
    StatusResult ParseHelper::parseTableName(TableName &aTableName) {

        StatusResult theResult{Errors::identifierExpected};
        Token &theToken = tokenizer.current(); //get token (should be identifier)
        if (TokenType::identifier == theToken.type) {
            aTableName.table = theToken.data;
            theResult.error = Errors::noError;
            tokenizer.next(); //skip ahead...
            if (tokenizer.skipIf(Keywords::as_kw)) { //try to skip 'as' for alias...
                theToken = tokenizer.current();
                aTableName.alias = theToken.data; //copy alias...
                tokenizer.next(); //skip past alias...
            }
        }
        return theResult;
    }

    StatusResult ParseHelper::parseTableField(Tokenizer &aTokenizer,
                                              TableField &aField) {
        StatusResult theResult{Errors::identifierExpected};
        Token &theToken = aTokenizer.current(); //identifier name?
        if (TokenType::identifier == theToken.type) {
            aField.fieldName = theToken.data;
            aTokenizer.next();
            if (aTokenizer.skipIf(Operators::dot_op)) {
                theToken = aTokenizer.current();
                if (TokenType::identifier == theToken.type) {
                    aTokenizer.next(); //yank it...
                    aField.table = aField.fieldName;
                    aField.fieldName = theToken.data;
                }
            }
            theResult.error = Errors::noError;
        }
        return theResult;
    }

    // USE: gets properties following the type in an attribute decl...
    StatusResult ParseHelper::parseAttributeOptions(Attribute &anAttribute) {
//    bool          options=true;
    StatusResult  theResult{Errors::noError};
//    char          thePunct[]={"),"}; //removed semi?
//
//    while(theResult && options && tokenizer.more()) {
//      Token &theToken=tokenizer.current();
//      switch(theToken.type) {
//        case TokenType::keyword:
//          switch(theToken.keyword) {
//            case Keywords::auto_increment_kw:
//              anAttribute.setAutoIncrement(true);
//              break;
//            case Keywords::primary_kw:
//              anAttribute.setPrimaryKey(true);
//              break;
//            case Keywords::not_kw:
//              tokenizer.next();
//              theToken=tokenizer.current();
//              if(Keywords::null_kw==theToken.keyword) {
//                anAttribute.setNullable(false);
//              }
//              else theResult.error=Errors::syntaxError;
//              break;
//
//            default: break;
//          }
//          break;
//
//        case TokenType::punctuation: //fall thru...
//          options=!in_array<char>(thePunct,theToken.data[0]);
//          if(semicolon==theToken.data[0])
//            theResult.error=Errors::syntaxError;
//          break;
//
//        default:
//          options=false;
//          theResult.error=Errors::syntaxError;
//      } //switch
//      if(theResult) tokenizer.next(); //skip ahead...
//    } //while
    return theResult;
    }

    //USE : parse an individual attribute (name type [options])
    StatusResult ParseHelper::parseAttribute(Attribute &anAttribute) {
        StatusResult theResult{Errors::noError};
//
//    if(tokenizer.more()) {
//      Token &theToken=tokenizer.current();
//      if(Helpers::isDatatype(theToken.keyword)) {
//        DataTypes theType = Helpers::getTypeForKeyword(theToken.keyword);
//        anAttribute.setDataType(theType);
//        tokenizer.next();
//
//        if(DataTypes::varchar_type==theType) {
//          if((tokenizer.skipIf(left_paren))) {
//            theToken=tokenizer.current();
//            tokenizer.next();
//            if((tokenizer.skipIf(right_paren))) {
//              anAttribute.setSize(atoi(theToken.data.c_str()));
//              // return theResult;
//            }
//          }
//        }
//
//        if(theResult) {
//          theResult=parseAttributeOptions(anAttribute);
//          if(theResult) {
//            if(!anAttribute.isValid()) {
//              theResult.error=Errors::invalidAttribute;
//            }
//          }
//        }
//
//      } //if
//      else theResult.error=Errors::unknownType;
//    } //if
    return theResult;
    }

    //USE: parse a comma-sep list of (unvalidated) identifiers;
    //     AUTO stop if keyword (or term)
    StatusResult ParseHelper::parseIdentifierList(StringList &aList) {
        StatusResult theResult;

        while (theResult && tokenizer.more()) {
            Token &theToken = tokenizer.current();
            if (TokenType::identifier == tokenizer.current().type) {
                aList.push_back(theToken.data);
                tokenizer.next(); //skip identifier...
                tokenizer.skipIf(comma);
            } else if (theToken.type == TokenType::keyword) {
                break; //Auto stop if we see a keyword...
            } else if (tokenizer.skipIf(right_paren)) {
                break;
            } else if (semicolon == theToken.data[0]) {
                break;
            } else theResult.error = Errors::syntaxError;
        }
        return theResult;
    }

    //** USE: get a list of values (identifiers, strings, numbers...)
    StatusResult ParseHelper::parseValueList(StringList &aList) {
        StatusResult theResult;

        while (theResult && tokenizer.more()) {
            Token &theToken = tokenizer.current();
            if (TokenType::identifier == theToken.type || TokenType::number == theToken.type) {
                aList.push_back(theToken.data);
                tokenizer.next(); //skip identifier...
                tokenizer.skipIf(comma);
            } else if (tokenizer.skipIf(right_paren)) {
                break;
            } else theResult.error = Errors::syntaxError;
        }
        return theResult;
    }

    bool isDotOperator(const Token &aToken) {
        if (TokenType::operators == aToken.type) {
            return aToken.op == Operators::dot_op;
        }
        return false;
    }

    const Attribute *getAttribute(Schema &aSchema, const std::string &aName,
                                  const std::string &aFieldName) {
//    Schema *theSchema=&aSchema;
//    if(aSchema.getName()!=aName) {
//      //theSchema=aSchema.getDatabase().getSchema(aName);
//    }
//    return theSchema ? theSchema->getAttribute(aFieldName) : nullptr;
        return nullptr;
    }

    //where operand is field, number, string...
    StatusResult ParseHelper::parseOperand(Schema &aSchema,
                                           Operand &anOp) {
        StatusResult theResult{Errors::noError};
//     Token &theToken =tokenizer.current();
//     if(TokenType::identifier==theToken.type) {
//       Token &theNext =tokenizer.peek();
//       std::string theEntityName(aSchema.getName());
//       if(isDotOperator(theNext)) {
//         theEntityName=theToken.data;
//         tokenizer.next(2);
//         theToken=tokenizer.current();
//       }
//       if(auto theAttr=getAttribute(aSchema,theEntityName,
//                                    theToken.data)) {
//         anOp.setAttribute(
//            theToken,aSchema.getHash(),theAttr->getType());
//       }
//       else anOp.setVarChar(theToken.data);
//     }
//     else if(TokenType::number==theToken.type) {
//       anOp.setNumber(theToken);
//     }
//     else theResult.error=Errors::syntaxError;
//     if(theResult) tokenizer.next();
     return theResult;
    }

    StatusResult ParseHelper::parseOperator(Operators &anOp) {
//     static Operators gOps[]={
//       Operators::equal_op, Operators::notequal_op,
//       Operators::lt_op, Operators::lte_op,
//       Operators::gt_op, Operators::gte_op
//     };
//
//     StatusResult theResult{Errors::operatorExpected};
//     Token &theToken=tokenizer.current();
//     if(in_array<Operators>(gOps, theToken.op)) {
//       anOp=theToken.op;
//       tokenizer.next();
//       theResult.error=Errors::noError;
//     }
     return StatusResult{Errors::noError};
    }

    StatusResult ParseHelper::parseExpression(Schema &aSchema,
                                              Expression &anExpr) {
        StatusResult theResult;

        if ((theResult = parseOperand(aSchema, anExpr.lhs))) {
            if ((theResult = parseOperator(anExpr.op))) {
                theResult = parseOperand(aSchema, anExpr.rhs);
            } else theResult.error = Errors::operatorExpected;
        }
        return theResult;
    }

    //read a comma-sep list of expressions...
    StatusResult ParseHelper::parseAssignments(Expressions &aList,
                                               Schema &aSchema) {
        StatusResult theResult;
        while (theResult && tokenizer.more()) {
            Expression theExpr;
            if ((theResult = parseExpression(aSchema, theExpr))) {
                aList.push_back(theExpr);
                if (!tokenizer.skipIf(',')) {
                    break;
                }
            } else theResult.error = Errors::syntaxError;
        }
        return theResult;
    }


}
