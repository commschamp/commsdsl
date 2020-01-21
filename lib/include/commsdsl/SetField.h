//
// Copyright 2018 - 2020 (C). Alex Robenko. All rights reserved.
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

#include "IntField.h"

namespace commsdsl
{

class SetFieldImpl;
class COMMSDSL_API SetField : public Field
{
    using Base = Field;
public:

    using Type = IntField::Type;
    struct BitInfo
    {
        unsigned m_idx = 0U;
        unsigned m_sinceVersion = 0U;
        unsigned m_deprecatedSince = commsdsl::Protocol::notYetDeprecated();
        std::string m_description;
        std::string m_displayName;
        bool m_defaultValue = false;
        bool m_reserved = false;
        bool m_reservedValue = false;
    };

    using Bits = std::map<std::string, BitInfo>;
    using RevBits = std::multimap<unsigned, std::string>;

    explicit SetField(const SetFieldImpl* impl);
    explicit SetField(Field field);

    Type type() const;
    Endian endian() const;
    bool defaultBitValue() const;
    bool reservedBitValue() const;
    const Bits& bits() const;
    const RevBits& revBits() const;
    bool isNonUniqueAllowed() const;
    bool isUnique() const;
    bool validCheckVersion() const;
};

inline
bool operator==(const SetField::BitInfo& i1, const SetField::BitInfo& i2)
{
    return (i1.m_idx == i2.m_idx) &&
           (i1.m_sinceVersion == i2.m_sinceVersion) &&
           (i1.m_deprecatedSince == i2.m_deprecatedSince);
}

inline
bool operator!=(const SetField::BitInfo& i1, const SetField::BitInfo& i2)
{
    return !(i1 == i2);
}


} // namespace commsdsl
