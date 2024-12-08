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

#include "commsdsl/parse/Field.h"
#include <cassert>

#include "FieldImpl.h"

namespace commsdsl
{

namespace parse
{

Field::Field(const FieldImpl* impl)
  : m_pImpl(impl)
{
}

Field::Field(const Field &) = default;

Field::~Field() = default;

bool Field::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& Field::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& Field::displayName() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->displayName();
}

const std::string& Field::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

Field::Kind Field::kind() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->kind();
}

Field::SemanticType Field::semanticType() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->semanticType();
}

std::size_t Field::minLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->minLength();
}

std::size_t Field::maxLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->maxLength();
}

std::size_t Field::bitLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->bitLength();
}

unsigned Field::sinceVersion() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->getSinceVersion();
}

unsigned Field::deprecatedSince() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->getDeprecated();
}

bool Field::isDeprecatedRemoved() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isDeprecatedRemoved();
}

std::string Field::externalRef(bool schemaRef) const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->externalRef(schemaRef);
}

bool Field::isPseudo() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isPseudo();
}

bool Field::isCustomizable() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isCustomizable();
}

bool Field::isFailOnInvalid() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isFailOnInvalid();
}

bool Field::isForceGen() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isForceGen();
}

std::string Field::schemaPos() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->schemaPos();
}

OverrideType Field::valueOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->valueOverride();
}

OverrideType Field::readOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->readOverride();
}

OverrideType Field::writeOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->writeOverride();
}

OverrideType Field::refreshOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->refreshOverride();
}

OverrideType Field::lengthOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->lengthOverride();
}

OverrideType Field::validOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->validOverride();
}

OverrideType Field::nameOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->nameOverride();
}

const std::string& Field::copyCodeFrom() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->copyCodeFrom();
}

bool Field::isValidInnerRef(const std::string& refStr) const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isValidInnerRef(refStr);
}

const Field::AttributesMap& Field::extraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraAttributes();
}

const Field::ElementsList& Field::extraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}

} // namespace parse

} // namespace commsdsl
