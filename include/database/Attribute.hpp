//
//  Attribute.hpp
//  PA3
//
//  Created by rick gessner on 4/18/22.
//  Copyright © 2023 rick gessner. All rights reserved.
//

#ifndef Attribute_hpp
#define Attribute_hpp

#include <cstdio>
#include <string>
#include <vector>
#include <optional>
#include "tokenizer/keywords.hpp"
#include "misc/BasicTypes.hpp"
#include "tokenizer/Tokenizer.hpp"
#include "storage/BinaryBuffer.h"

namespace ECE141 {


    class Attribute {
    protected:
        std::string   name;
        DataTypes     type;
        uint32_t      length;
        bool          auto_increment;
        bool          primary_key;
        bool          nullable;
        std::string   defaultValue;


    public:
        /*Attribute(DataTypes aType=DataTypes::no_type, std::string aDefaultValue="");
        Attribute(const std::string& aName, DataTypes aType, uint32_t aSize=0, std::string aDefaultValue="");*/

        Attribute() = default;
        Attribute(const Attribute &aCopy);
        ~Attribute();

        bool operator<(const Attribute& rhs) const {
            return std::tie(name, type, length, auto_increment, primary_key, nullable, defaultValue)
                   < std::tie(rhs.name, rhs.type, rhs.length, rhs.auto_increment, rhs.primary_key, rhs.nullable, rhs.defaultValue);
        }

        bool operator==(const Attribute& rhs) const {
            return std::tie(name, type, length, auto_increment, primary_key, nullable, defaultValue)
                   == std::tie(rhs.name, rhs.type, rhs.length, rhs.auto_increment, rhs.primary_key, rhs.nullable, rhs.defaultValue);
        }

        bool operator!=(const Attribute& rhs) const {
            return !(*this == rhs);
        }

        bool operator>(const Attribute& rhs) const {
            return rhs < *this;
        }

        bool operator<=(const Attribute& rhs) const {
            return !(*this > rhs);
        }

        bool operator>=(const Attribute& rhs) const {
            return !(*this < rhs);
        }


        bool setAttributes(std::vector<ECE141::Token> &aStatement, ECE141::StatusResult &aResult);
        const std::string& getName() const {return name;};
        DataTypes getType() const {return type;};
        uint32_t getLength() const {return length;}
        const std::string& getDefaultValue() const {return defaultValue;}
        bool isAutoIncrement() const { return auto_increment;}
        bool isPrimaryKey() const { return primary_key;}
        bool isNullable() const { return nullable;}
        bool isIndexed() const;
        std::optional<std::pair<std::string, std::string>> getForeignKey() const;

        // Setters...
        Attribute& setDefaultValue(const std::string& aValue);
        Attribute& setIndexed(bool aValue);
        Attribute& setForeignKey(const std::string& aTable, const std::string& aColumn);
        bool setName(ECE141::Token &aToken, ECE141::StatusResult &aResult);
        bool setType(ECE141::Token &aToken, ECE141::StatusResult &aResult);
        bool setType(ECE141::Token &aType, ECE141::Token &aLength,  ECE141::StatusResult &aResult);
        bool setAutoIncrement(ECE141::Token &aToken);
        bool setPrimaryKey(Token &aPrimary, Token &aKey);
        bool setNullable(Token &aBoolean);



        Attribute& deserializeData(BinaryBuffer& input);

        bool isValid() const; // Validate based on rules you define...
        std::string toLowercase(const std::string &input);

        bool handleConstraints(std::vector<ECE141::Token> &vector, StatusResult &result);
    };

    using AttributeOpt = std::optional<Attribute>;
    using AttributeList = std::vector<Attribute>;

}


#endif /* Attribute_hpp */