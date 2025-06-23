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

#include <limits>
#include <cassert>

#include "ParseSchemaImpl.h"
#include "parse_common.h"

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

bool ParseSchema::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& ParseSchema::name() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return common::emptyString();
    }

    return m_pImpl->name();
}

const std::string& ParseSchema::description() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return common::emptyString();
    }

    return m_pImpl->description();
}

unsigned ParseSchema::id() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return std::numeric_limits<unsigned>::max();
    }

    return m_pImpl->id();
}

unsigned ParseSchema::version() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return std::numeric_limits<unsigned>::max();
    }

    return m_pImpl->version();
}

unsigned ParseSchema::dslVersion() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return std::numeric_limits<unsigned>::max();
    }

    return m_pImpl->dslVersion();
}

ParseEndian ParseSchema::endian() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return ParseEndian_NumOfValues;
    }

    return m_pImpl->endian();
}

bool ParseSchema::nonUniqueMsgIdAllowed() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return false;
    }

    return m_pImpl->nonUniqueMsgIdAllowed();
}

const ParseSchema::AttributesMap& ParseSchema::parseExtraAttributes() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        static const AttributesMap Map;
        return Map;
    }

    return m_pImpl->parseExtraAttributes();
}

const ParseSchema::ElementsList& ParseSchema::parseExtraElements() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        static const ElementsList List;
        return List;
    }

    return m_pImpl->extraChildrenElements();

}

ParseSchema::NamespacesList ParseSchema::namespaces() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        static const NamespacesList List;
        return List;
    }

    return m_pImpl->namespacesList();
}

const ParseSchema::PlatformsList& ParseSchema::platforms() const
{
    return m_pImpl->platforms();
}

ParseSchema::MessagesList ParseSchema::allMessages() const
{
    return m_pImpl->allMessages();
}

std::string ParseSchema::externalRef() const
{
    return m_pImpl->externalRef();
}

} // namespace parse

} // namespace commsdsl
