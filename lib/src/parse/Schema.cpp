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

#include "commsdsl/parse/Schema.h"

#include <limits>
#include <cassert>

#include "SchemaImpl.h"
#include "common.h"

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

Schema::Schema(const SchemaImpl* impl)
  : m_pImpl(impl)
{
}

bool Schema::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& Schema::name() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return common::emptyString();
    }

    return m_pImpl->name();
}

const std::string& Schema::description() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return common::emptyString();
    }

    return m_pImpl->description();
}

unsigned Schema::id() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return std::numeric_limits<unsigned>::max();
    }

    return m_pImpl->id();
}

unsigned Schema::version() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return std::numeric_limits<unsigned>::max();
    }

    return m_pImpl->version();
}

unsigned Schema::dslVersion() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return std::numeric_limits<unsigned>::max();
    }

    return m_pImpl->dslVersion();
}

Endian Schema::endian() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return Endian_NumOfValues;
    }

    return m_pImpl->endian();
}

bool Schema::nonUniqueMsgIdAllowed() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        return false;
    }

    return m_pImpl->nonUniqueMsgIdAllowed();
}

const Schema::AttributesMap& Schema::extraAttributes() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        static const AttributesMap Map;
        return Map;
    }

    return m_pImpl->extraAttributes();
}

const Schema::ElementsList& Schema::extraElements() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        static const ElementsList List;
        return List;
    }

    return m_pImpl->extraChildrenElements();

}

Schema::NamespacesList Schema::namespaces() const
{
    if (!valid()) {
        assert(Unexpected_call_on_invalid_schema_object);
        static const NamespacesList List;
        return List;
    }

    return m_pImpl->namespacesList();
}

const Schema::PlatformsList& Schema::platforms() const
{
    return m_pImpl->platforms();
}

Schema::MessagesList Schema::allMessages() const
{
    return m_pImpl->allMessages();
}

std::string Schema::externalRef() const
{
    return m_pImpl->externalRef();
}

} // namespace parse

} // namespace commsdsl
