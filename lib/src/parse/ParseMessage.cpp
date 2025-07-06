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

#include "ParseMessageImpl.h"

#include <cassert>

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

bool ParseMessage::parseValid() const
{
    return m_pImpl != nullptr;
}

const std::string& ParseMessage::parseName() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseName();
}

const std::string& ParseMessage::parseDisplayName() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseDisplayName();
}

const std::string& ParseMessage::parseDescription() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseDescription();
}

std::uintmax_t ParseMessage::parseId() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseId();
}

unsigned ParseMessage::parseOrder() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseOrder();
}

std::size_t ParseMessage::parseMinLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseMinLength();
}

std::size_t ParseMessage::parseMaxLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseMaxLength();
}

unsigned ParseMessage::parseSinceVersion() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseGetSinceVersion();
}

unsigned ParseMessage::parseDeprecatedSince() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseGetDeprecated();
}

bool ParseMessage::parseIsDeprecatedRemoved() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseIsDeprecatedRemoved();
}

ParseMessage::FieldsList ParseMessage::parseFields() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseFieldsList();
}

ParseMessage::AliasesList ParseMessage::parseAliases() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseAliasesList();
}

std::string ParseMessage::parseExternalRef(bool schemaRef) const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExternalRef(schemaRef);
}

bool ParseMessage::parseIsCustomizable() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseIsCustomizable();
}

bool ParseMessage::parseIsFailOnInvalid() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseIsFailOnInvalid();
}

ParseMessage::Sender ParseMessage::parseSender() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseSender();
}

ParseOverrideType ParseMessage::parseReadOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseReadOverride();
}

ParseOverrideType ParseMessage::parseWriteOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseWriteOverride();
}

ParseOverrideType ParseMessage::parseRefreshOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseRefreshOverride();
}

ParseOverrideType ParseMessage::parseLengthOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseLengthOverride();
}

ParseOverrideType ParseMessage::parseValidOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseValidOverride();
}

ParseOverrideType ParseMessage::parseNameOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseNameOverride();
}

const std::string& ParseMessage::parseCopyCodeFrom() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseCopyCodeFrom();
}

ParseOptCond ParseMessage::parseConstruct() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseConstruct();
}

ParseOptCond ParseMessage::parseReadCond() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseReadCond();
}

ParseOptCond ParseMessage::parseValidCond() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseValidCond();
}

const ParseMessage::AttributesMap& ParseMessage::parseExtraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraAttributes();
}

const ParseMessage::ElementsList& ParseMessage::parseExtraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraChildren();
}

const ParseMessage::PlatformsList& ParseMessage::parsePlatforms() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parsePlatforms();
}


} // namespace parse

} // namespace commsdsl
