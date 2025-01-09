//
//  Filters.hpp
//  RGAssignment5
//
//  Created by rick gessner on 4/4/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#ifndef Filters_h
#define Filters_h

#include <cstdio>
#include <vector>
#include <memory>
#include <string>
#include "misc/BasicTypes.hpp"
#include "tokenizer/Tokenizer.hpp"

namespace ECE141 {

    using KeyValues = std::map<std::string, Value>;

    struct Operand {
        Operand() {}

        Operand(std::string aName, TokenType aType, Value aValue, size_t anId = 0)
                : ttype(aType), dtype(DataTypes::varchar_type), name(aName),
                  value(aValue), schemaId(anId) {}

        bool operator==(const Operand& other) const {
            return ttype == other.ttype &&
                   dtype == other.dtype &&
                   name == other.name &&
                   value == other.value &&
                   schemaId == other.schemaId;
        }

        TokenType ttype; //is it a field, or const (#, string)...
        DataTypes dtype;
        std::string name;  //attr name
        Value value;
        size_t schemaId;
    };

    //---------------------------------------------------

    struct Expression {
        Operand lhs;  //id
        Operand rhs;  //usually a constant; maybe a field...
        Operators op;   //=     //users.id=books.author_id
        Logical logic; //and, or, not...

        Expression(const Operand &aLHSOperand = {},
                   const Operators anOp = Operators::unknown_op,
                   const Operand &aRHSOperand = {})
                : lhs(aLHSOperand), rhs(aRHSOperand),
                  op(anOp), logic(Logical::no_op) {}

        Expression(const Expression &anExpr) :
                lhs(anExpr.lhs), rhs(anExpr.rhs),
                op(anExpr.op), logic(anExpr.logic) {}

        bool operator==(const Expression& other) const {
            return lhs == other.lhs &&
                   rhs == other.rhs &&
                   op == other.op &&
                   logic == other.logic;
        }

        bool operator()( /* args */);
    };

    using Expressions = std::vector<Expression>;

    //---------------------------------------------------

    class Filters {
    public:

        Filters();

        Filters(const Filters &aFilters);

        ~Filters();

        size_t getCount() const { return expressions.size(); }

        bool matches(KeyValues &aList) const;

        Filters &add(Expression *anExpression);

        Expressions &getExpressions() { return expressions; }

//        StatusResult parse(Tokenizer &aTokenizer, Schema &aSchema);

        bool operator==(const Filters& other) const {
            return expressions == other.expressions;
        }

    protected:
        Expressions expressions;
    };

}

#endif /* Filters_h */
