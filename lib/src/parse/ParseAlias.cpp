//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/parse/ParseAlias.h"
#include <cassert>

#include "ParseAliasImpl.h"

namespace commsdsl
{

namespace parse
{

ParseAlias::ParseAlias(const ParseAliasImpl* impl)
  : m_pImpl(impl)
{
}

ParseAlias::ParseAlias(const ParseAlias &) = default;

ParseAlias::~ParseAlias() = default;

const std::string& ParseAlias::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& ParseAlias::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

const std::string& ParseAlias::fieldName() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->fieldName();
}

const ParseAlias::AttributesMap& ParseAlias::extraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraAttributes();
}

const ParseAlias::ElementsList& ParseAlias::extraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}

} // namespace parse

} // namespace commsdsl
