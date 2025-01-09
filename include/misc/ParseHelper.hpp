//
//  ParseHelper.hpp
//  RGAssignment4
//
//  Created by rick gessner on 4/18/21.
//

#ifndef ParseHelper_hpp
#define ParseHelper_hpp

#include <cstdio>
#include "tokenizer/keywords.hpp"
#include "BasicTypes.hpp"
#include "tokenizer/Tokenizer.hpp"
#include "database/Attribute.hpp"
#include "Filters.hpp"
#include "database/Schema.hpp"

namespace ECE141 {
  
  class Entity;
  struct Expression;
  using StringList = std::vector<std::string>;
  
  struct ParseHelper {
            
    ParseHelper(Tokenizer &aTokenizer) : tokenizer(aTokenizer) {}
        
    StatusResult parseTableName(TableName &aTableName);
    StatusResult parseTableField(Tokenizer &aTokenizer, TableField &aField);
    StatusResult parseAttributeOptions(Attribute &anAttribute);

    StatusResult parseAttribute(Attribute &anAttribute);

    StatusResult parseIdentifierList(StringList &aList);

    StatusResult parseAssignments(Expressions &aList, Schema &aSchema);
    
    StatusResult parseValueList(StringList &aList);
            
    StatusResult parseOperator(Operators &anOp);
    StatusResult parseOperand(Schema&, Operand&);
    StatusResult parseExpression(Schema&, Expression&);

    Tokenizer &tokenizer;
  };

}

#endif /* ParseHelper_hpp */

