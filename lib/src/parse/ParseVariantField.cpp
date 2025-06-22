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

#include "commsdsl/parse/ParseVariantField.h"

#include <cassert>

#include "ParseVariantFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseVariantFieldImpl* cast(const ParseFieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const ParseVariantFieldImpl*>(ptr);
}

} // namespace

ParseVariantField::ParseVariantField(const ParseVariantFieldImpl* impl)
  : Base(impl)
{
}

ParseVariantField::ParseVariantField(ParseField field)
  : Base(field)
{
    assert(kind() == Kind::Variant);
}

ParseVariantField::Members ParseVariantField::members() const
{
    return cast(m_pImpl)->membersList();
}

std::size_t ParseVariantField::defaultMemberIdx() const
{
    return cast(m_pImpl)->defaultMemberIdx();
}

} // namespace parse

} // namespace commsdsl
