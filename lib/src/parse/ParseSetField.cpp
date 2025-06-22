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

#include "commsdsl/parse/ParseSetField.h"

#include <cassert>

#include "ParseSetFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseSetFieldImpl* cast(const ParseFieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const ParseSetFieldImpl*>(ptr);
}

} // namespace

ParseSetField::ParseSetField(const ParseSetFieldImpl* impl)
  : Base(impl)
{
}

ParseSetField::ParseSetField(ParseField field)
  : Base(field)
{
    assert(kind() == Kind::Set);
}

ParseSetField::Type ParseSetField::type() const
{
    return cast(m_pImpl)->type();
}

ParseEndian ParseSetField::endian() const
{
    return cast(m_pImpl)->endian();
}

bool ParseSetField::defaultBitValue() const
{
    return cast(m_pImpl)->defaultBitValue();
}

bool ParseSetField::reservedBitValue() const
{
    return cast(m_pImpl)->reservedBitValue();
}

const ParseSetField::Bits& ParseSetField::bits() const
{
    return cast(m_pImpl)->bits();
}

const ParseSetField::RevBits& ParseSetField::revBits() const
{
    return cast(m_pImpl)->revBits();
}

bool ParseSetField::isNonUniqueAllowed() const
{
    return cast(m_pImpl)->isNonUniqueAllowed();
}

bool ParseSetField::isUnique() const
{
    return cast(m_pImpl)->isUnique();
}

bool ParseSetField::validCheckVersion() const
{
    return cast(m_pImpl)->validCheckVersion();
}

bool ParseSetField::availableLengthLimit() const
{
    return cast(m_pImpl)->availableLengthLimit();
}

} // namespace parse

} // namespace commsdsl
