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

#include <cassert>

#include "ParseNamespaceImpl.h"

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

bool ParseNamespace::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& ParseNamespace::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& ParseNamespace::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

ParseNamespace::NamespacesList ParseNamespace::namespaces() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->namespacesList();
}

ParseNamespace::FieldsList ParseNamespace::fields() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->fieldsList();
}

ParseNamespace::MessagesList ParseNamespace::messages() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->messagesList();
}

bool ParseNamespace::hasMessages() const
{
    assert(m_pImpl != nullptr);
    return !m_pImpl->messages().empty();
}

ParseNamespace::InterfacesList ParseNamespace::interfaces() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->interfacesList();
}

ParseNamespace::FramesList ParseNamespace::frames() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->framesList();
}

std::string ParseNamespace::externalRef(bool schemaRef) const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->externalRef(schemaRef);
}

const ParseNamespace::AttributesMap& ParseNamespace::parseExtraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraAttributes();
}

const ParseNamespace::ElementsList& ParseNamespace::parseExtraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}


} // namespace parse

} // namespace commsdsl
