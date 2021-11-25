//
// Copyright 2019 - 2021 (C). Alex Robenko. All rights reserved.
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


#pragma once

#include <string>
#include <memory>

#include "XmlWrap.h"
#include "FieldImpl.h"

namespace commsdsl
{

class ProtocolImpl;
class AliasImpl
{
public:
    using PropsMap = XmlWrap::PropsMap;
    using ContentsList = XmlWrap::ContentsList;
    using Ptr = std::unique_ptr<AliasImpl>;

    const std::string& name() const
    {
        return m_state.m_name;
    }

    const std::string& description() const
    {
        return m_state.m_description;
    }

    const std::string& fieldName() const
    {
        return m_state.m_fieldName;
    }

    const PropsMap& extraAttributes() const
    {
        return m_state.m_extraAttrs;
    }

    PropsMap& extraAttributes()
    {
        return m_state.m_extraAttrs;
    }

    const ContentsList& extraChildren() const
    {
        return m_state.m_extraChildren;
    }

    ContentsList& extraChildren()
    {
        return m_state.m_extraChildren;
    }

    Ptr clone() const;

    static Ptr create(::xmlNodePtr node, ProtocolImpl& protocol);

    bool parse();

    ::xmlNodePtr getNode() const
    {
        return m_node;
    }

    bool verifyAlias(const std::vector<Ptr>& aliases, const std::vector<FieldImplPtr>& fields) const;

protected:
    AliasImpl(::xmlNodePtr node, ProtocolImpl& protocol) : m_node(node), m_protocol(protocol) {}

private:

    bool updateName(const PropsMap& props);
    bool updateDescription(const PropsMap& props);
    bool updateFieldName(const PropsMap& props);
    bool validateAndUpdateStringPropValue(
        const PropsMap& props,
        const std::string& str,
        std::string& value,
        bool mustHave = false,
        bool allowDeref = false);
    bool validateSinglePropInstance(const PropsMap& props, const std::string& str, bool mustHave);
    void reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    bool updateExtraAttrs(const XmlWrap::NamesList& names);
    bool updateExtraChildren(const XmlWrap::NamesList& names);

    ::xmlNodePtr m_node = nullptr;
    ProtocolImpl& m_protocol;

    struct State
    {
        std::string m_name;
        std::string m_description;
        std::string m_fieldName;
        PropsMap m_extraAttrs;
        ContentsList m_extraChildren;
    };

    State m_state;
};

using AliasImplPtr = AliasImpl::Ptr;

} // namespace commsdsl
