//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/IntField.h"

#include <cassert>

#include "IntFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const IntFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const IntFieldImpl*>(ptr);
}

} // namespace

IntField::IntField(const IntFieldImpl* impl)
  : Base(impl)
{
}

IntField::IntField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Int);
}

IntField::Type IntField::type() const
{
    return cast(m_pImpl)->type();
}

Endian IntField::endian() const
{
    return cast(m_pImpl)->endian();
}

std::intmax_t IntField::serOffset() const
{
    return cast(m_pImpl)->serOffset();
}

std::intmax_t IntField::minValue() const
{
    return cast(m_pImpl)->minValue();
}

std::intmax_t IntField::maxValue() const
{
    return cast(m_pImpl)->maxValue();
}

std::intmax_t IntField::defaultValue() const
{
    return cast(m_pImpl)->defaultValue();
}

IntField::ScalingRatio IntField::scaling() const
{
    return cast(m_pImpl)->scaling();
}

const IntField::ValidRangesList& IntField::validRanges() const
{
    return cast(m_pImpl)->validRanges();
}

const IntField::SpecialValues& IntField::specialValues() const
{
    return cast(m_pImpl)->specialValues();
}

Units IntField::units() const
{
    return cast(m_pImpl)->units();
}

bool IntField::validCheckVersion() const
{
    return cast(m_pImpl)->validCheckVersion();
}

unsigned IntField::displayDecimals() const
{
    return cast(m_pImpl)->displayDecimals();
}

std::intmax_t IntField::displayOffset() const
{
    return cast(m_pImpl)->displayOffset();
}

bool IntField::signExt() const
{
    return cast(m_pImpl)->signExt();
}

bool IntField::displaySpecials() const
{
    return cast(m_pImpl)->displaySpecials();
}

bool IntField::availableLengthLimit() const
{
    return cast(m_pImpl)->availableLengthLimit();
}

} // namespace parse

} // namespace commsdsl
