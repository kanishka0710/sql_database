//
//  Schema.hpp
//  PA3
//
//  Created by rick gessner on 3/18/23.
//  Copyright © 2023 rick gessner. All rights reserved.
//

#ifndef Schema_hpp
#define Schema_hpp

#include <memory>
#include <string>

#include "Attribute.hpp"
#include "misc/Errors.hpp"
#include "storage/BinaryBuffer.h"
#include "storage/Storage.hpp"
#include "parsing/CommandStatement.hpp"


namespace ECE141 {

    class Schema : public Storable {
    public:
        Schema() = default;
        Schema(const std::string aName);

        Schema(const Schema &aCopy);

        Schema& operator=(const Schema &aCopy);

        bool operator==(const Schema &aSchema) const;

        ~Schema();

        const std::string &getName() const { return name; }

        void setAttributes(AttributeList &attributeList);

        AttributeList getAttributeList() {return attributes;}

        StringVector getAllAttributeNames();
        StringVector getAttributeNames(DBQuery &aQuery);

        size_t getBinarySize() const override;

        StatusResult encode(BinaryBuffer &output) const override;

        StatusResult decode(BinaryBuffer &input) override;

        bool initHeader(Block &aBlock) const override;

        bool validateAttributes(std::vector<Attribute>& values);

        std::optional<Attribute> findAttributeByName(std::string attributeName);

        std::string attributesAsString();

    protected:
        AttributeList attributes;
        size_t attributeLength;
        std::string name;
    };

}
#endif /* Schema_hpp */
