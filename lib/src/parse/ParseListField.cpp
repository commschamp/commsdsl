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
    assert(kind() == Kind::List);
}

ParseField ParseListField::elementField() const
{
    return cast(m_pImpl)->elementField();
}

std::size_t ParseListField::fixedCount() const
{
    return cast(m_pImpl)->count();
}

bool ParseListField::hasCountPrefixField() const
{
    return cast(m_pImpl)->hasCountPrefixField();
}

ParseField ParseListField::countPrefixField() const
{
    return cast(m_pImpl)->countPrefixField();
}

const std::string& ParseListField::detachedCountPrefixFieldName() const
{
    return cast(m_pImpl)->detachedCountPrefixFieldName();
}

bool ParseListField::hasLengthPrefixField() const
{
    return cast(m_pImpl)->hasLengthPrefixField();
}

ParseField ParseListField::lengthPrefixField() const
{
    return cast(m_pImpl)->lengthPrefixField();
}

const std::string& ParseListField::detachedLengthPrefixFieldName() const
{
    return cast(m_pImpl)->detachedLengthPrefixFieldName();
}

bool ParseListField::hasElemLengthPrefixField() const
{
    return cast(m_pImpl)->hasElemLengthPrefixField();
}

ParseField ParseListField::elemLengthPrefixField() const
{
    return cast(m_pImpl)->elemLengthPrefixField();
}

const std::string& ParseListField::detachedElemLengthPrefixFieldName() const
{
    return cast(m_pImpl)->detachedElemLengthPrefixFieldName();
}

bool ParseListField::elemFixedLength() const
{
    return cast(m_pImpl)->elemFixedLength();
}

bool ParseListField::hasTermSuffixField() const
{
    return cast(m_pImpl)->hasTermSuffixField();
}

ParseField ParseListField::termSuffixField() const
{
    return cast(m_pImpl)->termSuffixField();
}

const std::string& ParseListField::detachedTermSuffixFieldName() const
{
    return cast(m_pImpl)->detachedTermSuffixFieldName();
}

} // namespace parse

} // namespace commsdsl
