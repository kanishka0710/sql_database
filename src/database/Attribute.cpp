//
//  Attribute.hpp
//  PA3
//
//  Created by rick gessner on 4/18/22.
//  Copyright © 2023 rick gessner. All rights reserved.
//

#include "database/Attribute.hpp"
#include "storage/BinaryBuffer.h"

namespace ECE141 {

    Attribute::Attribute(const Attribute &aCopy)
            : name(aCopy.name),
              type(aCopy.type),
              length(aCopy.length),
              auto_increment(aCopy.auto_increment),
              primary_key(aCopy.primary_key),
              nullable(aCopy.nullable),
              defaultValue(aCopy.defaultValue) {}

    Attribute::~Attribute() {
    }


    bool Attribute::setAttributes(std::vector<ECE141::Token> &aStatement, StatusResult &aResult) {
        int index = 0;
        //Add name of attribute (mandatory).
        if (aStatement.empty() || !setName(aStatement[index], aResult)) return false;
        aStatement.erase(aStatement.begin());//pop name because it is stored in attribute instance

        //Add type (mandatory).
        if (!aStatement.empty() && toLowercase(aStatement[index].data) == "varchar") {
            if (!setType(aStatement[index], aStatement[index + 2], aResult)) return false;
            aStatement.erase(aStatement.begin(), aStatement.begin() + 4);
        } else if (aStatement.empty() || !setType(aStatement[index], aResult)) return false;
        else aStatement.erase(aStatement.begin()); //if set type success then delete from vector.

        //optional constraints (chain of responsibility)
        if (!handleConstraints(aStatement, aResult)) return false;
        else return  true;

    }

    bool hasOnlyLetters(const std::string& str) {
        for (char c : str) {
            if (isdigit(c)) {
                return false;
            }
        }
        return true;

    }
    bool Attribute::setName(ECE141::Token &aToken, ECE141::StatusResult &aResult) {
        if(hasOnlyLetters(aToken.data)){
            name = aToken.data;
            return true;
        }
        else {
            aResult=Errors::invalidAttribute;
            return false;
        }
        //TODO: what are potential error for the column name of the table.
    }

    std::string Attribute::toLowercase(const std::string &input) {
        std::string result = input; // Make a copy of the input to transform
        for (char &c: result) {
            c = std::tolower(static_cast<unsigned char>(c)); // Use std::tolower on each character
        }
        return result;
    }

    bool Attribute::setType(Token &aType, StatusResult &aResult) {
        if ("int" == toLowercase(aType.data)) type = DataTypes::int_type;
        else if ("float" == toLowercase(aType.data)) type = DataTypes::float_type;
        else if ("timestamp" == toLowercase(aType.data)) type = DataTypes::datetime_type;
        else if ("boolean" == toLowercase(aType.data)) type = DataTypes::bool_type;
        else {
            aResult = Errors::unknownType;
            return false;
        }

        return true;
    }

    bool Attribute::setType(Token &aType, Token &aLength, StatusResult &aResult) {
        if ("varchar" == toLowercase(aType.data)) type = DataTypes::varchar_type;
        try {
            length = std::stoi(aLength.data);
        } catch (const std::invalid_argument &e) {
            aResult = Errors::invalidArguments;
            return false;
        }
        return true;
    }

    bool Attribute::handleConstraints(std::vector<ECE141::Token> &constraints, StatusResult &result) {
        for (size_t i = 0; i < constraints.size(); i++) {
            if ("primarykey" == toLowercase(constraints[i].data + constraints[i + 1].data)) {
                setPrimaryKey(constraints[i], constraints[i + 1]);
                i++;
                continue;
            } else if ("notnull" == toLowercase(constraints[i].data + constraints[i + 1].data)) {
                constraints[i].data += constraints[i + 1].data;
                setNullable(constraints[i]);
                i++;
                continue;
            } else if ("null" == toLowercase(constraints[i].data)) {
                setNullable(constraints[i]);
                continue;
            } else if ("auto_increment" == toLowercase(constraints[i].data)) {
                setAutoIncrement(constraints[i]);
            } else {
                result = Errors::invalidAttribute;
                return false;
            }

        }
        return true;
    }

    bool Attribute::setAutoIncrement(Token &aToken) {
        if ("auto_increment" == toLowercase(aToken.data)) {
            auto_increment = true;
            return true;
        } else return false;
    }

    bool Attribute::setPrimaryKey(Token &aPrimary, Token &aKey) {
        if ("primary" == toLowercase(aPrimary.data) && "key" == toLowercase(aKey.data)) {
            primary_key = true;
            return true;
        } else return false;
    }

    bool Attribute::setNullable(Token &aNull) {
        if ("notnull" == toLowercase(aNull.data)) {
            nullable = false;
            return true;
        } else if ("null" == toLowercase(aNull.data)) {
            nullable = true;
            return true;
        }
        return false;
    }

    Attribute& Attribute::deserializeData(BinaryBuffer& input) {
        name = input.readString();
        type = static_cast<DataTypes>(input.read<int>());
        length = input.read<uint32_t>();
        auto_increment = input.read<bool>();
        primary_key = input.read<bool>();
        nullable = input.read<bool>();
        defaultValue = input.readString();
        if (defaultValue == "*") defaultValue = "";
        return *this;
    }


}