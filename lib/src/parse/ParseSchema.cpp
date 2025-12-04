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

#include "commsdsl/parse/ParseSchema.h"

#include "ParseSchemaImpl.h"
#include "parse_common.h"

#include <cassert>
#include <limits>

namespace commsdsl
{

namespace parse
{

namespace
{

#ifndef NDEBUG
constexpr bool Unexpected_call_on_invalid_schema_object = false;
#endif

}

ParseSchema::ParseSchema(const ParseSchemaImpl* impl)
  : m_pImpl(impl)
{
}

bool ParseSchema::parseValid() const
{
    return m_pImpl != nullptr;
}

const std::string& ParseSchema::parseName() const
{
    if (!parseValid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return common::parseEmptyString();
    }

    return m_pImpl->parseName();
}

const std::string& ParseSchema::parseDisplayName() const
{
    return m_pImpl->parseDisplayName();
}

const std::string& ParseSchema::parseDescription() const
{
    if (!parseValid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return common::parseEmptyString();
    }

    return m_pImpl->parseDescription();
}

unsigned ParseSchema::parseId() const
{
    if (!parseValid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return std::numeric_limits<unsigned>::max();
    }

    return m_pImpl->parseId();
}

unsigned ParseSchema::parseVersion() const
{
    if (!parseValid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return std::numeric_limits<unsigned>::max();
    }

    return m_pImpl->parseVersion();
}

unsigned ParseSchema::parseDslVersion() const
{
    if (!parseValid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return std::numeric_limits<unsigned>::max();
    }

    return m_pImpl->parseDslVersion();
}

ParseEndian ParseSchema::parseEndian() const
{
    if (!parseValid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return ParseEndian_NumOfValues;
    }

    return m_pImpl->parseEndian();
}

bool ParseSchema::parseNonUniqueMsgIdAllowed() const
{
    if (!parseValid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return false;
    }

    return m_pImpl->parseNonUniqueMsgIdAllowed();
}

const ParseSchema::ParseAttributesMap& ParseSchema::parseExtraAttributes() const
{
    if (!parseValid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        static const ParseAttributesMap Map;
        return Map;
    }

    return m_pImpl->parseExtraAttributes();
}

const ParseSchema::ParseElementsList& ParseSchema::parseExtraElements() const
{
    if (!parseValid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        static const ParseElementsList List;
        return List;
    }

    return m_pImpl->parseExtraChildrenElements();

}

ParseSchema::ParseNamespacesList ParseSchema::parseNamespaces() const
{
    if (!parseValid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        static const ParseNamespacesList List;
        return List;
    }

    return m_pImpl->parseNamespacesList();
}

const ParseSchema::ParsePlatformsList& ParseSchema::parsePlatforms() const
{
    return m_pImpl->parsePlatforms();
}

ParseSchema::ParseMessagesList ParseSchema::parseAllMessages() const
{
    return m_pImpl->parseAllMessages();
}

std::string ParseSchema::parseExternalRef() const
{
    return m_pImpl->parseExternalRef();
}

} // namespace parse

} // namespace commsdsl
