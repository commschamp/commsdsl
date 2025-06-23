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

#include "commsdsl/parse/ParseMessage.h"
#include <cassert>

#include "ParseMessageImpl.h"

namespace commsdsl
{

namespace parse
{

ParseMessage::ParseMessage(const ParseMessageImpl* impl)
  : m_pImpl(impl)
{
}

ParseMessage::ParseMessage(const ParseMessage &) = default;

ParseMessage::~ParseMessage() = default;

bool ParseMessage::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& ParseMessage::name() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->name();
}

const std::string& ParseMessage::displayName() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->displayName();
}

const std::string& ParseMessage::description() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->description();
}

std::uintmax_t ParseMessage::id() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->id();
}

unsigned ParseMessage::order() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->order();
}

std::size_t ParseMessage::minLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->minLength();
}

std::size_t ParseMessage::maxLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->maxLength();
}

unsigned ParseMessage::sinceVersion() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->getSinceVersion();
}

unsigned ParseMessage::deprecatedSince() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->getDeprecated();
}

bool ParseMessage::isDeprecatedRemoved() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isDeprecatedRemoved();
}

ParseMessage::FieldsList ParseMessage::fields() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->fieldsList();
}

ParseMessage::AliasesList ParseMessage::aliases() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->aliasesList();
}

std::string ParseMessage::externalRef(bool schemaRef) const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->externalRef(schemaRef);
}

bool ParseMessage::isCustomizable() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isCustomizable();
}

bool ParseMessage::isFailOnInvalid() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->isFailOnInvalid();
}

ParseMessage::Sender ParseMessage::sender() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->sender();
}

ParseOverrideType ParseMessage::readOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->readOverride();
}

ParseOverrideType ParseMessage::writeOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->writeOverride();
}

ParseOverrideType ParseMessage::refreshOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->refreshOverride();
}

ParseOverrideType ParseMessage::lengthOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->lengthOverride();
}

ParseOverrideType ParseMessage::validOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->validOverride();
}

ParseOverrideType ParseMessage::nameOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->nameOverride();
}

const std::string& ParseMessage::copyCodeFrom() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->copyCodeFrom();
}

ParseOptCond ParseMessage::construct() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->construct();
}

ParseOptCond ParseMessage::readCond() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->readCond();
}

ParseOptCond ParseMessage::validCond() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->validCond();
}

const ParseMessage::AttributesMap& ParseMessage::parseExtraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraAttributes();
}

const ParseMessage::ElementsList& ParseMessage::parseExtraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->extraChildren();
}

const ParseMessage::PlatformsList& ParseMessage::platforms() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->platforms();
}


} // namespace parse

} // namespace commsdsl
