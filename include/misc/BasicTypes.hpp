//
//  BasicTypes.hpp
//  RGAssignement1
//
//  Created by rick gessner on 3/30/23.
//  Copyright © 2018-2023 rick gessner. All rights reserved.
//

#ifndef BasicTypes_h
#define BasicTypes_h

#include <optional>
#include <string>
#include <variant>
#include <map>
#include <unordered_map>
#include <vector>


namespace ECE141 {

        enum class DataTypes {
            no_type = 'N', bool_type = 'B', datetime_type = 'D',
            float_type = 'F', int_type = 'I', varchar_type = 'V',
        };

    using OptString = std::optional<std::string_view>;
    using Value = std::variant<bool, int, float, std::string>;
    using RowKeyValues = std::unordered_map<std::string, Value>;
    using StringVector = std::vector<std::string>;

    using OptString = std::optional<std::string_view>;

    template <typename CRTP>
    class AsSingleton {
    public:
        static CRTP& getInstance() {
            static CRTP theInstance;
            return theInstance;
        }
    };



}
#endif /* BasicTypes_h */
