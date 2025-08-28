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
    using ParsePropsMap = ParseXmlWrap::ParsePropsMap;
    using ParseContentsList = ParseXmlWrap::ParseContentsList;
    using ParsePtr = std::unique_ptr<ParseAliasImpl>;

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

    const ParsePropsMap& parseExtraAttributes() const
    {
        return m_state.m_extraAttrs;
    }

    ParsePropsMap& parseExtraAttributes()
    {
        return m_state.m_extraAttrs;
    }

    const ParseContentsList& parseExtraChildren() const
    {
        return m_state.m_extraChildren;
    }

    ParseContentsList& parseExtraChildren()
    {
        return m_state.m_extraChildren;
    }

    ParsePtr parseClone() const;

    static ParsePtr parseCreate(::xmlNodePtr node, ParseProtocolImpl& protocol);

    bool parse();

    ::xmlNodePtr parseGetNode() const
    {
        return m_node;
    }

    bool parseVerifyAlias(const std::vector<ParsePtr>& aliases, const std::vector<ParseFieldImplPtr>& fields) const;

protected:
    ParseAliasImpl(::xmlNodePtr node, ParseProtocolImpl& protocol) : m_node(node), m_protocol(protocol) {}

private:

    bool parseUpdateName(const ParsePropsMap& props);
    bool parseUpdateDescription(const ParsePropsMap& props);
    bool parseUpdateFieldName(const ParsePropsMap& props);
    bool parseValidateAndUpdateStringPropValue(
        const ParsePropsMap& props,
        const std::string& str,
        std::string& value,
        bool mustHave = false,
        bool allowDeref = false);
    bool parseValidateSinglePropInstance(const ParsePropsMap& props, const std::string& str, bool mustHave);
    void parseReportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    bool parseUpdateExtraAttrs(const ParseXmlWrap::ParseNamesList& names);
    bool parseUpdateExtraChildren(const ParseXmlWrap::ParseNamesList& names);

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;

    struct ParseState
    {
        std::string m_name;
        std::string m_description;
        std::string m_fieldName;
        ParsePropsMap m_extraAttrs;
        ParseContentsList m_extraChildren;
    };

    ParseState m_state;
};

using ParseAliasImplPtr = ParseAliasImpl::ParsePtr;

} // namespace parse

} // namespace commsdsl
