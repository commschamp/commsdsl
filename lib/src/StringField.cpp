//
// Copyright 2018 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/StringField.h"

#include <cassert>

#include "StringFieldImpl.h"

namespace commsdsl
{

namespace
{

const StringFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const StringFieldImpl*>(ptr);
}

} // namespace

StringField::StringField(const StringFieldImpl* impl)
  : Base(impl)
{
}

StringField::StringField(Field field)
  : Base(field)
{
    assert(kind() == Kind::String);
}

const std::string& StringField::defaultValue() const
{
    return cast(m_pImpl)->defaultValue();
}

const std::string& StringField::encodingStr() const
{
    return cast(m_pImpl)->encodingStr();
}

std::size_t StringField::fixedLength() const
{
    return cast(m_pImpl)->length();
}

bool StringField::hasLengthPrefixField() const
{
    return cast(m_pImpl)->hasPrefixField();
}

Field StringField::lengthPrefixField() const
{
    return cast(m_pImpl)->prefixField();
}

bool StringField::hasZeroTermSuffix() const
{
    return cast(m_pImpl)->hasZeroTermSuffix();
}

const std::string& StringField::detachedPrefixFieldName() const
{
    return cast(m_pImpl)->detachedPrefixFieldName();
}

} // namespace commsdsl
