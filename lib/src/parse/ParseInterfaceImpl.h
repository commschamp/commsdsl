//
// Copyright 2018 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

    [[nodiscard]] ::xmlNodePtr parseGetNode() const
    {
        return m_node;
    }

    [[nodiscard]] bool parse();

    [[nodiscard]] const ParsePropsMap& parseProps() const
    {
        return m_props;
    }

    [[nodiscard]] const std::string& parseName() const;
    [[nodiscard]] const std::string& parseDisplayName() const;
    [[nodiscard]] const std::string& parseDescription() const;
    [[nodiscard]] const std::string& parseCopyCodeFrom() const;

    [[nodiscard]] ParseFieldsList parseFieldsList() const;
    [[nodiscard]] ParseAliasesList parseAliasesList() const;

    [[nodiscard]] std::string parseExternalRef(bool schemaRef) const;

    [[nodiscard]] const ParsePropsMap& parseExtraAttributes() const
    {
        return m_extraAttrs;
    }

    [[nodiscard]] const ParseContentsList& parseExtraChildren() const
    {
        return m_extraChildren;
    }

    [[nodiscard]] std::size_t parseFindFieldIdx(const std::string& name) const;

    [[nodiscard]] ParseImplFieldsList parseAllImplFields() const;

    [[nodiscard]] ParseFieldRefInfo processInnerFieldRef(const std::string refStr) const;

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

        [[nodiscard]] auto parseForwardAsTuple()
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

        [[nodiscard]] auto parseForwardAsTuple() const
        {
            return const_cast<ParseReusableState*>(this)->parseForwardAsTuple();
        }

        ParseReusableState& operator=(const ParseReusableState& other)
        {
            parseForwardAsTuple() = other.parseForwardAsTuple();

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

    [[nodiscard]] ParseLogWrapper parseLogError() const;
    [[nodiscard]] ParseLogWrapper parseLogWarning() const;
    [[nodiscard]] ParseLogWrapper parseLogInfo() const;

    [[nodiscard]] static const ParseXmlWrap::ParseNamesList& parseCommonProps();
    [[nodiscard]] static ParseXmlWrap::ParseNamesList parseAllNames();

    [[nodiscard]] bool parseValidateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave = false);
    [[nodiscard]] bool parseValidateSinglePropInstance(const std::string& str, bool mustHave = false);
    [[nodiscard]] bool parseValidateAndUpdateStringPropValue(const std::string& str, std::string& value, bool mustHave = false);
    void parseReportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    [[nodiscard]] bool parseCheckReuse();
    [[nodiscard]] bool parseUpdateName();
    [[nodiscard]] bool parseUpdateDescription();
    [[nodiscard]] bool parseCopyFields();
    [[nodiscard]] bool parseUpdateFields();
    [[nodiscard]] bool parseCopyAliases();
    [[nodiscard]] bool parseUpdateAliases();
    void parseCloneFieldsFrom(const ParseInterfaceImpl& other);
    void parseCloneFieldsFrom(const ParseBundleFieldImpl& other);
    void parseCloneAliasesFrom(const ParseInterfaceImpl& other);
    void parseCloneAliasesFrom(const ParseBundleFieldImpl& other);
    [[nodiscard]] bool parseUpdateExtraAttrs();
    [[nodiscard]] bool parseUpdateExtraChildren();

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
