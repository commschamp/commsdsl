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

#include "commsdsl/parse/ParseListField.h"

#include <cassert>

#include "ParseListFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseListFieldImpl* cast(const ParseFieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const ParseListFieldImpl*>(ptr);
}

} // namespace

ParseListField::ParseListField(const ParseListFieldImpl* impl)
  : Base(impl)
{
}

ParseListField::ParseListField(ParseField field)
  : Base(field)
{
    assert(parseKind() == Kind::List);
}

ParseField ParseListField::parseElementField() const
{
    return cast(m_pImpl)->parseElementField();
}

std::size_t ParseListField::parseFixedCount() const
{
    return cast(m_pImpl)->parseCount();
}

bool ParseListField::parseHasCountPrefixField() const
{
    return cast(m_pImpl)->parseHasCountPrefixField();
}

ParseField ParseListField::parseCountPrefixField() const
{
    return cast(m_pImpl)->parseCountPrefixField();
}

const std::string& ParseListField::parseDetachedCountPrefixFieldName() const
{
    return cast(m_pImpl)->parseDetachedCountPrefixFieldName();
}

bool ParseListField::parseHasLengthPrefixField() const
{
    return cast(m_pImpl)->parseHasLengthPrefixField();
}

ParseField ParseListField::parseLengthPrefixField() const
{
    return cast(m_pImpl)->parseLengthPrefixField();
}

const std::string& ParseListField::parseDetachedLengthPrefixFieldName() const
{
    return cast(m_pImpl)->parseDetachedLengthPrefixFieldName();
}

bool ParseListField::parseHasElemLengthPrefixField() const
{
    return cast(m_pImpl)->parseHasElemLengthPrefixField();
}

ParseField ParseListField::parseElemLengthPrefixField() const
{
    return cast(m_pImpl)->parseElemLengthPrefixField();
}

const std::string& ParseListField::parseDetachedElemLengthPrefixFieldName() const
{
    return cast(m_pImpl)->parseDetachedElemLengthPrefixFieldName();
}

bool ParseListField::parseElemFixedLength() const
{
    return cast(m_pImpl)->parseElemFixedLength();
}

bool ParseListField::parseHasTermSuffixField() const
{
    return cast(m_pImpl)->parseHasTermSuffixField();
}

ParseField ParseListField::parseTermSuffixField() const
{
    return cast(m_pImpl)->parseTermSuffixField();
}

const std::string& ParseListField::parseDetachedTermSuffixFieldName() const
{
    return cast(m_pImpl)->parseDetachedTermSuffixFieldName();
}

} // namespace parse

} // namespace commsdsl
