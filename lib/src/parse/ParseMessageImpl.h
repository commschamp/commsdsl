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

#include "commsdsl/parse/ParseMessage.h"

#include "ParseAliasImpl.h"
#include "ParseBundleFieldImpl.h"
#include "ParseFieldImpl.h"
#include "ParseLogger.h"
#include "ParseObject.h"
#include "ParseOptCondImpl.h"
#include "ParseXmlWrap.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
class ParseMessageImpl final : public ParseObject
{
    using Base = ParseObject;
public:
    using ParsePtr = std::unique_ptr<ParseMessageImpl>;
    using ParsePropsMap = ParseXmlWrap::ParsePropsMap;
    using ParseFieldsList = ParseMessage::ParseFieldsList;
    using ParseAliasesList = ParseMessage::ParseAliasesList;
    using ParseContentsList = ParseXmlWrap::ParseContentsList;
    using ParsePlatformsList = ParseMessage::ParsePlatformsList;
    using ParseSender = ParseMessage::ParseSender;

    ParseMessageImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseMessageImpl(const ParseMessageImpl&) = delete;
    ParseMessageImpl(ParseMessageImpl&&) = default;
    virtual ~ParseMessageImpl() = default;

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

    [[nodiscard]] std::uintmax_t parseId() const
    {
        return m_state.m_id;
    }

    [[nodiscard]] unsigned parseOrder() const
    {
        return m_state.m_order;
    }

    [[nodiscard]] std::size_t parseMinLength() const;

    [[nodiscard]] std::size_t parseMaxLength() const;

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

    [[nodiscard]] const ParsePlatformsList& parsePlatforms() const
    {
        return m_state.m_platforms;
    }

    [[nodiscard]] bool parseIsCustomizable() const
    {
        return m_state.m_customizable;
    }

    [[nodiscard]] bool parseIsFailOnInvalid() const
    {
        return m_state.m_failOnInvalid;
    }

    [[nodiscard]] ParseSender parseSender() const
    {
        return m_state.m_sender;
    }

    [[nodiscard]] ParseOverrideType parseReadOverride() const
    {
        return m_state.m_readOverride;
    }

    [[nodiscard]] ParseOverrideType parseWriteOverride() const
    {
        return m_state.m_writeOverride;
    }

    [[nodiscard]] ParseOverrideType parseRefreshOverride() const
    {
        return m_state.m_refreshOverride;
    }

    [[nodiscard]] ParseOverrideType parseLengthOverride() const
    {
        return m_state.m_lengthOverride;
    }

    [[nodiscard]] ParseOverrideType parseValidOverride() const
    {
        return m_state.m_validOverride;
    }

    [[nodiscard]] ParseOverrideType parseNameOverride() const
    {
        return m_state.m_nameOverride;
    }

    [[nodiscard]] const std::string& parseCopyCodeFrom() const
    {
        return m_state.m_copyCodeFrom;
    }

    [[nodiscard]] ParseOptCond parseConstruct() const
    {
        return ParseOptCond(m_state.m_construct.get());
    }

    [[nodiscard]] ParseOptCond parseReadCond() const
    {
        return ParseOptCond(m_state.m_readCond.get());
    }

    [[nodiscard]] ParseOptCond parseValidCond() const
    {
        return ParseOptCond(m_state.m_validCond.get());
    }

    [[nodiscard]] unsigned parseReadAlign() const
    {
        return m_state.m_readAlign;
    }

    [[nodiscard]] unsigned parseWritePadAlign() const
    {
        return m_state.m_writePadAlign;
    }

    [[nodiscard]] bool parseWritePadInLength() const
    {
        return m_state.m_writePadInLength;
    }

protected:
    virtual ParseObjKind parseObjKindImpl() const override;

private:
    struct ParseReusableState
    {
        std::string m_name;
        std::string m_displayName;
        std::string m_description;
        std::uintmax_t m_id = 0;
        unsigned m_order = 0;
        int m_validateMinLength = -1;
        unsigned m_readAlign = 0;
        unsigned m_writePadAlign = 0;
        std::vector<ParseFieldImplPtr> m_fields;
        std::vector<ParseAliasImplPtr> m_aliases;
        ParsePlatformsList m_platforms;
        ParseSender m_sender = ParseSender::Both;
        ParseOverrideType m_readOverride = ParseOverrideType_Any;
        ParseOverrideType m_writeOverride = ParseOverrideType_Any;
        ParseOverrideType m_refreshOverride = ParseOverrideType_Any;
        ParseOverrideType m_lengthOverride = ParseOverrideType_Any;
        ParseOverrideType m_validOverride = ParseOverrideType_Any;
        ParseOverrideType m_nameOverride = ParseOverrideType_Any;
        std::string m_copyCodeFrom;
        ParseOptCondImplPtr m_construct;
        ParseOptCondImplPtr m_readCond;
        ParseOptCondImplPtr m_validCond;
        bool m_customizable = false;
        bool m_failOnInvalid = false;
        bool m_writePadInLength = false;

        ParseReusableState() = default;
        ParseReusableState(ParseReusableState&&) = default;

        [[nodiscard]] auto parseForwardAsTuple()
        {
            return
                std::forward_as_tuple(
                    m_name,
                    m_displayName,
                    m_description,
                    m_id,
                    m_order,
                    m_validateMinLength,
                    m_readAlign,
                    m_writePadAlign,
                    // m_fields,
                    // m_aliases,
                    m_platforms,
                    m_sender,
                    m_readOverride,
                    m_writeOverride,
                    m_refreshOverride,
                    m_lengthOverride,
                    m_validOverride,
                    m_nameOverride,
                    m_copyCodeFrom,
                    // m_construct,
                    // m_readCond,
                    // m_validCond,
                    m_customizable,
                    m_failOnInvalid,
                    m_writePadInLength
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

            if (other.m_construct) {
                m_construct = other.m_construct->parseClone();
            }

            if (other.m_readCond) {
                m_readCond = other.m_readCond->parseClone();
            }

            if (other.m_validCond) {
                m_validCond = other.m_validCond->parseClone();
            }

            return *this;
        }
    };

    [[nodiscard]] ParseLogWrapper parseLogError() const;
    [[nodiscard]] ParseLogWrapper parseLogWarning() const;
    [[nodiscard]] ParseLogWrapper parseLogInfo() const;

    [[nodiscard]] static const ParseXmlWrap::ParseNamesList& parseCommonProps();
    [[nodiscard]] static const ParseXmlWrap::ParseNamesList& parseExtraProps();
    [[nodiscard]] static const ParseXmlWrap::ParseNamesList& parseAllProps();

    [[nodiscard]] static ParseXmlWrap::ParseNamesList parseAllNames();

    [[nodiscard]] bool parseValidateSinglePropInstance(const std::string& str, bool mustHave = false);
    [[nodiscard]] bool parseValidateAndUpdateStringPropValue(const std::string& str, std::string& value, bool mustHave = false, bool allowDeref = false);
    [[nodiscard]] bool parseValidateAndUpdateOverrideTypePropValue(const std::string& propName, ParseOverrideType& value);
    [[nodiscard]] bool parseValidateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave = false);
    [[nodiscard]] bool parseValidateAndUpdateUnsignedPropValue(const std::string& propName, unsigned& value, bool mustHave = false);
    void parseReportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    [[nodiscard]] bool parseCheckReuse();
    [[nodiscard]] bool parseUpdateName();
    [[nodiscard]] bool parseUpdateDescription();
    [[nodiscard]] bool parseUpdateDisplayName();
    [[nodiscard]] bool parseUpdateId();
    [[nodiscard]] bool parseUpdateOrder();
    [[nodiscard]] bool parseUpdateVersions();
    [[nodiscard]] bool parseUpdatePlatforms();
    [[nodiscard]] bool parseUpdateCustomizable();
    [[nodiscard]] bool parseUpdateSender();
    [[nodiscard]] bool parseUpdateValidateMinLength();
    [[nodiscard]] bool parseUpdateFailOnInvalid();
    [[nodiscard]] bool parseUpdateReadAlign();
    [[nodiscard]] bool parseUpdateWritePadAlign();
    [[nodiscard]] bool parseUpdateWritePadInLength();
    [[nodiscard]] bool parseCopyFields();
    [[nodiscard]] bool parseCopyAliases();
    [[nodiscard]] bool parseReplaceFields();
    [[nodiscard]] bool parseUpdateFields();
    [[nodiscard]] bool parseUpdateAliases();
    void parseCloneFieldsFrom(const ParseMessageImpl& other);
    void parseCloneFieldsFrom(const ParseBundleFieldImpl& other);
    void parseCloneAliasesFrom(const ParseMessageImpl& other);
    void parseCloneAliasesFrom(const ParseBundleFieldImpl& other);
    [[nodiscard]] bool parseUpdateReadOverride();
    [[nodiscard]] bool parseUpdateWriteOverride();
    [[nodiscard]] bool parseUpdateRefreshOverride();
    [[nodiscard]] bool parseUpdateLengthOverride();
    [[nodiscard]] bool parseUpdateValidOverride();
    [[nodiscard]] bool parseUpdateNameOverride();
    [[nodiscard]] bool parseUpdateCopyOverrideCodeFrom();
    [[nodiscard]] bool parseCopyConstruct();
    [[nodiscard]] bool parseCopyReadCond();
    [[nodiscard]] bool parseCopyValidCond();
    [[nodiscard]] bool parseUpdateSingleConstruct();
    [[nodiscard]] bool parseUpdateMultiConstruct();
    [[nodiscard]] bool parseUpdateSingleReadCond();
    [[nodiscard]] bool parseUpdateMultiReadCond();
    [[nodiscard]] bool parseUpdateSingleValidCond();
    [[nodiscard]] bool parseUpdateMultiValidCond();
    [[nodiscard]] bool parseCopyConstructToReadCond();
    [[nodiscard]] bool parseCopyConstructToValidCond();
    [[nodiscard]] bool parseUpdateExtraAttrs();
    [[nodiscard]] bool parseUpdateExtraChildren();

    [[nodiscard]] bool parseUpdateSingleCondInternal(const std::string& prop, ParseOptCondImplPtr& cond, bool allowFieldsAccess = false);
    [[nodiscard]] bool parseUpdateMultiCondInternal(const std::string& prop, ParseOptCondImplPtr& cond, bool allowFieldsAccess = false);
    [[nodiscard]] bool parseCopyCondInternal(
        const std::string& copyProp,
        const std::string& fromProp,
        const ParseOptCondImplPtr& fromCond,
        const std::string& toProp,
        ParseOptCondImplPtr& toCond,
        bool allowOverride = true);

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;
    ParsePropsMap m_props;
    ParsePropsMap m_extraAttrs;
    ParseContentsList m_extraChildren;
    const ParseMessageImpl* m_copyFieldsFromMsg = nullptr;
    const ParseBundleFieldImpl* m_copyFieldsFromBundle = nullptr;
    ParseReusableState m_state;
};

using ParseMessageImplPtr = ParseMessageImpl::ParsePtr;

} // namespace parse

} // namespace commsdsl
