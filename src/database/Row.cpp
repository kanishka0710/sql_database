#include <cstdio>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <memory>
#include <unordered_map>
#include "database/Row.hpp"

namespace ECE141 {

    Row::Row()
            : entityId(0), blockNumber(0), dataLength(0), data(), dataSchema() {}

    Row::Row(uint32_t entityId)
            : entityId(entityId), blockNumber(0), dataLength(0), data(), dataSchema() {}

    Row::Row(uint32_t entityId, uint32_t blockNumber, uint32_t dataLength, const RowKeyValues &data, const Schema &dataSchema)
            : entityId(entityId), blockNumber(blockNumber), dataLength(dataLength), data(data), dataSchema(dataSchema) {}

    Row::Row(const Row &aRow)
            : entityId(aRow.entityId), blockNumber(aRow.blockNumber), dataLength(aRow.dataLength), data(aRow.data), dataSchema(aRow.dataSchema) {}

    Row &Row::operator=(const Row &aRow) {
        if (this != &aRow) {  // Avoid self-assignment
            entityId = aRow.entityId;
            blockNumber = aRow.blockNumber;
            dataLength = aRow.dataLength;
            data = aRow.data;
            dataSchema = aRow.dataSchema;
        }
        return *this;
    }

    bool Row::operator==(Row &aCopy) const {
        return entityId == aCopy.entityId && blockNumber == aCopy.blockNumber && dataLength == aCopy.dataLength && data == aCopy.data && dataSchema == aCopy.dataSchema;
    }

    Row &Row::set(const std::string &aKey, const Value &aValue) {
        data[aKey] = aValue;
        dataLength = data.size();
        return *this;
    }

    StatusResult Row::setSchema(Schema& aSchema) {
        dataSchema = aSchema;
        return StatusResult{Errors::noError};
    }

    StatusResult Row::setField(std::string &field, ECE141::Value &val) {
        for(auto pair : data){
            if(pair.first==field){
                pair.second = val;
            }
        }
        return StatusResult{Errors::noError};
    }

    bool Row::initHeader(Block &aBlock) const {
        aBlock.header.type = static_cast<char>(BlockType::data_block);
        return true;
    }

    size_t Row::getBinarySize() const {
        size_t res = 0;
        res += sizeof(entityId);
        res += sizeof(blockNumber);
        res += sizeof(dataLength);
        for (auto &pair : data) {
            res += pair.first.size();
            if (std::holds_alternative<std::string>(pair.second)) {
                res += std::get<std::string>(pair.second).size();
            } else {
                res += sizeof(pair.second);
            }
        }
        return res;
    }

    StatusResult Row::encode(BinaryBuffer &output) const {
        output.write(entityId);
        output.write(blockNumber);
        output.write(dataLength);
        for (auto& pair : data) {
            output.writeString(pair.first);
            if (std::holds_alternative<std::string>(pair.second)) {
                output.writeString(std::get<std::string>(pair.second));
            } else if (std::holds_alternative<bool>(pair.second)){
                output.writeString(std::to_string(std::get<bool>(pair.second)));
            } else if (std::holds_alternative<int>(pair.second)) {
                output.writeString(std::to_string(std::get<int>(pair.second)));
            } else if (std::holds_alternative<float>(pair.second)) {
                output.writeString(std::to_string(std::get<float>(pair.second)));
            }
        }
        return StatusResult{Errors::noError};
    }

    StatusResult Row::decode(BinaryBuffer &input) {
        entityId = input.read<uint32_t>();
        blockNumber = input.read<uint32_t>();
        dataLength = input.read<uint32_t>();

        std::unordered_map<DataTypes, std::function<void(const std::string&, const std::string&)>> decodeMap = {
                { DataTypes::varchar_type, [this](const std::string &key, const std::string &value) {
                    data[key] = value;
                }},
                { DataTypes::bool_type, [this](const std::string &key, const std::string &value) {
                    data[key] = (value == "true");
                }},
                { DataTypes::datetime_type, [this](const std::string &key, const std::string &value) {
                    data[key] = value;
                }},
                { DataTypes::float_type, [this](const std::string &key, const std::string &value) {
                    data[key] = std::stof(value);
                }},
                { DataTypes::int_type, [this](const std::string &key, const std::string &value) {
                    data[key] = std::stoi(value);
                }}
        };

        for (size_t i = 0; i < dataLength; i++) {
            std::string key = input.readString();
            std::string aValue = input.readString();
            std::optional<Attribute> attribute = dataSchema.findAttributeByName(key);

            if (attribute.has_value()) {
                auto it = decodeMap.find(attribute->getType());
                if (it != decodeMap.end()) {
                    it->second(key, aValue);
                } else {
                    return StatusResult{Errors::invalidAttribute};
                }
            } else {
                return StatusResult{Errors::invalidAttribute};
            }
        }
        return StatusResult{Errors::noError};
    }
}
