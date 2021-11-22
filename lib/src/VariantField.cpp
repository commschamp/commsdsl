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

#include "commsdsl/VariantField.h"

#include <cassert>

#include "VariantFieldImpl.h"

namespace commsdsl
{

namespace
{

const VariantFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const VariantFieldImpl*>(ptr);
}

} // namespace

VariantField::VariantField(const VariantFieldImpl* impl)
  : Base(impl)
{
}

VariantField::VariantField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Variant);
}

VariantField::Members VariantField::members() const
{
    return cast(m_pImpl)->membersList();
}

std::size_t VariantField::defaultMemberIdx() const
{
    return cast(m_pImpl)->defaultMemberIdx();
}

bool VariantField::displayIdxReadOnlyHidden() const
{
    return cast(m_pImpl)->displayIdxReadOnlyHidden();
}

} // namespace commsdsl
