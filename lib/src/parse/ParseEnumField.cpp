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

#include <cassert>

#include "ParseEnumFieldImpl.h"

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
    assert(kind() == Kind::Enum);
}

ParseEnumField::Type ParseEnumField::type() const
{
    return cast(m_pImpl)->type();
}

ParseEndian ParseEnumField::endian() const
{
    return cast(m_pImpl)->endian();
}

std::intmax_t ParseEnumField::defaultValue() const
{
    return cast(m_pImpl)->defaultValue();
}

const ParseEnumField::Values& ParseEnumField::values() const
{
    return cast(m_pImpl)->values();
}

const ParseEnumField::RevValues& ParseEnumField::revValues() const
{
    return cast(m_pImpl)->revValues();
}

bool ParseEnumField::isNonUniqueAllowed() const
{
    return cast(m_pImpl)->isNonUniqueAllowed();
}

bool ParseEnumField::isUnique() const
{
    return cast(m_pImpl)->isUnique();
}

bool ParseEnumField::validCheckVersion() const
{
    return cast(m_pImpl)->validCheckVersion();
}

bool ParseEnumField::hexAssign() const
{
    return cast(m_pImpl)->hexAssign();
}

bool ParseEnumField::availableLengthLimit() const
{
    return cast(m_pImpl)->availableLengthLimit();
}

} // namespace parse

} // namespace commsdsl
