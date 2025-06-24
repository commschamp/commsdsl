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

#include "commsdsl/parse/ParseOptionalField.h"

#include <cassert>

#include "ParseOptionalFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseOptionalFieldImpl* cast(const ParseFieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const ParseOptionalFieldImpl*>(ptr);
}

} // namespace

ParseOptionalField::ParseOptionalField(const ParseOptionalFieldImpl* impl)
  : Base(impl)
{
}

ParseOptionalField::ParseOptionalField(ParseField field)
  : Base(field)
{
    assert(parseKind() == Kind::Optional);
}

ParseOptionalField::Mode ParseOptionalField::parseDefaultMode() const
{
    return cast(m_pImpl)->parseDefaultMode();
}

ParseField ParseOptionalField::parseField() const
{
    return cast(m_pImpl)->parseField();
}

ParseOptCond ParseOptionalField::parseCond() const
{
    return cast(m_pImpl)->parseWrappedCondition();
}

bool ParseOptionalField::parseMissingOnReadFail() const
{
    return cast(m_pImpl)->parseMissingOnReadFail();
}

bool ParseOptionalField::parseMissingOnInvalid() const
{
    return cast(m_pImpl)->parseMissingOnInvalid();
}

} // namespace parse

} // namespace commsdsl
