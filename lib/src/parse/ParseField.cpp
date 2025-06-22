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

#include "commsdsl/parse/ParseField.h"
#include <cassert>

#include "ParseFieldImpl.h"

namespace commsdsl
{

namespace parse
{

ParseField::ParseField(const ParseFieldImpl* impl)
  : m_pImpl(impl)
{
}

ParseField::ParseField(const ParseField &) = default;

ParseField::~ParseField() = default;

bool ParseField::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& ParseField::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& ParseField::displayName() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->displayName();
}

const std::string& ParseField::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

ParseField::Kind ParseField::kind() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->kind();
}

ParseField::SemanticType ParseField::semanticType() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->semanticType();
}

std::size_t ParseField::minLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->minLength();
}

std::size_t ParseField::maxLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->maxLength();
}

std::size_t ParseField::bitLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->bitLength();
}

unsigned ParseField::sinceVersion() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->getSinceVersion();
}

unsigned ParseField::deprecatedSince() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->getDeprecated();
}

bool ParseField::isDeprecatedRemoved() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isDeprecatedRemoved();
}

std::string ParseField::externalRef(bool schemaRef) const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->externalRef(schemaRef);
}

bool ParseField::isPseudo() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isPseudo();
}

bool ParseField::isFixedValue() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isFixedValue();
}

bool ParseField::isCustomizable() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isCustomizable();
}

bool ParseField::isFailOnInvalid() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isFailOnInvalid();
}

bool ParseField::isForceGen() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isForceGen();
}

std::string ParseField::schemaPos() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->schemaPos();
}

ParseOverrideType ParseField::valueOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->valueOverride();
}

ParseOverrideType ParseField::readOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->readOverride();
}

ParseOverrideType ParseField::writeOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->writeOverride();
}

ParseOverrideType ParseField::refreshOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->refreshOverride();
}

ParseOverrideType ParseField::lengthOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->lengthOverride();
}

ParseOverrideType ParseField::validOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->validOverride();
}

ParseOverrideType ParseField::nameOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->nameOverride();
}

const std::string& ParseField::copyCodeFrom() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->copyCodeFrom();
}

bool ParseField::isValidInnerRef(const std::string& refStr) const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isValidInnerRef(refStr);
}

const ParseField::AttributesMap& ParseField::extraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraAttributes();
}

const ParseField::ElementsList& ParseField::extraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}

} // namespace parse

} // namespace commsdsl
