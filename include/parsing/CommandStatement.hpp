//
// Created by aryam on 28-04-2024.
//

#ifndef ECE141DB_COMMANDSTATEMENT_HPP
#define ECE141DB_COMMANDSTATEMENT_HPP

#include <vector>
#include <unordered_map>
#include <variant>
#include <string>
#include "database/Attribute.hpp"
#include "tokenizer/Tokenizer.hpp"
#include "misc/DBQuery.hpp"

namespace ECE141 {

    struct AttributeValuePairs {
        Attribute anAttribute;
        std::variant<bool, int, float, std::string> value;

        bool operator==(const AttributeValuePairs &other) const {
            return anAttribute == other.anAttribute && value == other.value;
        }
    };

    struct CommandStatement {
        std::vector<ECE141::Attribute> attributeList;
        std::vector<ECE141::Token> statement;
        std::vector<std::vector<AttributeValuePairs>> values;
        DBQuery aQuery{};

        bool operator==(const CommandStatement &other) const {
            return attributeList == other.attributeList &&
                   statement == other.statement &&
                   values == other.values &&
                   aQuery == other.aQuery;
        }

    };
}

    namespace std {
        template<>
        struct hash<ECE141::CommandStatement> {
            size_t operator()(const ECE141::CommandStatement &cs) const {
                size_t hashValue = 0;
                // Hash the attributeList
                for (const auto &attr: cs.attributeList) {
                    hashValue ^= std::hash<std::string>{}(attr.getName()) +
                                 std::hash<int>{}(static_cast<int>(attr.getType())) +
                                 std::hash<uint32_t>{}(attr.getLength()) +
                                 std::hash<bool>{}(attr.isAutoIncrement()) +
                                 std::hash<bool>{}(attr.isPrimaryKey()) +
                                 std::hash<bool>{}(attr.isNullable()) +
                                 std::hash<std::string>{}(attr.getDefaultValue());
                }

                // Hash the statement
                for (const auto &token: cs.statement) {
                    hashValue ^= std::hash<int>{}(static_cast<int>(token.type)) +
                                 std::hash<int>{}(static_cast<int>(token.keyword)) +
                                 std::hash<std::string>{}(token.data);
                }

                // Hash the values
                for (const auto &vec: cs.values) {
                    for (const auto &pair: vec) {
                        hashValue ^= std::hash<std::string>{}(pair.anAttribute.getName());
                        // Assuming that variant holds at most one active value at a time
                        std::visit([&](auto &&arg) {
                            hashValue ^= std::hash<std::decay_t<decltype(arg)>>{}(arg);
                        }, pair.value);
                    }
                }

                // Hash the aQuery (simplified example, assuming getTableName and other getters are defined)
                hashValue ^= std::hash<std::string>{}(cs.aQuery.getTableName());

                return hashValue;
            }
        };
    }

#endif //ECE141DB_COMMANDSTATEMENT_HPP
