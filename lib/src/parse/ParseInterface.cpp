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

#include "commsdsl/parse/ParseInterface.h"

#include "ParseInterfaceImpl.h"

#include <cassert>

namespace commsdsl
{

namespace parse
{

ParseInterface::ParseInterface(const ParseInterfaceImpl* impl)
  : m_pImpl(impl)
{
}

ParseInterface::ParseInterface(const ParseInterface &) = default;

ParseInterface::~ParseInterface() = default;

bool ParseInterface::parseValid() const
{
    return m_pImpl != nullptr;
}

const std::string& ParseInterface::parseName() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseName();
}

const std::string& ParseInterface::parseDescription() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseDescription();
}

ParseInterface::FieldsList ParseInterface::parseFields() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseFieldsList();
}

ParseInterface::AliasesList ParseInterface::parseAliases() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseAliasesList();
}

std::string ParseInterface::parseExternalRef(bool schemaRef) const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExternalRef(schemaRef);
}

const std::string& ParseInterface::parseCopyCodeFrom() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseCopyCodeFrom();
}

const ParseInterface::AttributesMap& ParseInterface::parseExtraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraAttributes();
}

const ParseInterface::ElementsList& ParseInterface::parseExtraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraChildren();
}

} // namespace parse

} // namespace commsdsl
