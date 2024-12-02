//
// Copyright 2018 - 2024 (C). Alex Robenko. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "commsdsl/parse/Field.h"
#include "commsdsl/parse/Protocol.h"

#include <cstdint>
#include <vector>

namespace commsdsl
{

namespace parse
{

class DataFieldImpl;
class COMMSDSL_API DataField : public Field
{
    using Base = Field;
public:
    using ValueType = std::vector<std::uint8_t>;

    struct ValidValueInfo
    {
        ValueType m_value;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = Protocol::notYetDeprecated();
    };

    using ValidValuesList = std::vector<ValidValueInfo>;    

    explicit DataField(const DataFieldImpl* impl);
    explicit DataField(Field field);

    const ValueType& defaultValue() const;
    std::size_t fixedLength() const;
    bool hasLengthPrefixField() const;
    Field lengthPrefixField() const;
    const std::string& detachedPrefixFieldName() const;
    const ValidValuesList& validValues() const;
};

} // namespace parse

} // namespace commsdsl
