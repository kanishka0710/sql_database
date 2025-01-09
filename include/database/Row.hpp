#ifndef Row_hpp
#define Row_hpp

#include <cstdio>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <memory>
#include "Attribute.hpp"
#include "misc/BasicTypes.hpp"
#include "storage/Storage.hpp"
#include "Schema.hpp"

//feel free to use this, or create your own version...

namespace ECE141 {

    class Row : public Storable {
    public:
        Row();
        Row(uint32_t entityId);
        Row(uint32_t entityId, uint32_t blockNumber, uint32_t dataLength, const RowKeyValues &data, const Schema &dataSchema);
        Row(const Row &aRow);

        // Row(const Attribute &anAttribute); //maybe?

        ~Row() = default;

        Row &operator=(const Row &aRow);
        bool operator==(Row &aCopy) const;

        size_t getBinarySize() const override;
        StatusResult encode(BinaryBuffer &output) const override;
        StatusResult decode(BinaryBuffer &input) override;
        bool initHeader(Block &aBlock) const override;

        //STUDENT: What other methods do you require?

        Row &set(const std::string &aKey, const Value &aValue);
        StatusResult setSchema(Schema& aSchema);
        StatusResult setField(std::string &field, Value &val);

        RowKeyValues &getData() { return data; }

        uint32_t entityId;
        uint32_t blockNumber;
        uint32_t dataLength;

    protected:
        RowKeyValues data;
        Schema dataSchema;
    };

    //-------------------------------------------

    using RowCollection = std::vector<std::unique_ptr<Row>>;

}
#endif /* Row_hpp */
