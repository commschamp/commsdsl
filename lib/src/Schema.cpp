//
// Copyright 2018 - 2020 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/Schema.h"

#include <limits>
#include <cassert>

#include "SchemaImpl.h"
#include "common.h"

namespace commsdsl
{

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
        assert(!"Unexpected call on invalid schema object");
        return common::emptyString();
    }

    return m_pImpl->name();
}

const std::string& Schema::description() const
{
    if (!valid()) {
        assert(!"Unexpected call on invalid schema object");
        return common::emptyString();
    }

    return m_pImpl->description();
}

unsigned Schema::id() const
{
    if (!valid()) {
        assert(!"Unexpected call on invalid schema object");
        return std::numeric_limits<unsigned>::max();
    }

    return m_pImpl->id();
}

unsigned Schema::version() const
{
    if (!valid()) {
        assert(!"Unexpected call on invalid schema object");
        return std::numeric_limits<unsigned>::max();
    }

    return m_pImpl->version();
}

unsigned Schema::dslVersion() const
{
    if (!valid()) {
        assert(!"Unexpected call on invalid schema object");
        return std::numeric_limits<unsigned>::max();
    }

    return m_pImpl->dslVersion();
}

Endian Schema::endian() const
{
    if (!valid()) {
        assert(!"Unexpected call on invalid schema object");
        return Endian_NumOfValues;
    }

    return m_pImpl->endian();
}

bool Schema::nonUniqueMsgIdAllowed() const
{
    if (!valid()) {
        assert(!"Unexpected call on invalid schema object");
        return false;
    }

    return m_pImpl->nonUniqueMsgIdAllowed();
}

const Schema::AttributesMap& Schema::extraAttributes() const
{
    if (!valid()) {
        assert(!"Unexpected call on invalid schema object");
        static const AttributesMap Map;
        return Map;
    }

    return m_pImpl->extraAttributes();
}

const Schema::ElementsList& Schema::extraElements() const
{
    if (!valid()) {
        assert(!"Unexpected call on invalid schema object");
        static const ElementsList List;
        return List;
    }

    return m_pImpl->extraChildrenElements();

}

} // namespace commsdsl
