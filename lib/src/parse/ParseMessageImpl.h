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
    using Ptr = std::unique_ptr<ParseMessageImpl>;
    using PropsMap = ParseXmlWrap::PropsMap;
    using FieldsList = ParseMessage::FieldsList;
    using AliasesList = ParseMessage::AliasesList;
    using ContentsList = ParseXmlWrap::ContentsList;
    using PlatformsList = ParseMessage::PlatformsList;
    using Sender = ParseMessage::Sender;

    ParseMessageImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseMessageImpl(const ParseMessageImpl&) = delete;
    ParseMessageImpl(ParseMessageImpl&&) = default;
    virtual ~ParseMessageImpl() = default;

    ::xmlNodePtr parseGetNode() const
    {
        return m_node;
    }

    bool parse();

    const PropsMap& parseProps() const
    {
        return m_props;
    }

    const std::string& parseName() const;
    const std::string& parseDisplayName() const;
    const std::string& parseDescription() const;

    std::uintmax_t parseId() const
    {
        return m_state.m_id;
    }

    unsigned parseOrder() const
    {
        return m_state.m_order;
    }

    std::size_t parseMinLength() const;

    std::size_t parseMaxLength() const;

    FieldsList parseFieldsList() const;
    AliasesList parseAliasesList() const;

    std::string parseExternalRef(bool schemaRef) const;

    const PropsMap& parseExtraAttributes() const
    {
        return m_extraAttrs;
    }

    const ContentsList& parseExtraChildren() const
    {
        return m_extraChildren;
    }

    const PlatformsList& parsePlatforms() const
    {
        return m_state.m_platforms;
    }

    bool parseIsCustomizable() const
    {
        return m_state.m_customizable;
    }

    bool parseIsFailOnInvalid() const
    {
        return m_state.m_failOnInvalid;
    }       

    Sender parseSender() const
    {
        return m_state.m_sender;
    }

    ParseOverrideType parseReadOverride() const
    {
        return m_state.m_readOverride;
    }

    ParseOverrideType parseWriteOverride() const
    {
        return m_state.m_writeOverride;
    }    

    ParseOverrideType parseRefreshOverride() const
    {
        return m_state.m_refreshOverride;
    }

    ParseOverrideType parseLengthOverride() const
    {
        return m_state.m_lengthOverride;
    }

    ParseOverrideType parseValidOverride() const
    {
        return m_state.m_validOverride;
    }

    ParseOverrideType parseNameOverride() const
    {
        return m_state.m_nameOverride;
    }        

    const std::string& parseCopyCodeFrom() const
    {
        return m_state.m_copyCodeFrom;
    }

    ParseOptCond parseConstruct() const
    {
        return ParseOptCond(m_state.m_construct.get());
    }

    ParseOptCond parseReadCond() const
    {
        return ParseOptCond(m_state.m_readCond.get());
    }  

    ParseOptCond parseValidCond() const
    {
        return ParseOptCond(m_state.m_validCond.get());
    }          

protected:
    virtual ObjKind parseObjKindImpl() const override;

private:
    struct ReusableState
    {
        std::string m_name;
        std::string m_displayName;
        std::string m_description;
        std::uintmax_t m_id = 0;
        unsigned m_order = 0;
        int m_validateMinLength = -1;
        std::vector<ParseFieldImplPtr> m_fields;
        std::vector<ParseAliasImplPtr> m_aliases;
        PlatformsList m_platforms;
        Sender m_sender = Sender::Both;
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

        ReusableState() = default;
        ReusableState(ReusableState&&) = default;

        auto basicForwardAsTuple()
        {
            return 
                std::forward_as_tuple(
                    m_name,
                    m_displayName,
                    m_description,
                    m_id,
                    m_order,
                    m_validateMinLength,
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
                    m_failOnInvalid
                );
        }

        auto basicForwardAsTuple() const
        {
            return const_cast<ReusableState*>(this)->basicForwardAsTuple();
        }

        ReusableState& operator=(const ReusableState& other)
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

    LogWrapper parseLogError() const;
    LogWrapper parseLogWarning() const;
    LogWrapper parseLogInfo() const;

    static const ParseXmlWrap::NamesList& parseCommonProps();
    static const ParseXmlWrap::NamesList& parseExtraProps();
    static const ParseXmlWrap::NamesList& parseAllProps();
    
    static ParseXmlWrap::NamesList parseAllNames();

    bool parseValidateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool parseValidateAndUpdateStringPropValue(const std::string& str, std::string& value, bool mustHave = false, bool allowDeref = false);
    bool parseValidateAndUpdateOverrideTypePropValue(const std::string& propName, ParseOverrideType& value);
    bool parseValidateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave = false);
    void parseReportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    bool parseCheckReuse();
    bool parseUpdateName();
    bool parseUpdateDescription();
    bool parseUpdateDisplayName();
    bool parseUpdateId();
    bool parseUpdateOrder();
    bool parseUpdateVersions();
    bool parseUpdatePlatforms();
    bool parseUpdateCustomizable();
    bool parseUpdateSender();
    bool parseUpdateValidateMinLength();
    bool parseUpdateFailOnInvalid();
    bool parseCopyFields();
    bool parseCopyAliases();
    bool parseReplaceFields();
    bool parseUpdateFields();
    bool parseUpdateAliases();
    void parseCloneFieldsFrom(const ParseMessageImpl& other);
    void parseCloneFieldsFrom(const ParseBundleFieldImpl& other);
    void parseCloneAliasesFrom(const ParseMessageImpl& other);
    void parseCloneAliasesFrom(const ParseBundleFieldImpl& other);
    bool parseUpdateReadOverride();
    bool parseUpdateWriteOverride();
    bool parseUpdateRefreshOverride();
    bool parseUpdateLengthOverride();
    bool parseUpdateValidOverride();
    bool parseUpdateNameOverride();    
    bool parseUpdateCopyOverrideCodeFrom();    
    bool parseCopyConstruct();
    bool parseCopyReadCond();
    bool parseCopyValidCond();
    bool parseUpdateSingleConstruct();
    bool parseUpdateMultiConstruct();
    bool parseUpdateSingleReadCond();
    bool parseUpdateMultiReadCond();
    bool parseUpdateSingleValidCond();
    bool parseUpdateMultiValidCond();    
    bool parseCopyConstructToReadCond();
    bool parseCopyConstructToValidCond();
    bool parseUpdateExtraAttrs();
    bool parseUpdateExtraChildren();

    bool parseUpdateSingleCondInternal(const std::string& prop, ParseOptCondImplPtr& cond, bool allowFieldsAccess = false);
    bool parseUpdateMultiCondInternal(const std::string& prop, ParseOptCondImplPtr& cond, bool allowFieldsAccess = false);
    bool parseCopyCondInternal(
        const std::string& copyProp,
        const std::string& fromProp, 
        const ParseOptCondImplPtr& fromCond, 
        const std::string& toProp, 
        ParseOptCondImplPtr& toCond,
        bool allowOverride = true);

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;
    PropsMap m_props;
    PropsMap m_extraAttrs;
    ContentsList m_extraChildren;
    const ParseMessageImpl* m_copyFieldsFromMsg = nullptr;
    const ParseBundleFieldImpl* m_copyFieldsFromBundle = nullptr;    
    ReusableState m_state;
};

using ParseMessageImplPtr = ParseMessageImpl::Ptr;

} // namespace parse

} // namespace commsdsl
