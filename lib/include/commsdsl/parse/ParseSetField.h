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

#include <string>

namespace commsdsl
{

namespace parse
{

class ParseSetFieldImpl;
class COMMSDSL_API ParseSetField : public ParseField
{
    using Base = ParseField;
public:

    using ParseType = ParseIntField::ParseType;
    struct ParseBitInfo
    {
        unsigned m_idx = 0U;
        unsigned m_sinceVersion = 0U;
        unsigned m_deprecatedSince = ParseProtocol::parseNotYetDeprecated();
        std::string m_description;
        std::string m_displayName;
        bool m_defaultValue = false;
        bool m_reserved = false;
        bool m_reservedValue = false;
    };

    using ParseBits = std::map<std::string, ParseBitInfo>;
    using ParseRevBits = std::multimap<unsigned, std::string>;

    explicit ParseSetField(const ParseSetFieldImpl* impl);
    explicit ParseSetField(ParseField field);

    ParseType parseType() const;
    ParseEndian parseEndian() const;
    bool parseDefaultBitValue() const;
    bool parseReservedBitValue() const;
    const ParseBits& parseBits() const;
    const ParseRevBits& parseRevBits() const;
    bool parseIsNonUniqueAllowed() const;
    bool parseIsUnique() const;
    bool parseValidCheckVersion() const;
    bool parseAvailableLengthLimit() const;
};

inline
bool operator==(const ParseSetField::ParseBitInfo& i1, const ParseSetField::ParseBitInfo& i2)
{
    return (i1.m_idx == i2.m_idx) &&
           (i1.m_sinceVersion == i2.m_sinceVersion) &&
           (i1.m_deprecatedSince == i2.m_deprecatedSince);
}

inline
bool operator!=(const ParseSetField::ParseBitInfo& i1, const ParseSetField::ParseBitInfo& i2)
{
    return !(i1 == i2);
}


} // namespace parse

} // namespace commsdsl
