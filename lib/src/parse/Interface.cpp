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

#include "commsdsl/parse/Interface.h"
#include <cassert>

#include "InterfaceImpl.h"

namespace commsdsl
{

namespace parse
{

Interface::Interface(const InterfaceImpl* impl)
  : m_pImpl(impl)
{
}

Interface::Interface(const Interface &) = default;

Interface::~Interface() = default;

bool Interface::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& Interface::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& Interface::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

Interface::FieldsList Interface::fields() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->fieldsList();
}

Interface::AliasesList Interface::aliases() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->aliasesList();
}

std::string Interface::externalRef(bool schemaRef) const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->externalRef(schemaRef);
}

const std::string& Interface::copyCodeFrom() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->copyCodeFrom();
}

const Interface::AttributesMap& Interface::extraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraAttributes();
}

const Interface::ElementsList& Interface::extraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}

} // namespace parse

} // namespace commsdsl
