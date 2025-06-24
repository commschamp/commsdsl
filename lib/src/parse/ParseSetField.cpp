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
    assert(parseKind() == Kind::Set);
}

ParseSetField::Type ParseSetField::parseType() const
{
    return cast(m_pImpl)->parseType();
}

ParseEndian ParseSetField::parseEndian() const
{
    return cast(m_pImpl)->parseEndian();
}

bool ParseSetField::parseDefaultBitValue() const
{
    return cast(m_pImpl)->parseDefaultBitValue();
}

bool ParseSetField::parseReservedBitValue() const
{
    return cast(m_pImpl)->parseReservedBitValue();
}

const ParseSetField::Bits& ParseSetField::parseBits() const
{
    return cast(m_pImpl)->parseBits();
}

const ParseSetField::RevBits& ParseSetField::parseRevBits() const
{
    return cast(m_pImpl)->parseRevBits();
}

bool ParseSetField::parseIsNonUniqueAllowed() const
{
    return cast(m_pImpl)->parseIsNonUniqueAllowed();
}

bool ParseSetField::parseIsUnique() const
{
    return cast(m_pImpl)->parseIsUnique();
}

bool ParseSetField::parseValidCheckVersion() const
{
    return cast(m_pImpl)->parseValidCheckVersion();
}

bool ParseSetField::parseAvailableLengthLimit() const
{
    return cast(m_pImpl)->parseAvailableLengthLimit();
}

} // namespace parse

} // namespace commsdsl
