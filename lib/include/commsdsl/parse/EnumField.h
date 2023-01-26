//
// Copyright 2018 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/Endian.h"
#include "commsdsl/parse/IntField.h"
#include "commsdsl/parse/Protocol.h"

namespace commsdsl
{

namespace parse
{

class EnumFieldImpl;
class COMMSDSL_API EnumField : public Field
{
    using Base = Field;
public:

    struct ValueInfo
    {
        std::intmax_t m_value = 0;
        unsigned m_sinceVersion = 0;
        unsigned m_deprecatedSince = Protocol::notYetDeprecated();
        std::string m_description;
        std::string m_displayName;
    };

    using Type = IntField::Type;
    using Values = std::map<std::string, ValueInfo>;
    using RevValues = std::multimap<std::intmax_t, std::string>;

    explicit EnumField(const EnumFieldImpl* impl);
    explicit EnumField(Field field);

    Type type() const;
    Endian endian() const;
    std::intmax_t defaultValue() const;
    const Values& values() const;
    const RevValues& revValues() const;
    bool isNonUniqueAllowed() const;
    bool isUnique() const;
    bool validCheckVersion() const;
    bool hexAssign() const;
    bool availableLengthLimit() const;
};

inline
bool operator==(const EnumField::ValueInfo& i1, const EnumField::ValueInfo& i2)
{
    return (i1.m_value == i2.m_value) &&
           (i1.m_sinceVersion == i2.m_sinceVersion) &&
           (i1.m_deprecatedSince == i2.m_deprecatedSince);
}

} // namespace parse

} // namespace commsdsl
