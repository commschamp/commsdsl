//
// Copyright 2018 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/ListField.h"

#include <cassert>

#include "ListFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const ListFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const ListFieldImpl*>(ptr);
}

} // namespace

ListField::ListField(const ListFieldImpl* impl)
  : Base(impl)
{
}

ListField::ListField(Field field)
  : Base(field)
{
    assert(kind() == Kind::List);
}

Field ListField::elementField() const
{
    return cast(m_pImpl)->elementField();
}

std::size_t ListField::fixedCount() const
{
    return cast(m_pImpl)->count();
}

bool ListField::hasCountPrefixField() const
{
    return cast(m_pImpl)->hasCountPrefixField();
}

Field ListField::countPrefixField() const
{
    return cast(m_pImpl)->countPrefixField();
}

const std::string& ListField::detachedCountPrefixFieldName() const
{
    return cast(m_pImpl)->detachedCountPrefixFieldName();
}

bool ListField::hasLengthPrefixField() const
{
    return cast(m_pImpl)->hasLengthPrefixField();
}

Field ListField::lengthPrefixField() const
{
    return cast(m_pImpl)->lengthPrefixField();
}

const std::string& ListField::detachedLengthPrefixFieldName() const
{
    return cast(m_pImpl)->detachedLengthPrefixFieldName();
}

bool ListField::hasElemLengthPrefixField() const
{
    return cast(m_pImpl)->hasElemLengthPrefixField();
}

Field ListField::elemLengthPrefixField() const
{
    return cast(m_pImpl)->elemLengthPrefixField();
}

const std::string& ListField::detachedElemLengthPrefixFieldName() const
{
    return cast(m_pImpl)->detachedElemLengthPrefixFieldName();
}

bool ListField::elemFixedLength() const
{
    return cast(m_pImpl)->elemFixedLength();
}

bool ListField::hasTermSuffixField() const
{
    return cast(m_pImpl)->hasTermSuffixField();
}

Field ListField::termSuffixField() const
{
    return cast(m_pImpl)->termSuffixField();
}

const std::string& ListField::detachedTermSuffixFieldName() const
{
    return cast(m_pImpl)->detachedTermSuffixFieldName();
}

} // namespace parse

} // namespace commsdsl
