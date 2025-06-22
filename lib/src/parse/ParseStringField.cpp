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

#include "commsdsl/parse/ParseStringField.h"

#include <cassert>

#include "ParseStringFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseStringFieldImpl* cast(const ParseFieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const ParseStringFieldImpl*>(ptr);
}

} // namespace

ParseStringField::ParseStringField(const ParseStringFieldImpl* impl)
  : Base(impl)
{
}

ParseStringField::ParseStringField(ParseField field)
  : Base(field)
{
    assert(kind() == Kind::String);
}

const std::string& ParseStringField::defaultValue() const
{
    return cast(m_pImpl)->defaultValue();
}

const std::string& ParseStringField::encodingStr() const
{
    return cast(m_pImpl)->encodingStr();
}

std::size_t ParseStringField::fixedLength() const
{
    return cast(m_pImpl)->length();
}

bool ParseStringField::hasLengthPrefixField() const
{
    return cast(m_pImpl)->hasPrefixField();
}

ParseField ParseStringField::lengthPrefixField() const
{
    return cast(m_pImpl)->prefixField();
}

bool ParseStringField::hasZeroTermSuffix() const
{
    return cast(m_pImpl)->hasZeroTermSuffix();
}

const std::string& ParseStringField::detachedPrefixFieldName() const
{
    return cast(m_pImpl)->detachedPrefixFieldName();
}

const ParseStringField::ValidValuesList& ParseStringField::validValues() const
{
    return cast(m_pImpl)->validValues();
}

} // namespace parse

} // namespace commsdsl
