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

#include "ParseAliasImpl.h"

#include <cassert>

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

const std::string& ParseAlias::parseName() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseName();
}

const std::string& ParseAlias::parseDescription() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseDescription();
}

const std::string& ParseAlias::parseFieldName() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseFieldName();
}

const ParseAlias::AttributesMap& ParseAlias::parseExtraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraAttributes();
}

const ParseAlias::ElementsList& ParseAlias::parseExtraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraChildren();
}

} // namespace parse

} // namespace commsdsl
