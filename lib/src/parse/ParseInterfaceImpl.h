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

#pragma once

#include "commsdsl/parse/ParseInterface.h"
#include "commsdsl/parse/ParseProtocol.h"
#include "ParseAliasImpl.h"
#include "ParseBundleFieldImpl.h"
#include "ParseFieldImpl.h"
#include "ParseLogger.h"
#include "ParseObject.h"
#include "ParseXmlWrap.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
class ParseInterfaceImpl final : public ParseObject
{
    using Base = ParseObject;
public:
    using ParsePtr = std::unique_ptr<ParseInterfaceImpl>;
    using ParsePropsMap = ParseXmlWrap::ParsePropsMap;
    using ParseFieldsList = ParseInterface::ParseFieldsList;
    using ParseAliasesList = ParseInterface::ParseAliasesList;
    using ParseContentsList = ParseXmlWrap::ParseContentsList;
    using ParseImplFieldsList = std::vector<ParseFieldImpl*>;
    using ParseFieldRefInfo = ParseFieldImpl::ParseFieldRefInfo;
    using ParseFieldRefType = ParseFieldImpl::ParseFieldRefType;

    ParseInterfaceImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseInterfaceImpl(const ParseInterfaceImpl&) = delete;
    ParseInterfaceImpl(ParseInterfaceImpl&&) = default;
    virtual ~ParseInterfaceImpl() = default;

    ::xmlNodePtr parseGetNode() const
    {
        return m_node;
    }

    bool parse();

    const ParsePropsMap& parseProps() const
    {
        return m_props;
    }

    const std::string& parseName() const;
    const std::string& parseDisplayName() const;
    const std::string& parseDescription() const;
    const std::string& parseCopyCodeFrom() const;

    ParseFieldsList parseFieldsList() const;
    ParseAliasesList parseAliasesList() const;

    std::string parseExternalRef(bool schemaRef) const;

    const ParsePropsMap& parseExtraAttributes() const
    {
        return m_extraAttrs;
    }

    const ParseContentsList& parseExtraChildren() const
    {
        return m_extraChildren;
    }

    std::size_t parseFindFieldIdx(const std::string& name) const;

    ParseImplFieldsList parseAllImplFields() const;

    ParseFieldRefInfo processInnerFieldRef(const std::string refStr) const;

protected:

    virtual ParseObjKind parseObjKindImpl() const override;

private:
    struct ParseReusableState
    {
        std::string m_name;
        std::string m_description;
        std::vector<ParseFieldImplPtr> m_fields;
        std::vector<ParseAliasImplPtr> m_aliases;
        std::string m_copyCodeFrom;

        ParseReusableState() = default;
        ParseReusableState(ParseReusableState&&) = default;

        auto basicForwardAsTuple()
        {
            return
                std::forward_as_tuple(
                    m_name,
                    m_description,
                    // m_fields,
                    // m_aliases,
                    m_copyCodeFrom
                );
        }

        auto basicForwardAsTuple() const
        {
            return const_cast<ParseReusableState*>(this)->basicForwardAsTuple();
        }

        ParseReusableState& operator=(const ParseReusableState& other)
        {
            basicForwardAsTuple() = other.basicForwardAsTuple();

            m_fields.clear();
            m_fields.reserve(other.m_fields.size());
            for (auto& f : other.m_fields) {
                m_fields.push_back(f->parseClone());
            }

            m_aliases.clear();
            m_aliases.reserve(other.m_aliases.size());
            for (auto& a : other.m_aliases) {
                m_aliases.push_back(a->parseClone());
            }

            return *this;
        }
    };

    ParseLogWrapper parseLogError() const;
    ParseLogWrapper parseLogWarning() const;
    ParseLogWrapper parseLogInfo() const;

    static const ParseXmlWrap::ParseNamesList& parseCommonProps();
    static ParseXmlWrap::ParseNamesList parseAllNames();

    bool parseValidateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave = false);
    bool parseValidateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool parseValidateAndUpdateStringPropValue(const std::string& str, std::string& value, bool mustHave = false);
    void parseReportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    bool parseCheckReuse();
    bool parseUpdateName();
    bool parseUpdateDescription();
    bool parseCopyFields();
    bool parseUpdateFields();
    bool parseCopyAliases();
    bool parseUpdateAliases();
    void parseCloneFieldsFrom(const ParseInterfaceImpl& other);
    void parseCloneFieldsFrom(const ParseBundleFieldImpl& other);
    void parseCloneAliasesFrom(const ParseInterfaceImpl& other);
    void parseCloneAliasesFrom(const ParseBundleFieldImpl& other);
    bool parseUpdateExtraAttrs();
    bool parseUpdateExtraChildren();

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;
    ParsePropsMap m_props;
    ParsePropsMap m_extraAttrs;
    ParseContentsList m_extraChildren;
    ParseReusableState m_state;

    const ParseInterfaceImpl* m_copyFieldsFromInterface = nullptr;
    const ParseBundleFieldImpl* m_copyFieldsFromBundle = nullptr;
};

using ParseInterfaceImplPtr = ParseInterfaceImpl::ParsePtr;

} // namespace parse

} // namespace commsdsl
