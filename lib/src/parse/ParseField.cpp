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

bool ParseField::parseValid() const
{
    return m_pImpl != nullptr;
}

const std::string& ParseField::parseName() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseName();
}

const std::string& ParseField::parseDisplayName() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseDisplayName();
}

const std::string& ParseField::parseDescription() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseDescription();
}

ParseField::Kind ParseField::parseKind() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseKind();
}

ParseField::SemanticType ParseField::parseSemanticType() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseSemanticType();
}

std::size_t ParseField::parseMinLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseMinLength();
}

std::size_t ParseField::parseMaxLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseMaxLength();
}

std::size_t ParseField::parseBitLength() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseBitLength();
}

unsigned ParseField::parseSinceVersion() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseGetSinceVersion();
}

unsigned ParseField::parseDeprecatedSince() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseGetDeprecated();
}

bool ParseField::parseIsDeprecatedRemoved() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseIsDeprecatedRemoved();
}

std::string ParseField::parseExternalRef(bool schemaRef) const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExternalRef(schemaRef);
}

bool ParseField::parseIsPseudo() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseIsPseudo();
}

bool ParseField::parseIsFixedValue() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseIsFixedValue();
}

bool ParseField::parseIsCustomizable() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseIsCustomizable();
}

bool ParseField::parseIsFailOnInvalid() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseIsFailOnInvalid();
}

bool ParseField::parseIsForceGen() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseIsForceGen();
}

std::string ParseField::parseSchemaPos() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseSchemaPos();
}

ParseOverrideType ParseField::parseValueOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseValueOverride();
}

ParseOverrideType ParseField::parseReadOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseReadOverride();
}

ParseOverrideType ParseField::parseWriteOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseWriteOverride();
}

ParseOverrideType ParseField::parseRefreshOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseRefreshOverride();
}

ParseOverrideType ParseField::parseLengthOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseLengthOverride();
}

ParseOverrideType ParseField::parseValidOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseValidOverride();
}

ParseOverrideType ParseField::parseNameOverride() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseNameOverride();
}

const std::string& ParseField::parseCopyCodeFrom() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseCopyCodeFrom();
}

bool ParseField::parseIsValidInnerRef(const std::string& refStr) const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseIsValidInnerRef(refStr);
}

const ParseField::AttributesMap& ParseField::parseExtraAttributes() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraAttributes();
}

const ParseField::ElementsList& ParseField::parseExtraElements() const
{
    assert(m_pImpl != nullptr);
    return m_pImpl->parseExtraChildren();
}

} // namespace parse

} // namespace commsdsl
