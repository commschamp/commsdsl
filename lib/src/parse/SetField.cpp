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

#include "commsdsl/parse/SetField.h"

#include <cassert>

#include "SetFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const SetFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const SetFieldImpl*>(ptr);
}

} // namespace

SetField::SetField(const SetFieldImpl* impl)
  : Base(impl)
{
}

SetField::SetField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Set);
}

SetField::Type SetField::type() const
{
    return cast(m_pImpl)->type();
}

Endian SetField::endian() const
{
    return cast(m_pImpl)->endian();
}

bool SetField::defaultBitValue() const
{
    return cast(m_pImpl)->defaultBitValue();
}

bool SetField::reservedBitValue() const
{
    return cast(m_pImpl)->reservedBitValue();
}

const SetField::Bits& SetField::bits() const
{
    return cast(m_pImpl)->bits();
}

const SetField::RevBits& SetField::revBits() const
{
    return cast(m_pImpl)->revBits();
}

bool SetField::isNonUniqueAllowed() const
{
    return cast(m_pImpl)->isNonUniqueAllowed();
}

bool SetField::isUnique() const
{
    return cast(m_pImpl)->isUnique();
}

bool SetField::validCheckVersion() const
{
    return cast(m_pImpl)->validCheckVersion();
}

} // namespace parse

} // namespace commsdsl
