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

#include "commsdsl/parse/ParseDataField.h"

#include <cassert>

#include "ParseDataFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseDataFieldImpl* cast(const ParseFieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const ParseDataFieldImpl*>(ptr);
}

} // namespace

ParseDataField::ParseDataField(const ParseDataFieldImpl* impl)
  : Base(impl)
{
}

ParseDataField::ParseDataField(ParseField field)
  : Base(field)
{
    assert(kind() == Kind::Data);
}

const ParseDataField::ValueType& ParseDataField::defaultValue() const
{
    return cast(m_pImpl)->defaultValue();
}

std::size_t ParseDataField::fixedLength() const
{
    return cast(m_pImpl)->length();
}

bool ParseDataField::hasLengthPrefixField() const
{
    return cast(m_pImpl)->hasPrefixField();
}

ParseField ParseDataField::lengthPrefixField() const
{
    return cast(m_pImpl)->prefixField();
}

const std::string& ParseDataField::detachedPrefixFieldName() const
{
    return cast(m_pImpl)->detachedPrefixFieldName();
}

const ParseDataField::ValidValuesList& ParseDataField::validValues() const
{
    return cast(m_pImpl)->validValues();
}

} // namespace parse

} // namespace commsdsl
