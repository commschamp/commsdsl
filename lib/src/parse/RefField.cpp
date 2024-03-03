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

#include "commsdsl/parse/RefField.h"

#include <cassert>

#include "RefFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const RefFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const RefFieldImpl*>(ptr);
}

} // namespace

RefField::RefField(const RefFieldImpl* impl)
  : Base(impl)
{
}

RefField::RefField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Ref);
}

Field RefField::field() const
{
    return cast(m_pImpl)->field();
}

} // namespace parse

} // namespace commsdsl
