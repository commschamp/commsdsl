//
// Copyright 2018 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/OptionalField.h"

#include <cassert>

#include "OptionalFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const OptionalFieldImpl* cast(const FieldImpl* ptr)
{
    assert(ptr != nullptr);
    return static_cast<const OptionalFieldImpl*>(ptr);
}

} // namespace

OptionalField::OptionalField(const OptionalFieldImpl* impl)
  : Base(impl)
{
}

OptionalField::OptionalField(Field field)
  : Base(field)
{
    assert(kind() == Kind::Optional);
}

OptionalField::Mode OptionalField::defaultMode() const
{
    return cast(m_pImpl)->defaultMode();
}

Field OptionalField::field() const
{
    return cast(m_pImpl)->field();
}

OptCond OptionalField::cond() const
{
    return cast(m_pImpl)->wrappedCondition();
}

bool OptionalField::externalModeCtrl() const
{
    return cast(m_pImpl)->externalModeCtrl();
}

bool OptionalField::missingOnReadFail() const
{
    return cast(m_pImpl)->missingOnReadFail();
}

} // namespace parse

} // namespace commsdsl
