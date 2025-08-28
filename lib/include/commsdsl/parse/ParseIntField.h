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
#include "commsdsl/parse/ParseField.h"
#include "commsdsl/parse/ParseProtocol.h"
#include "commsdsl/parse/ParseUnits.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <utility>

namespace commsdsl
{

namespace parse
{

class ParseIntFieldImpl;
class COMMSDSL_API ParseIntField : public ParseField
{
    using Base = ParseField;
public:

    enum class ParseType
    {
        Int8,
        Uint8,
        Int16,
        Uint16,
        Int32,
        Uint32,
        Int64,
        Uint64,
        Intvar,
        Uintvar,
        NumOfValues
    };
    using ParseScalingRatio = std::pair<std::intmax_t, std::intmax_t>;

    struct ParseValidRangeInfo
    {
        std::intmax_t m_min = 0;
        std::intmax_t m_max = 0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = ParseProtocol::parseNotYetDeprecated();
    };

    using ParseValidRangesList = std::vector<ParseValidRangeInfo>;

    struct ParseSpecialValueInfo
    {
        std::intmax_t m_value = 0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = ParseProtocol::parseNotYetDeprecated();
        std::string m_description;
        std::string m_displayName;
    };

    using ParseSpecialValues = std::map<std::string, ParseSpecialValueInfo>;

    explicit ParseIntField(const ParseIntFieldImpl* impl);
    explicit ParseIntField(ParseField field);

    ParseType parseType() const;
    ParseEndian parseEndian() const;
    std::intmax_t parseSerOffset() const;
    std::intmax_t parseMinValue() const;
    std::intmax_t parseMaxValue() const;
    std::intmax_t parseDefaultValue() const;
    ParseScalingRatio parseScaling() const;
    const ParseValidRangesList& parseValidRanges() const;
    const ParseSpecialValues& parseSpecialValues() const;
    ParseUnits parseUnits() const;
    bool parseValidCheckVersion() const;
    unsigned parseDisplayDecimals() const;
    std::intmax_t parseDisplayOffset() const;
    bool parseSignExt() const;
    bool parseAvailableLengthLimit() const;
};

inline
bool operator==(const ParseIntField::ParseSpecialValueInfo& i1, const ParseIntField::ParseSpecialValueInfo& i2)
{
    return (i1.m_value == i2.m_value) &&
           (i1.m_sinceVersion == i2.m_sinceVersion) &&
           (i1.m_deprecatedSince == i2.m_deprecatedSince);
}

inline
bool operator!=(const ParseIntField::ParseSpecialValueInfo& i1, const ParseIntField::ParseSpecialValueInfo& i2)
{
    return !(i1 == i2);
}

inline
bool operator==(const ParseIntField::ParseValidRangeInfo& i1, const ParseIntField::ParseValidRangeInfo& i2)
{
    return (i1.m_min == i2.m_min) &&
           (i1.m_max == i2.m_max) &&
           (i1.m_sinceVersion == i2.m_sinceVersion) &&
           (i1.m_deprecatedSince == i2.m_deprecatedSince);
}

inline
bool operator!=(const ParseIntField::ParseValidRangeInfo& i1, const ParseIntField::ParseValidRangeInfo& i2)
{
    return !(i1 == i2);
}

} // namespace parse

} // namespace commsdsl
