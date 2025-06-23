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

#include <cstdint>
#include <memory>
#include <map>
#include <string>
#include <tuple>
#include <utility>

#include "commsdsl/parse/ParseMessage.h"

#include "ParseAliasImpl.h"
#include "ParseBundleFieldImpl.h"
#include "ParseFieldImpl.h"
#include "ParseLogger.h"
#include "ParseOptCondImpl.h"
#include "ParseObject.h"
#include "ParseXmlWrap.h"

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

    ::xmlNodePtr getNode() const
    {
        return m_node;
    }

    bool parse();

    const PropsMap& props() const
    {
        return m_props;
    }

    const std::string& name() const;
    const std::string& displayName() const;
    const std::string& description() const;

    std::uintmax_t id() const
    {
        return m_state.m_id;
    }

    unsigned order() const
    {
        return m_state.m_order;
    }

    std::size_t minLength() const;

    std::size_t maxLength() const;

    FieldsList fieldsList() const;
    AliasesList aliasesList() const;

    std::string externalRef(bool schemaRef) const;

    const PropsMap& parseExtraAttributes() const
    {
        return m_extraAttrs;
    }

    const ContentsList& extraChildren() const
    {
        return m_extraChildren;
    }

    const PlatformsList& platforms() const
    {
        return m_state.m_platforms;
    }

    bool isCustomizable() const
    {
        return m_state.m_customizable;
    }

    bool isFailOnInvalid() const
    {
        return m_state.m_failOnInvalid;
    }       

    Sender sender() const
    {
        return m_state.m_sender;
    }

    ParseOverrideType readOverride() const
    {
        return m_state.m_readOverride;
    }

    ParseOverrideType writeOverride() const
    {
        return m_state.m_writeOverride;
    }    

    ParseOverrideType refreshOverride() const
    {
        return m_state.m_refreshOverride;
    }

    ParseOverrideType lengthOverride() const
    {
        return m_state.m_lengthOverride;
    }

    ParseOverrideType validOverride() const
    {
        return m_state.m_validOverride;
    }

    ParseOverrideType nameOverride() const
    {
        return m_state.m_nameOverride;
    }        

    const std::string& copyCodeFrom() const
    {
        return m_state.m_copyCodeFrom;
    }

    ParseOptCond construct() const
    {
        return ParseOptCond(m_state.m_construct.get());
    }

    ParseOptCond readCond() const
    {
        return ParseOptCond(m_state.m_readCond.get());
    }  

    ParseOptCond validCond() const
    {
        return ParseOptCond(m_state.m_validCond.get());
    }          

protected:
    virtual ObjKind objKindImpl() const override;

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
                m_fields.push_back(f->clone());
            }

            m_aliases.clear();
            m_aliases.reserve(other.m_aliases.size());
            for (auto& a : other.m_aliases) {
                m_aliases.push_back(a->clone());
            }            

            if (other.m_construct) {
                m_construct = other.m_construct->clone();
            }

            if (other.m_readCond) {
                m_readCond = other.m_readCond->clone();
            }    

            if (other.m_validCond) {
                m_validCond = other.m_validCond->clone();
            }    

            return *this;          
        }
    };

    LogWrapper logError() const;
    LogWrapper logWarning() const;
    LogWrapper logInfo() const;

    static const ParseXmlWrap::NamesList& commonProps();
    static const ParseXmlWrap::NamesList& extraProps();
    static const ParseXmlWrap::NamesList& allProps();
    
    static ParseXmlWrap::NamesList allNames();

    bool validateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool validateAndUpdateStringPropValue(const std::string& str, std::string& value, bool mustHave = false, bool allowDeref = false);
    bool validateAndUpdateOverrideTypePropValue(const std::string& propName, ParseOverrideType& value);
    bool validateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave = false);
    void reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    bool checkReuse();
    bool updateName();
    bool updateDescription();
    bool updateDisplayName();
    bool updateId();
    bool updateOrder();
    bool updateVersions();
    bool updatePlatforms();
    bool updateCustomizable();
    bool updateSender();
    bool updateValidateMinLength();
    bool updateFailOnInvalid();
    bool copyFields();
    bool copyAliases();
    bool replaceFields();
    bool updateFields();
    bool updateAliases();
    void cloneFieldsFrom(const ParseMessageImpl& other);
    void cloneFieldsFrom(const ParseBundleFieldImpl& other);
    void cloneAliasesFrom(const ParseMessageImpl& other);
    void cloneAliasesFrom(const ParseBundleFieldImpl& other);
    bool updateReadOverride();
    bool updateWriteOverride();
    bool updateRefreshOverride();
    bool updateLengthOverride();
    bool updateValidOverride();
    bool updateNameOverride();    
    bool updateCopyOverrideCodeFrom();    
    bool copyConstruct();
    bool copyReadCond();
    bool copyValidCond();
    bool updateSingleConstruct();
    bool updateMultiConstruct();
    bool updateSingleReadCond();
    bool updateMultiReadCond();
    bool updateSingleValidCond();
    bool updateMultiValidCond();    
    bool copyConstructToReadCond();
    bool copyConstructToValidCond();
    bool updateExtraAttrs();
    bool updateExtraChildren();

    bool updateSingleCondInternal(const std::string& prop, ParseOptCondImplPtr& cond, bool allowFieldsAccess = false);
    bool updateMultiCondInternal(const std::string& prop, ParseOptCondImplPtr& cond, bool allowFieldsAccess = false);
    bool copyCondInternal(
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
