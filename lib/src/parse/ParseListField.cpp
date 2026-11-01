//
// Copyright 2018 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

#include "ParseListFieldImpl.h"

#include <cassert>

namespace commsdsl
{

namespace parse
{

namespace
{

[[nodiscard]] const ParseListFieldImpl* parseCastInternal(const ParseFieldImpl* ptr)
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
    assert(parseKind() == ParseKind::List);
}

ParseField ParseListField::parseElementField() const
{
    return parseCastInternal(m_pImpl)->parseElementField();
}

std::size_t ParseListField::parseFixedCount() const
{
    return parseCastInternal(m_pImpl)->parseCount();
}

bool ParseListField::parseHasCountPrefixField() const
{
    return parseCastInternal(m_pImpl)->parseHasCountPrefixField();
}

ParseField ParseListField::parseCountPrefixField() const
{
    return parseCastInternal(m_pImpl)->parseCountPrefixField();
}

const std::string& ParseListField::parseDetachedCountPrefixFieldName() const
{
    return parseCastInternal(m_pImpl)->parseDetachedCountPrefixFieldName();
}

bool ParseListField::parseHasLengthPrefixField() const
{
    return parseCastInternal(m_pImpl)->parseHasLengthPrefixField();
}

ParseField ParseListField::parseLengthPrefixField() const
{
    return parseCastInternal(m_pImpl)->parseLengthPrefixField();
}

const std::string& ParseListField::parseDetachedLengthPrefixFieldName() const
{
    return parseCastInternal(m_pImpl)->parseDetachedLengthPrefixFieldName();
}

bool ParseListField::parseHasElemLengthPrefixField() const
{
    return parseCastInternal(m_pImpl)->parseHasElemLengthPrefixField();
}

ParseField ParseListField::parseElemLengthPrefixField() const
{
    return parseCastInternal(m_pImpl)->parseElemLengthPrefixField();
}

const std::string& ParseListField::parseDetachedElemLengthPrefixFieldName() const
{
    return parseCastInternal(m_pImpl)->parseDetachedElemLengthPrefixFieldName();
}

bool ParseListField::parseElemFixedLength() const
{
    return parseCastInternal(m_pImpl)->parseElemFixedLength();
}

bool ParseListField::parseHasTermSuffixField() const
{
    return parseCastInternal(m_pImpl)->parseHasTermSuffixField();
}

ParseField ParseListField::parseTermSuffixField() const
{
    return parseCastInternal(m_pImpl)->parseTermSuffixField();
}

const std::string& ParseListField::parseDetachedTermSuffixFieldName() const
{
    return parseCastInternal(m_pImpl)->parseDetachedTermSuffixFieldName();
}

} // namespace parse

} // namespace commsdsl
