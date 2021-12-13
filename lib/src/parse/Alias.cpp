//
// Copyright 2019 - 2021 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/Alias.h"
#include <cassert>

#include "AliasImpl.h"

namespace commsdsl
{

namespace parse
{

Alias::Alias(const AliasImpl* impl)
  : m_pImpl(impl)
{
}

Alias::Alias(const Alias &) = default;

Alias::~Alias() = default;

const std::string& Alias::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& Alias::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

const std::string& Alias::fieldName() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->fieldName();
}

const Alias::AttributesMap& Alias::extraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraAttributes();
}

const Alias::ElementsList& Alias::extraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}

} // namespace parse

} // namespace commsdsl
