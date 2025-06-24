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

#include "commsdsl/parse/ParseEndian.h"
#include "commsdsl/parse/ParseIntField.h"
#include "commsdsl/parse/ParseProtocol.h"

namespace commsdsl
{

namespace parse
{

class ParseEnumFieldImpl;
class COMMSDSL_API ParseEnumField : public ParseField
{
    using Base = ParseField;
public:

    struct ValueInfo
    {
        std::intmax_t m_value = 0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = ParseProtocol::parseNotYetDeprecated();
        std::string m_description;
        std::string m_displayName;
    };

    using Type = ParseIntField::Type;
    using Values = std::map<std::string, ValueInfo>;
    using RevValues = std::multimap<std::intmax_t, std::string>;

    explicit ParseEnumField(const ParseEnumFieldImpl* impl);
    explicit ParseEnumField(ParseField field);

    Type parseType() const;
    ParseEndian parseEndian() const;
    std::intmax_t parseDefaultValue() const;
    const Values& parseValues() const;
    const RevValues& parseRevValues() const;
    bool parseIsNonUniqueAllowed() const;
    bool parseIsUnique() const;
    bool parseValidCheckVersion() const;
    bool parseHexAssign() const;
    bool parseAvailableLengthLimit() const;
};

inline
bool operator==(const ParseEnumField::ValueInfo& i1, const ParseEnumField::ValueInfo& i2)
{
    return (i1.m_value == i2.m_value) &&
           (i1.m_sinceVersion == i2.m_sinceVersion) &&
           (i1.m_deprecatedSince == i2.m_deprecatedSince);
}

} // namespace parse

} // namespace commsdsl
