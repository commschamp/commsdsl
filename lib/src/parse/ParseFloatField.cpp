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

#include <cassert>

#include "ParseFloatFieldImpl.h"

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
    assert(kind() == Kind::Float);
}

ParseFloatField::Type ParseFloatField::type() const
{
    return cast(m_pImpl)->type();
}

ParseEndian ParseFloatField::endian() const
{
    return cast(m_pImpl)->endian();
}

double ParseFloatField::defaultValue() const
{
    return cast(m_pImpl)->defaultValue();
}

const ParseFloatField::ValidRangesList& ParseFloatField::validRanges() const
{
    return cast(m_pImpl)->validRanges();
}

const ParseFloatField::SpecialValues& ParseFloatField::specialValues() const
{
    return cast(m_pImpl)->specialValues();
}

bool ParseFloatField::validCheckVersion() const
{
    return cast(m_pImpl)->validCheckVersion();
}

ParseUnits ParseFloatField::units() const
{
    return cast(m_pImpl)->units();
}

unsigned ParseFloatField::displayDecimals() const
{
    return cast(m_pImpl)->displayDecimals();
}

bool ParseFloatField::hasNonUniqueSpecials() const
{
    return cast(m_pImpl)->hasNonUniqueSpecials();
}

} // namespace parse

} // namespace commsdsl
