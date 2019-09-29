//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
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
#include <functional>
#include <string>

#include "commsdsl/Field.h"
#include "XmlWrap.h"
#include "Logger.h"
#include "Object.h"

namespace commsdsl
{

class ProtocolImpl;
class FieldImpl : public Object
{
    using Base = Object;
public:
    using Ptr = std::unique_ptr<FieldImpl>;
    using PropsMap = XmlWrap::PropsMap;
    using ContentsList = XmlWrap::ContentsList;
    using FieldsList = std::vector<Ptr>;
    using Kind = Field::Kind;
    using SemanticType = Field::SemanticType;

    virtual ~FieldImpl() = default;

    static Ptr create(const std::string& kind, ::xmlNodePtr node, ProtocolImpl& protocol);
    Ptr clone() const
    {
        return cloneImpl();
    }

    ::xmlNodePtr getNode() const
    {
        return m_node;
    }

    bool parse();
    bool verifySiblings(const FieldsList& fields) const
    {
        return verifySiblingsImpl(fields);
    }

    const PropsMap& props() const
    {
        return m_props;
    }

    const std::string& name() const;
    const std::string& displayName() const;
    const std::string& description() const;

    Kind kind() const
    {
        return kindImpl();
    }

    const std::string& kindStr() const;

    SemanticType semanticType() const
    {
        return m_state.m_semanticType;
    }

    bool isPseudo() const
    {
        return m_state.m_pseudo;
    }

    bool isDisplayReadOnly() const
    {
        return m_state.m_displayReadOnly;
    }    

    bool isDisplayHidden() const
    {
        return m_state.m_displayHidden;
    }    

    bool isCustomizable() const
    {
        return m_state.m_customizable;
    }    

    bool isFailOnInvalid() const
    {
        return m_state.m_failOnInvalid;
    }    

    std::size_t minLength() const
    {
        return minLengthImpl();
    }

    std::size_t maxLength() const
    {
        return maxLengthImpl();
    }

    std::size_t bitLength() const
    {
        return bitLengthImpl();
    }

    static XmlWrap::NamesList supportedTypes();

    static bool validateMembersNames(
            const FieldsList& fields,
            Logger& logger);

    bool validateMembersNames(const FieldsList& fields);

    const XmlWrap::NamesList& extraPropsNames() const
    {
        return extraPropsNamesImpl();
    }

    const XmlWrap::NamesList& extraPossiblePropsNames() const
    {
        return extraPossiblePropsNamesImpl();
    }

    const XmlWrap::NamesList& extraChildrenNames() const
    {
        return extraChildrenNamesImpl();
    }

    bool isBitfieldMember() const;
    bool isBundleMember() const;
    bool isMessageMember() const;

    std::string externalRef() const;

    bool isBitCheckable(const std::string& val) const
    {
        return isBitCheckableImpl(val);
    }

    bool isComparableToValue(const std::string& val) const
    {
        return isComparableToValueImpl(val);
    }

    bool isComparableToField(const FieldImpl& field) const;

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

    bool strToNumeric(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
    {
        return strToNumericImpl(ref, val, isBigUnsigned);
    }

    bool strToFp(const std::string& ref, double& val) const
    {
        return strToFpImpl(ref, val);
    }

    bool strToBool(const std::string& ref, bool& val) const
    {
        return strToBoolImpl(ref, val);
    }

    bool strToString(const std::string& ref, std::string& val) const
    {
        return strToStringImpl(ref, val);
    }

    bool strToData(const std::string& ref, std::vector<std::uint8_t>& val) const
    {
        return strToDataImpl(ref, val);
    }

    bool validateBitLengthValue(::xmlNodePtr node, std::size_t bitLength) const
    {
        return validateBitLengthValueImpl(node, bitLength);
    }

    bool validateBitLengthValue(std::size_t bitLength) const
    {
        return validateBitLengthValue(m_node, bitLength);
    }

    bool verifySemanticType() const;
    bool verifySemanticType(::xmlNodePtr node, SemanticType type) const;
    bool verifyAliasedMember(const std::string& fieldName);

    std::string schemaPos() const;

protected:
    FieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    FieldImpl(const FieldImpl&);

    ProtocolImpl& protocol()
    {
        return m_protocol;
    }

    const ProtocolImpl& protocol() const
    {
        return m_protocol;
    }

    void setName(const std::string& val)
    {
        m_state.m_name = val;
    }

    void setDisplayName(const std::string& val)
    {
        m_state.m_displayName = val;
    }

    void setSemanticType(SemanticType val)
    {
        m_state.m_semanticType = val;
    }

    LogWrapper logError() const;
    LogWrapper logWarning() const;
    LogWrapper logInfo() const;

    virtual ObjKind objKindImpl() const override final;
    virtual Kind kindImpl() const = 0;
    virtual Ptr cloneImpl() const = 0;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const;
    virtual const XmlWrap::NamesList& extraPossiblePropsNamesImpl() const;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const;
    virtual bool reuseImpl(const FieldImpl& other);
    virtual bool parseImpl();
    virtual bool verifySiblingsImpl(const FieldsList& fields) const;
    virtual std::size_t minLengthImpl() const = 0;
    virtual std::size_t maxLengthImpl() const;
    virtual std::size_t bitLengthImpl() const;
    virtual bool isBitCheckableImpl(const std::string& val) const;
    virtual bool isComparableToValueImpl(const std::string& val) const;
    virtual bool isComparableToFieldImpl(const FieldImpl& field) const;
    virtual bool strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const;
    virtual bool strToFpImpl(const std::string& ref, double& val) const;
    virtual bool strToBoolImpl(const std::string& ref, bool& val) const;
    virtual bool strToStringImpl(const std::string& ref, std::string& val) const;
    virtual bool strToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const;
    virtual bool validateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const;
    virtual bool verifySemanticTypeImpl(::xmlNodePtr node, SemanticType type) const;
    virtual bool verifyAliasedMemberImpl(const std::string& fieldName) const;

    bool validateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool validateNoPropInstance(const std::string& str);
    bool validateAndUpdateStringPropValue(
            const std::string& str,
            std::string &value,
            bool mustHave = false,
            bool allowDeref = false);
    void reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    bool validateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave = false);

    static const XmlWrap::NamesList& commonProps();
    static const XmlWrap::NamesList& commonChildren();
    const FieldImpl* findSibling(const FieldsList& fields, const std::string& sibName) const;
    static Kind getNonRefFieldKind(const FieldImpl& field);
    bool checkDetachedPrefixAllowed() const;

    using StrToValueFieldConvertFunc = std::function<bool (const FieldImpl& f, const std::string& ref)>;
    bool strToValueOnFields(
        const std::string& ref,
        const FieldsList& fields,
        StrToValueFieldConvertFunc&& func) const;

    bool strToNumericOnFields(
        const std::string& ref,
        const FieldsList& fields,
        std::intmax_t& val,
        bool& isBigUnsigned) const;

    bool strToFpOnFields(
        const std::string& ref,
        const FieldsList& fields,
        double& val) const;

    bool strToBoolOnFields(
        const std::string& ref,
        const FieldsList& fields,
        bool& val) const;

    bool strToStringOnFields(
        const std::string& ref,
        const FieldsList& fields,
        std::string& val) const;

    bool strToDataOnFields(
        const std::string& ref,
        const FieldsList& fields,
        std::vector<std::uint8_t>& val) const;

private:
    using CreateFunc = std::function<Ptr (::xmlNodePtr n, ProtocolImpl& p)>;
    using CreateMap = std::map<std::string, CreateFunc>;

    struct ReusableState
    {
        std::string m_name;
        std::string m_displayName;
        std::string m_description;
        PropsMap m_extraAttrs;
        ContentsList m_extraChildren;
        SemanticType m_semanticType = SemanticType::None;
        bool m_pseudo = false;
        bool m_displayReadOnly = false;
        bool m_displayHidden = false;
        bool m_customizable = false;
        bool m_failOnInvalid = false;
    };

    bool checkReuse();
    bool updateName();
    bool updateDescription();
    bool updateDisplayName();
    bool updateVersions();
    bool updateSemanticType();
    bool updatePseudo();
    bool updateDisplayReadOnly();
    bool updateDisplayHidden();
    bool updateCustomizable();
    bool updateFailOnInvalid();
    bool updateExtraAttrs(const XmlWrap::NamesList& names);
    bool updateExtraChildren(const XmlWrap::NamesList& names);

    bool verifyName() const;

    static const CreateMap& createMap();

    ::xmlNodePtr m_node = nullptr;
    ProtocolImpl& m_protocol;
    PropsMap m_props;
    ReusableState m_state;
};

using FieldImplPtr = FieldImpl::Ptr;

} // namespace commsdsl
