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
#include <cassert>

#include "ParseInterfaceImpl.h"

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

bool ParseInterface::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& ParseInterface::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& ParseInterface::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

ParseInterface::FieldsList ParseInterface::fields() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->fieldsList();
}

ParseInterface::AliasesList ParseInterface::aliases() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->aliasesList();
}

std::string ParseInterface::externalRef(bool schemaRef) const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->externalRef(schemaRef);
}

const std::string& ParseInterface::copyCodeFrom() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->copyCodeFrom();
}

const ParseInterface::AttributesMap& ParseInterface::parseExtraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraAttributes();
}

const ParseInterface::ElementsList& ParseInterface::parseExtraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}

} // namespace parse

} // namespace commsdsl
