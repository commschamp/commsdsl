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

#include <map>
#include <string>
#include <utility>

namespace commsdsl
{

namespace parse
{

class ParseFloatFieldImpl;
class COMMSDSL_API ParseFloatField : public ParseField
{
    using Base = ParseField;
public:

    enum class Type
    {
        Float,
        Double,
        NumOfValues
    };

    struct ValidRangeInfo
    {
        double m_min = 0.0;
        double m_max = 0.0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = ParseProtocol::parseNotYetDeprecated();
    };
    using ValidRangesList = std::vector<ValidRangeInfo>;

    struct SpecialValueInfo
    {
        double m_value = 0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = ParseProtocol::parseNotYetDeprecated();
        std::string m_description;
        std::string m_displayName;
    };
    using SpecialValues = std::map<std::string, SpecialValueInfo>;

    explicit ParseFloatField(const ParseFloatFieldImpl* impl);
    explicit ParseFloatField(ParseField field);

    Type parseType() const;
    ParseEndian parseEndian() const;
    double parseDefaultValue() const;
    const ValidRangesList& parseValidRanges() const;
    const SpecialValues& parseSpecialValues() const;
    bool parseValidCheckVersion() const;
    ParseUnits parseUnits() const;
    unsigned parseDisplayDecimals() const;
    bool parseHasNonUniqueSpecials() const;
};

inline
bool operator==(const ParseFloatField::SpecialValueInfo& i1, const ParseFloatField::SpecialValueInfo& i2)
{
    return (i1.m_value == i2.m_value) &&
           (i1.m_sinceVersion == i2.m_sinceVersion) &&
           (i1.m_deprecatedSince == i2.m_deprecatedSince);
}

inline
bool operator!=(const ParseFloatField::SpecialValueInfo& i1, const ParseFloatField::SpecialValueInfo& i2)
{
    return !(i1 == i2);
}

inline
bool operator==(const ParseFloatField::ValidRangeInfo& i1, const ParseFloatField::ValidRangeInfo& i2)
{
    return (i1.m_min == i2.m_min) &&
           (i1.m_max == i2.m_max) &&
           (i1.m_sinceVersion == i2.m_sinceVersion) &&
           (i1.m_deprecatedSince == i2.m_deprecatedSince);
}

inline
bool operator!=(const ParseFloatField::ValidRangeInfo& i1, const ParseFloatField::ValidRangeInfo& i2)
{
    return !(i1 == i2);
}

} // namespace parse

} // namespace commsdsl
