//
// Copyright 2018 - 2022 (C). Alex Robenko. All rights reserved.
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

#include <memory>
#include <map>
#include <string>
#include <cstdint>

#include "XmlWrap.h"
#include "Logger.h"
#include "Object.h"
#include "commsdsl/parse/Message.h"
#include "commsdsl/parse/Protocol.h"
#include "FieldImpl.h"
#include "AliasImpl.h"
#include "BundleFieldImpl.h"

namespace commsdsl
{

namespace parse
{

class ProtocolImpl;
class MessageImpl final : public Object
{
    using Base = Object;
public:
    using Ptr = std::unique_ptr<MessageImpl>;
    using PropsMap = XmlWrap::PropsMap;
    using FieldsList = Message::FieldsList;
    using AliasesList = Message::AliasesList;
    using ContentsList = XmlWrap::ContentsList;
    using PlatformsList = Protocol::PlatformsList;
    using Sender = Message::Sender;

    MessageImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    MessageImpl(const MessageImpl&) = delete;
    MessageImpl(MessageImpl&&) = default;
    virtual ~MessageImpl() = default;

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
        return m_id;
    }

    unsigned order() const
    {
        return m_order;
    }

    std::size_t minLength() const;

    std::size_t maxLength() const;

    FieldsList fieldsList() const;
    AliasesList aliasesList() const;

    std::string externalRef() const;

    const PropsMap& extraAttributes() const
    {
        return m_extraAttrs;
    }

    const ContentsList& extraChildren() const
    {
        return m_extraChildren;
    }

    const PlatformsList& platforms() const
    {
        return m_platforms;
    }

    bool isCustomizable() const
    {
        return m_customizable;
    }

    Sender sender() const
    {
        return m_sender;
    }

    OverrideType readOverride() const
    {
        return m_readOverride;
    }

    OverrideType writeOverride() const
    {
        return m_writeOverride;
    }    

    OverrideType refreshOverride() const
    {
        return m_refreshOverride;
    }

    OverrideType lengthOverride() const
    {
        return m_lengthOverride;
    }

    OverrideType validOverride() const
    {
        return m_validOverride;
    }

    OverrideType nameOverride() const
    {
        return m_nameOverride;
    }        

    const std::string& copyOverrideCodeFrom() const
    {
        return m_copyOverrideCodeFrom;
    }

protected:
    virtual ObjKind objKindImpl() const override;

private:
    LogWrapper logError() const;
    LogWrapper logWarning() const;
    LogWrapper logInfo() const;

    static const XmlWrap::NamesList& commonProps();
    static XmlWrap::NamesList allNames();

    bool validateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool validateAndUpdateStringPropValue(const std::string& str, std::string& value, bool mustHave = false, bool allowDeref = false);
    bool validateAndUpdateOverrideTypePropValue(const std::string& propName, OverrideType& value);
    void reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
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
    bool copyFields();
    bool copyAliases();
    bool replaceFields();
    bool updateFields();
    bool updateAliases();
    void cloneFieldsFrom(const MessageImpl& other);
    void cloneFieldsFrom(const BundleFieldImpl& other);
    void cloneAliasesFrom(const MessageImpl& other);
    void cloneAliasesFrom(const BundleFieldImpl& other);
    bool updateReadOverride();
    bool updateWriteOverride();
    bool updateRefreshOverride();
    bool updateLengthOverride();
    bool updateValidOverride();
    bool updateNameOverride();    
    bool updateCopyOverrideCodeFrom();    
    bool updateExtraAttrs();
    bool updateExtraChildren();

    ::xmlNodePtr m_node = nullptr;
    ProtocolImpl& m_protocol;
    PropsMap m_props;
    PropsMap m_extraAttrs;
    ContentsList m_extraChildren;

    std::string m_name;
    std::string m_displayName;
    std::string m_description;
    std::uintmax_t m_id = 0;
    unsigned m_order = 0;
    int m_validateMinLength = -1;
    std::vector<FieldImplPtr> m_fields;
    std::vector<AliasImplPtr> m_aliases;
    PlatformsList m_platforms;
    Sender m_sender = Sender::Both;
    const MessageImpl* m_copyFieldsFromMsg = nullptr;
    const BundleFieldImpl* m_copyFieldsFromBundle = nullptr;
    OverrideType m_readOverride = OverrideType_Any;
    OverrideType m_writeOverride = OverrideType_Any;
    OverrideType m_refreshOverride = OverrideType_Any;
    OverrideType m_lengthOverride = OverrideType_Any;
    OverrideType m_validOverride = OverrideType_Any;
    OverrideType m_nameOverride = OverrideType_Any;    
    std::string m_copyOverrideCodeFrom;
    bool m_customizable = false;
};

using MessageImplPtr = MessageImpl::Ptr;

} // namespace parse

} // namespace commsdsl
