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

#include "commsdsl/parse/ParseNamespace.h"

#include "ParseNamespaceImpl.h"

#include <cassert>

namespace commsdsl
{

namespace parse
{

ParseNamespace::ParseNamespace(const ParseNamespaceImpl* impl)
  : m_pImpl(impl)
{
}

ParseNamespace::ParseNamespace(const ParseNamespace &) = default;

ParseNamespace::~ParseNamespace() = default;

bool ParseNamespace::parseValid() const
{
    return m_pImpl != nullptr;
}

const std::string& ParseNamespace::parseName() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseName();
}

const std::string& ParseNamespace::parseDescription() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseDescription();
}

ParseNamespace::ParseNamespacesList ParseNamespace::parseNamespaces() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseNamespacesList();
}

ParseNamespace::ParseFieldsList ParseNamespace::parseFields() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseFieldsList();
}

ParseNamespace::ParseMessagesList ParseNamespace::parseMessages() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseMessagesList();
}

bool ParseNamespace::parseHasMessages() const
{
    assert(m_pImpl != nullptr);
    return !m_pImpl->parseMessages().empty();
}

ParseNamespace::ParseInterfacesList ParseNamespace::parseInterfaces() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseInterfacesList();
}

ParseNamespace::ParseFramesList ParseNamespace::parseFrames() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseFramesList();
}

std::string ParseNamespace::parseExternalRef(bool schemaRef) const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExternalRef(schemaRef);
}

const ParseNamespace::ParseAttributesMap& ParseNamespace::parseExtraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraAttributes();
}

const ParseNamespace::ParseElementsList& ParseNamespace::parseExtraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraChildren();
}


} // namespace parse

} // namespace commsdsl
