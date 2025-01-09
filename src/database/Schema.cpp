//
//  Schema.cpp
//  PA3
//
//  Created by rick gessner on 3/2/23.
//

#include "database/Schema.hpp"
#include "storage/BlockIO.hpp"
#include "storage/BinaryBuffer.h"
#include "misc/Helpers.hpp"
#include <cmath>
#include <unordered_set>

namespace ECE141 {

    //STUDENT: Implement this class...

    Schema::Schema(const std::string aName) : name(aName) {}

    Schema::Schema(const Schema &aCopy) : name(aCopy.name), attributes(aCopy.attributes), attributeLength(aCopy.attributeLength) {}

    Schema::~Schema() = default;

    Schema& Schema::operator=(const Schema &aCopy) {
        if (this != &aCopy) {  // Avoid self-assignment
            name = aCopy.name;
            attributes = aCopy.attributes;
            attributeLength = aCopy.attributeLength;
        }
        return *this;
    }

    bool Schema::operator==(const Schema &aSchema) const {
        return name == aSchema.name &&
               attributeLength == aSchema.attributeLength &&
               attributes == aSchema.attributes;
    }


    void Schema::setAttributes(AttributeList &attributeList) {
        attributes = attributeList;
        attributeLength = attributeList.size();
    }

    size_t Schema::getBinarySize() const {
        size_t res = 0;
        res += name.size();
        res += sizeof(attributeLength);
        for (auto &it : attributes) {
            res += it.getName().size();
            res += sizeof(it.getType());
            res += sizeof(it.getLength());
            res += sizeof(it.isAutoIncrement());
            res += sizeof(it.isPrimaryKey());
            res += sizeof(it.isNullable());
            res += it.getDefaultValue().size();
        }
        return res;
    }

    StatusResult Schema::encode(BinaryBuffer &output) const  {
        output.writeString(name);
        output.write<size_t>(attributeLength);
        for (auto& it : attributes) {
            output.writeString(it.getName());
            output.write<DataTypes>(it.getType());
            output.write<uint32_t>(it.getLength());
            output.write<bool>(it.isAutoIncrement());
            output.write<bool>(it.isPrimaryKey());
            output.write<bool>(it.isNullable());
            output.writeString(it.getDefaultValue());
        }
        return Errors::noError;
    }

    StatusResult Schema::decode(BinaryBuffer &input)  {
        name = input.readString();
        attributeLength = input.read<size_t>();
        Attribute attribute;
        for (size_t i = 0; i < attributeLength; i++) {
            attribute.deserializeData(input);
            attributes.push_back(attribute);
        }
        return Errors::noError;
    }

    bool Schema::initHeader(Block &aBlock) const {
        aBlock.header.type = static_cast<char>(BlockType::schema_block);
        return true;
    }

    bool Schema::validateAttributes(std::vector<Attribute>& values) {
        std::unordered_set<std::string> attributeNames;
        for (const auto& attribute : attributes) {
            attributeNames.insert(attribute.getName());
        }
        for (const auto& value : values) {
            if (attributeNames.find(value.getName()) == attributeNames.end()) {
                return false;
            }
        }
        return true;
    }

    std::optional<Attribute> Schema::findAttributeByName(std::string attributeName) {
        for (auto& it : attributes) {
            if (it.getName() == attributeName) {
                return it;
            }
        }
        return std::nullopt;
    }

    StringVector Schema::getAllAttributeNames() {
        StringVector attributeNames;
        for (auto& it : attributes) {
            attributeNames.push_back(it.getName());
        }
        return attributeNames;
    }

    StringVector Schema::getAttributeNames(DBQuery &aQuery) {
        StringVector attributeNames;
        auto selectFields = aQuery.getSelectFields();
        for (auto& it : attributes) {
            if (!selectFields.empty() && std::find(selectFields.begin(), selectFields.end(), it.getName()) != selectFields.end()) {
                attributeNames.push_back(it.getName());
//            } else if (!aQuery.getJoin().empty()){
//                if (aQuery.getJoin().onLeft.table == name && it.getName() == aQuery.getJoin().onLeft.fieldName) {
//                    attributeNames.push_back(it.getName());
//                } else if (aQuery.getJoin().onRight.table == name && it.getName() == aQuery.getJoin().onRight.fieldName) {
//                    attributeNames.push_back(it.getName());
//                }
            } else if (aQuery.getIfAllFields()){
                attributeNames.push_back(it.getName());
            }
        }
        return attributeNames;
    }

    std::string Schema::attributesAsString() {
        std::string output;
        for (auto& it : attributes) {
            output.append(it.getName() + " ");
            output.append(Helpers::dataTypeToString(it.getType()));
            if (it.getLength() && DataTypes::varchar_type == it.getType()) {
                output.append("(" + std::to_string(it.getLength())  + ") ");
            } else {
                output.append(" ");
            }
            if (it.isAutoIncrement()) {
                output.append("AUTO_INCREMENT ");
            }
            if (it.isPrimaryKey()) {
                output.append("PRIMARY KEY ");
            }
            if (!it.isNullable()) {
                output.append("NOT NULL ");
            }
            if(!it.getDefaultValue().empty()) {
                output.append("DEFAULT" + it.getDefaultValue());
            }
            output.append(", ");
        }
        return output;
    }
}
