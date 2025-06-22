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

#include <cstdint>
#include <cstddef>
#include <utility>
#include <map>
#include <string>

#include "commsdsl/parse/ParseEndian.h"
#include "commsdsl/parse/ParseField.h"
#include "commsdsl/parse/ParseProtocol.h"
#include "commsdsl/parse/ParseUnits.h"

namespace commsdsl
{

namespace parse
{

class ParseIntFieldImpl;
class COMMSDSL_API ParseIntField : public ParseField
{
    using Base = ParseField;
public:

    enum class Type
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
    using ScalingRatio = std::pair<std::intmax_t, std::intmax_t>;

    struct ValidRangeInfo
    {
        std::intmax_t m_min = 0;
        std::intmax_t m_max = 0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = ParseProtocol::notYetDeprecated();
    };

    using ValidRangesList = std::vector<ValidRangeInfo>;

    struct SpecialValueInfo
    {
        std::intmax_t m_value = 0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = ParseProtocol::notYetDeprecated();
        std::string m_description;
        std::string m_displayName;
    };

    using SpecialValues = std::map<std::string, SpecialValueInfo>;

    explicit ParseIntField(const ParseIntFieldImpl* impl);
    explicit ParseIntField(ParseField field);

    Type type() const;
    ParseEndian endian() const;
    std::intmax_t serOffset() const;
    std::intmax_t minValue() const;
    std::intmax_t maxValue() const;
    std::intmax_t defaultValue() const;
    ScalingRatio scaling() const;
    const ValidRangesList& validRanges() const;
    const SpecialValues& specialValues() const;
    ParseUnits units() const;
    bool validCheckVersion() const;
    unsigned displayDecimals() const;
    std::intmax_t displayOffset() const;
    bool signExt() const;
    bool availableLengthLimit() const;
};

inline
bool operator==(const ParseIntField::SpecialValueInfo& i1, const ParseIntField::SpecialValueInfo& i2)
{
    return (i1.m_value == i2.m_value) &&
           (i1.m_sinceVersion == i2.m_sinceVersion) &&
           (i1.m_deprecatedSince == i2.m_deprecatedSince);
}

inline
bool operator!=(const ParseIntField::SpecialValueInfo& i1, const ParseIntField::SpecialValueInfo& i2)
{
    return !(i1 == i2);
}

inline
bool operator==(const ParseIntField::ValidRangeInfo& i1, const ParseIntField::ValidRangeInfo& i2)
{
    return (i1.m_min == i2.m_min) &&
           (i1.m_max == i2.m_max) &&
           (i1.m_sinceVersion == i2.m_sinceVersion) &&
           (i1.m_deprecatedSince == i2.m_deprecatedSince);
}

inline
bool operator!=(const ParseIntField::ValidRangeInfo& i1, const ParseIntField::ValidRangeInfo& i2)
{
    return !(i1 == i2);
}

} // namespace parse

} // namespace commsdsl
