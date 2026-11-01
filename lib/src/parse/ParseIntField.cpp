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

#include "commsdsl/parse/ParseIntField.h"

#include "ParseIntFieldImpl.h"

#include <cassert>

namespace commsdsl
{

namespace parse
{

namespace
{

[[nodiscard]] const ParseIntFieldImpl* parseCastInternal(const ParseFieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const ParseIntFieldImpl*>(ptr);
}

} // namespace

ParseIntField::ParseIntField(const ParseIntFieldImpl* impl)
  : Base(impl)
{
}

ParseIntField::ParseIntField(ParseField field)
  : Base(field)
{
    assert(parseKind() == ParseKind::Int);
}

ParseIntField::ParseType ParseIntField::parseType() const
{
    return parseCastInternal(m_pImpl)->parseType();
}

ParseEndian ParseIntField::parseEndian() const
{
    return parseCastInternal(m_pImpl)->parseEndian();
}

std::intmax_t ParseIntField::parseSerOffset() const
{
    return parseCastInternal(m_pImpl)->parseSerOffset();
}

std::intmax_t ParseIntField::parseMinValue() const
{
    return parseCastInternal(m_pImpl)->parseMinValue();
}

std::intmax_t ParseIntField::parseMaxValue() const
{
    return parseCastInternal(m_pImpl)->parseMaxValue();
}

std::intmax_t ParseIntField::parseDefaultValue() const
{
    return parseCastInternal(m_pImpl)->parseDefaultValue();
}

ParseIntField::ParseScalingRatio ParseIntField::parseScaling() const
{
    return parseCastInternal(m_pImpl)->parseScaling();
}

const ParseIntField::ParseValidRangesList& ParseIntField::parseValidRanges() const
{
    return parseCastInternal(m_pImpl)->parseValidRanges();
}

const ParseIntField::ParseSpecialValues& ParseIntField::parseSpecialValues() const
{
    return parseCastInternal(m_pImpl)->parseSpecialValues();
}

ParseUnits ParseIntField::parseUnits() const
{
    return parseCastInternal(m_pImpl)->parseUnits();
}

bool ParseIntField::parseValidCheckVersion() const
{
    return parseCastInternal(m_pImpl)->parseValidCheckVersion();
}

unsigned ParseIntField::parseDisplayDecimals() const
{
    return parseCastInternal(m_pImpl)->parseDisplayDecimals();
}

std::intmax_t ParseIntField::parseDisplayOffset() const
{
    return parseCastInternal(m_pImpl)->parseDisplayOffset();
}

bool ParseIntField::parseSignExt() const
{
    return parseCastInternal(m_pImpl)->parseSignExt();
}

bool ParseIntField::parseAvailableLengthLimit() const
{
    return parseCastInternal(m_pImpl)->parseAvailableLengthLimit();
}

} // namespace parse

} // namespace commsdsl
