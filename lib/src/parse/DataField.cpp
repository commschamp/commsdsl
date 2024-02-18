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

#include "commsdsl/parse/DataField.h"

#include <cassert>

#include "DataFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const DataFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const DataFieldImpl*>(ptr);
}

} // namespace

DataField::DataField(const DataFieldImpl* impl)
  : Base(impl)
{
}

DataField::DataField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Data);
}

const DataField::ValueType& DataField::defaultValue() const
{
    return cast(m_pImpl)->defaultValue();
}

std::size_t DataField::fixedLength() const
{
    return cast(m_pImpl)->length();
}

bool DataField::hasLengthPrefixField() const
{
    return cast(m_pImpl)->hasPrefixField();
}

Field DataField::lengthPrefixField() const
{
    return cast(m_pImpl)->prefixField();
}

const std::string& DataField::detachedPrefixFieldName() const
{
    return cast(m_pImpl)->detachedPrefixFieldName();
}

} // namespace parse

} // namespace commsdsl
