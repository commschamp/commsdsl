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

#include "ParseStringFieldImpl.h"

#include <cassert>

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
    assert(parseKind() == ParseKind::String);
}

const std::string& ParseStringField::parseDefaultValue() const
{
    return cast(m_pImpl)->parseDefaultValue();
}

const std::string& ParseStringField::parseEncodingStr() const
{
    return cast(m_pImpl)->parseEncodingStr();
}

std::size_t ParseStringField::parseFixedLength() const
{
    return cast(m_pImpl)->parseLength();
}

bool ParseStringField::parseHasLengthPrefixField() const
{
    return cast(m_pImpl)->parseHasPrefixField();
}

ParseField ParseStringField::parseLengthPrefixField() const
{
    return cast(m_pImpl)->parsePrefixField();
}

bool ParseStringField::parseHasZeroTermSuffix() const
{
    return cast(m_pImpl)->parseHasZeroTermSuffix();
}

const std::string& ParseStringField::parseDetachedPrefixFieldName() const
{
    return cast(m_pImpl)->parseDetachedPrefixFieldName();
}

const ParseStringField::ParseValidValuesList& ParseStringField::parseValidValues() const
{
    return cast(m_pImpl)->parseValidValues();
}

} // namespace parse

} // namespace commsdsl
