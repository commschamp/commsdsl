//
// Copyright 2018 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/ParseField.h"
#include "commsdsl/parse/ParseProtocol.h"

#include <cstdint>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseDataFieldImpl;
class COMMSDSL_API ParseDataField : public ParseField
{
    using Base = ParseField;
public:
    using ParseValueType = std::vector<std::uint8_t>;

    struct ParseValidValueInfo
    {
        ParseValueType m_value;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = ParseProtocol::parseNotYetDeprecated();
    };

    using ParseValidValuesList = std::vector<ParseValidValueInfo>;    

    explicit ParseDataField(const ParseDataFieldImpl* impl);
    explicit ParseDataField(ParseField field);

    const ParseValueType& parseDefaultValue() const;
    std::size_t parseFixedLength() const;
    bool parseHasLengthPrefixField() const;
    ParseField parseLengthPrefixField() const;
    const std::string& parseDetachedPrefixFieldName() const;
    const ParseValidValuesList& parseValidValues() const;
};

} // namespace parse

} // namespace commsdsl
