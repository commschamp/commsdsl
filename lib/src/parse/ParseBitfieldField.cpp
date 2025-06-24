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

#include "commsdsl/parse/ParseBitfieldField.h"

#include <cassert>

#include "ParseBitfieldFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseBitfieldFieldImpl* cast(const ParseFieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const ParseBitfieldFieldImpl*>(ptr);
}

} // namespace

ParseBitfieldField::ParseBitfieldField(const ParseBitfieldFieldImpl* impl) :
    Base(impl)
{
}

ParseBitfieldField::ParseBitfieldField(ParseField field)
  : Base(field)
{
    assert(parseKind() == Kind::Bitfield);
}

ParseEndian ParseBitfieldField::parseEndian() const
{
    return cast(m_pImpl)->parseEndian();
}

ParseBitfieldField::Members ParseBitfieldField::parseMembers() const
{
    return cast(m_pImpl)->parseMembersList();
}

ParseOptCond ParseBitfieldField::parseValidCond() const
{
    return cast(m_pImpl)->parseValidCond();
}

} // namespace parse

} // namespace commsdsl
