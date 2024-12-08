//
// Copyright 2018 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/FloatField.h"

#include <cassert>

#include "FloatFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const FloatFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const FloatFieldImpl*>(ptr);
}

} // namespace

FloatField::FloatField(const FloatFieldImpl* impl)
  : Base(impl)
{
}

FloatField::FloatField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Float);
}

FloatField::Type FloatField::type() const
{
    return cast(m_pImpl)->type();
}

Endian FloatField::endian() const
{
    return cast(m_pImpl)->endian();
}

double FloatField::defaultValue() const
{
    return cast(m_pImpl)->defaultValue();
}

const FloatField::ValidRangesList& FloatField::validRanges() const
{
    return cast(m_pImpl)->validRanges();
}

const FloatField::SpecialValues& FloatField::specialValues() const
{
    return cast(m_pImpl)->specialValues();
}

bool FloatField::validCheckVersion() const
{
    return cast(m_pImpl)->validCheckVersion();
}

Units FloatField::units() const
{
    return cast(m_pImpl)->units();
}

unsigned FloatField::displayDecimals() const
{
    return cast(m_pImpl)->displayDecimals();
}

bool FloatField::hasNonUniqueSpecials() const
{
    return cast(m_pImpl)->hasNonUniqueSpecials();
}

} // namespace parse

} // namespace commsdsl
