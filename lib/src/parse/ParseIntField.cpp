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

#include "commsdsl/parse/ParseIntField.h"

#include <cassert>

#include "ParseIntFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseIntFieldImpl* cast(const ParseFieldImpl* ptr)
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
    assert(kind() == Kind::Int);
}

ParseIntField::Type ParseIntField::type() const
{
    return cast(m_pImpl)->type();
}

ParseEndian ParseIntField::endian() const
{
    return cast(m_pImpl)->endian();
}

std::intmax_t ParseIntField::serOffset() const
{
    return cast(m_pImpl)->serOffset();
}

std::intmax_t ParseIntField::minValue() const
{
    return cast(m_pImpl)->minValue();
}

std::intmax_t ParseIntField::maxValue() const
{
    return cast(m_pImpl)->maxValue();
}

std::intmax_t ParseIntField::defaultValue() const
{
    return cast(m_pImpl)->defaultValue();
}

ParseIntField::ScalingRatio ParseIntField::scaling() const
{
    return cast(m_pImpl)->scaling();
}

const ParseIntField::ValidRangesList& ParseIntField::validRanges() const
{
    return cast(m_pImpl)->validRanges();
}

const ParseIntField::SpecialValues& ParseIntField::specialValues() const
{
    return cast(m_pImpl)->specialValues();
}

ParseUnits ParseIntField::units() const
{
    return cast(m_pImpl)->units();
}

bool ParseIntField::validCheckVersion() const
{
    return cast(m_pImpl)->validCheckVersion();
}

unsigned ParseIntField::displayDecimals() const
{
    return cast(m_pImpl)->displayDecimals();
}

std::intmax_t ParseIntField::displayOffset() const
{
    return cast(m_pImpl)->displayOffset();
}

bool ParseIntField::signExt() const
{
    return cast(m_pImpl)->signExt();
}

bool ParseIntField::availableLengthLimit() const
{
    return cast(m_pImpl)->availableLengthLimit();
}

} // namespace parse

} // namespace commsdsl
