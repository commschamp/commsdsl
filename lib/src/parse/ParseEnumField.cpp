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

#include "commsdsl/parse/ParseEnumField.h"

#include "ParseEnumFieldImpl.h"

#include <cassert>

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseEnumFieldImpl* cast(const ParseFieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const ParseEnumFieldImpl*>(ptr);
}

} // namespace

ParseEnumField::ParseEnumField(const ParseEnumFieldImpl* impl)
  : Base(impl)
{
}

ParseEnumField::ParseEnumField(ParseField field)
  : Base(field)
{
    assert(parseKind() == ParseKind::Enum);
}

ParseEnumField::ParseType ParseEnumField::parseType() const
{
    return cast(m_pImpl)->parseType();
}

ParseEndian ParseEnumField::parseEndian() const
{
    return cast(m_pImpl)->parseEndian();
}

std::intmax_t ParseEnumField::parseDefaultValue() const
{
    return cast(m_pImpl)->parseDefaultValue();
}

const ParseEnumField::ParseValues& ParseEnumField::parseValues() const
{
    return cast(m_pImpl)->parseValues();
}

const ParseEnumField::ParseRevValues& ParseEnumField::parseRevValues() const
{
    return cast(m_pImpl)->parseRevValues();
}

bool ParseEnumField::parseIsNonUniqueAllowed() const
{
    return cast(m_pImpl)->parseIsNonUniqueAllowed();
}

bool ParseEnumField::parseIsUnique() const
{
    return cast(m_pImpl)->parseIsUnique();
}

bool ParseEnumField::parseValidCheckVersion() const
{
    return cast(m_pImpl)->parseValidCheckVersion();
}

bool ParseEnumField::parseHexAssign() const
{
    return cast(m_pImpl)->parseHexAssign();
}

bool ParseEnumField::parseAvailableLengthLimit() const
{
    return cast(m_pImpl)->parseAvailableLengthLimit();
}

} // namespace parse

} // namespace commsdsl
