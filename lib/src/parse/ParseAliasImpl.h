//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "ParseFieldImpl.h"
#include "ParseXmlWrap.h"

#include <memory>
#include <string>

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
class ParseAliasImpl
{
public:
    using PropsMap = ParseXmlWrap::PropsMap;
    using ContentsList = ParseXmlWrap::ContentsList;
    using Ptr = std::unique_ptr<ParseAliasImpl>;

    const std::string& parseName() const
    {
        return m_state.m_name;
    }

    const std::string& parseDescription() const
    {
        return m_state.m_description;
    }

    const std::string& parseFieldName() const
    {
        return m_state.m_fieldName;
    }

    const PropsMap& parseExtraAttributes() const
    {
        return m_state.m_extraAttrs;
    }

    PropsMap& parseExtraAttributes()
    {
        return m_state.m_extraAttrs;
    }

    const ContentsList& parseExtraChildren() const
    {
        return m_state.m_extraChildren;
    }

    ContentsList& parseExtraChildren()
    {
        return m_state.m_extraChildren;
    }

    Ptr parseClone() const;

    static Ptr parseCreate(::xmlNodePtr node, ParseProtocolImpl& protocol);

    bool parse();

    ::xmlNodePtr parseGetNode() const
    {
        return m_node;
    }

    bool parseVerifyAlias(const std::vector<Ptr>& aliases, const std::vector<ParseFieldImplPtr>& fields) const;

protected:
    ParseAliasImpl(::xmlNodePtr node, ParseProtocolImpl& protocol) : m_node(node), m_protocol(protocol) {}

private:

    bool parseUpdateName(const PropsMap& props);
    bool parseUpdateDescription(const PropsMap& props);
    bool parseUpdateFieldName(const PropsMap& props);
    bool parseValidateAndUpdateStringPropValue(
        const PropsMap& props,
        const std::string& str,
        std::string& value,
        bool mustHave = false,
        bool allowDeref = false);
    bool parseValidateSinglePropInstance(const PropsMap& props, const std::string& str, bool mustHave);
    void parseReportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    bool parseUpdateExtraAttrs(const ParseXmlWrap::NamesList& names);
    bool parseUpdateExtraChildren(const ParseXmlWrap::NamesList& names);

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;

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

using ParseAliasImplPtr = ParseAliasImpl::Ptr;

} // namespace parse

} // namespace commsdsl
