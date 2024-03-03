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

#include "commsdsl/parse/Namespace.h"

#include <cassert>

#include "NamespaceImpl.h"

namespace commsdsl
{

namespace parse
{

Namespace::Namespace(const NamespaceImpl* impl)
  : m_pImpl(impl)
{
}

Namespace::Namespace(const Namespace &) = default;

Namespace::~Namespace() = default;

bool Namespace::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& Namespace::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& Namespace::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

Namespace::NamespacesList Namespace::namespaces() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->namespacesList();
}

Namespace::FieldsList Namespace::fields() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->fieldsList();
}

Namespace::MessagesList Namespace::messages() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->messagesList();
}

Namespace::InterfacesList Namespace::interfaces() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->interfacesList();
}

Namespace::FramesList Namespace::frames() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->framesList();
}

std::string Namespace::externalRef(bool schemaRef) const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->externalRef(schemaRef);
}

const Namespace::AttributesMap& Namespace::extraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraAttributes();
}

const Namespace::ElementsList& Namespace::extraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}


} // namespace parse

} // namespace commsdsl
