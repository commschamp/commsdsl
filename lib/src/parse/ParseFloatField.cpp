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

#include "commsdsl/parse/ParseFloatField.h"

#include "ParseFloatFieldImpl.h"

#include <cassert>

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseFloatFieldImpl* cast(const ParseFieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const ParseFloatFieldImpl*>(ptr);
}

} // namespace

ParseFloatField::ParseFloatField(const ParseFloatFieldImpl* impl)
  : Base(impl)
{
}

ParseFloatField::ParseFloatField(ParseField field)
  : Base(field)
{
    assert(parseKind() == ParseKind::Float);
}

ParseFloatField::ParseType ParseFloatField::parseType() const
{
    return cast(m_pImpl)->parseType();
}

ParseEndian ParseFloatField::parseEndian() const
{
    return cast(m_pImpl)->parseEndian();
}

double ParseFloatField::parseDefaultValue() const
{
    return cast(m_pImpl)->parseDefaultValue();
}

const ParseFloatField::ParseValidRangesList& ParseFloatField::parseValidRanges() const
{
    return cast(m_pImpl)->parseValidRanges();
}

const ParseFloatField::ParseSpecialValues& ParseFloatField::parseSpecialValues() const
{
    return cast(m_pImpl)->parseSpecialValues();
}

bool ParseFloatField::parseValidCheckVersion() const
{
    return cast(m_pImpl)->parseValidCheckVersion();
}

ParseUnits ParseFloatField::parseUnits() const
{
    return cast(m_pImpl)->parseUnits();
}

unsigned ParseFloatField::parseDisplayDecimals() const
{
    return cast(m_pImpl)->parseDisplayDecimals();
}

bool ParseFloatField::parseHasNonUniqueSpecials() const
{
    return cast(m_pImpl)->parseHasNonUniqueSpecials();
}

} // namespace parse

} // namespace commsdsl
