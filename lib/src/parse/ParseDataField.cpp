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

#include "ParseDataFieldImpl.h"

#include <cassert>

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
    assert(parseKind() == Kind::Data);
}

const ParseDataField::ValueType& ParseDataField::parseDefaultValue() const
{
    return cast(m_pImpl)->parseDefaultValue();
}

std::size_t ParseDataField::parseFixedLength() const
{
    return cast(m_pImpl)->parseLength();
}

bool ParseDataField::parseHasLengthPrefixField() const
{
    return cast(m_pImpl)->parseHasPrefixField();
}

ParseField ParseDataField::parseLengthPrefixField() const
{
    return cast(m_pImpl)->parsePrefixField();
}

const std::string& ParseDataField::parseDetachedPrefixFieldName() const
{
    return cast(m_pImpl)->parseDetachedPrefixFieldName();
}

const ParseDataField::ValidValuesList& ParseDataField::parseValidValues() const
{
    return cast(m_pImpl)->parseValidValues();
}

} // namespace parse

} // namespace commsdsl
