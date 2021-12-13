//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/EnumField.h"

#include <cassert>

#include "EnumFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const EnumFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const EnumFieldImpl*>(ptr);
}

} // namespace

EnumField::EnumField(const EnumFieldImpl* impl)
  : Base(impl)
{
}

EnumField::EnumField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Enum);
}

EnumField::Type EnumField::type() const
{
    return cast(m_pImpl)->type();
}

Endian EnumField::endian() const
{
    return cast(m_pImpl)->endian();
}

std::intmax_t EnumField::defaultValue() const
{
    return cast(m_pImpl)->defaultValue();
}

const EnumField::Values& EnumField::values() const
{
    return cast(m_pImpl)->values();
}

const EnumField::RevValues& EnumField::revValues() const
{
    return cast(m_pImpl)->revValues();
}

bool EnumField::isNonUniqueAllowed() const
{
    return cast(m_pImpl)->isNonUniqueAllowed();
}

bool EnumField::isUnique() const
{
    return cast(m_pImpl)->isUnique();
}

bool EnumField::validCheckVersion() const
{
    return cast(m_pImpl)->validCheckVersion();
}

bool EnumField::hexAssign() const
{
    return cast(m_pImpl)->hexAssign();
}

} // namespace parse

} // namespace commsdsl
