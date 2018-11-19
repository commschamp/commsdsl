//
// Copyright 2018 (C). Alex Robenko. All rights reserved.
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

#include "Field.h"
#include "Endian.h"
#include "Protocol.h"
#include "Units.h"

namespace commsdsl
{

class IntFieldImpl;
class COMMSDSL_API IntField : public Field
{
    using Base = Field;
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
        unsigned m_deprecatedSince = Protocol::notYetDeprecated();
    };

    using ValidRangesList = std::vector<ValidRangeInfo>;

    struct SpecialValueInfo
    {
        std::intmax_t m_value = 0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = Protocol::notYetDeprecated();
        std::string m_description;
    };

    using SpecialValues = std::map<std::string, SpecialValueInfo>;

    explicit IntField(const IntFieldImpl* impl);
    explicit IntField(Field field);

    Type type() const;
    Endian endian() const;
    std::intmax_t serOffset() const;
    std::intmax_t minValue() const;
    std::intmax_t maxValue() const;
    std::intmax_t defaultValue() const;
    ScalingRatio scaling() const;
    const ValidRangesList& validRanges() const;
    const SpecialValues& specialValues() const;
    Units units() const;
    bool validCheckVersion() const;
    unsigned displayDecimals() const;
    std::intmax_t displayOffset() const;
    bool signExt() const;
};

inline
bool operator==(const IntField::SpecialValueInfo& i1, const IntField::SpecialValueInfo& i2)
{
    return (i1.m_value == i2.m_value) &&
           (i1.m_sinceVersion == i2.m_sinceVersion) &&
           (i1.m_deprecatedSince == i2.m_deprecatedSince);
}

inline
bool operator!=(const IntField::SpecialValueInfo& i1, const IntField::SpecialValueInfo& i2)
{
    return !(i1 == i2);
}

inline
bool operator==(const IntField::ValidRangeInfo& i1, const IntField::ValidRangeInfo& i2)
{
    return (i1.m_min == i2.m_min) &&
           (i1.m_max == i2.m_max) &&
           (i1.m_sinceVersion == i2.m_sinceVersion) &&
           (i1.m_deprecatedSince == i2.m_deprecatedSince);
}

inline
bool operator!=(const IntField::ValidRangeInfo& i1, const IntField::ValidRangeInfo& i2)
{
    return !(i1 == i2);
}

} // namespace commsdsl
