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
#include <functional>
#include <string>

#include "commsdsl/parse/ParseField.h"
#include "ParseXmlWrap.h"
#include "ParseLogger.h"
#include "ParseObject.h"

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
class ParseFieldImpl : public ParseObject
{
    using Base = ParseObject;
public:
    using Ptr = std::unique_ptr<ParseFieldImpl>;
    using PropsMap = ParseXmlWrap::PropsMap;
    using ContentsList = ParseXmlWrap::ContentsList;
    using FieldsList = std::vector<Ptr>;
    using Kind = ParseField::Kind;
    using SemanticType = ParseField::SemanticType;

    enum FieldRefType
    {
        FieldRefType_Invalid,
        FieldRefType_Field,
        FieldRefType_InnerValue,
        FieldRefType_Size,
        FieldRefType_Exists,
        FieldRefType_ValuesLimit
    };

    struct FieldRefInfo
    {
        const ParseFieldImpl* m_field = nullptr;
        std::string m_valueName;
        FieldRefType m_refType = FieldRefType_Invalid;
    };

    using FieldRefInfosList = std::vector<FieldRefInfo>;

    virtual ~ParseFieldImpl() = default;

    static Ptr create(const std::string& kind, ::xmlNodePtr node, ParseProtocolImpl& protocol);
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

    bool isFixedValue() const
    {
        return m_state.m_fixedValue;
    }    

    bool isCustomizable() const
    {
        return m_state.m_customizable;
    }    

    bool isFailOnInvalid() const
    {
        return m_state.m_failOnInvalid;
    }    

    bool isForceGen() const
    {
        return m_state.m_forceGen;
    }

    ParseOverrideType valueOverride() const
    {
        return m_state.m_valueOverride;
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

    static ParseXmlWrap::NamesList supportedTypes();

    static bool validateMembersNames(
            const FieldsList& fields,
            ParseLogger& logger);

    bool validateMembersNames(const FieldsList& fields);

    const ParseXmlWrap::NamesList& extraPropsNames() const
    {
        return extraPropsNamesImpl();
    }

    const ParseXmlWrap::NamesList& extraPossiblePropsNames() const
    {
        return extraPossiblePropsNamesImpl();
    }

    const ParseXmlWrap::NamesList& extraChildrenNames() const
    {
        return extraChildrenNamesImpl();
    }

    bool isBitfieldMember() const;
    bool isBundleMember() const;
    bool isMessageMember() const;

    std::string externalRef(bool schemaRef) const;

    bool isComparableToValue(const std::string& val) const;
    bool isComparableToField(const ParseFieldImpl& field) const;

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
    bool verifyMinLength() const;

    std::string schemaPos() const;

    const FieldsList& members() const
    {
        return membersImpl();
    }

    static FieldRefInfo processSiblingRef(const FieldsList& siblings, const std::string& refStr);

    FieldRefInfo processInnerRef(const std::string& refStr) const;

    bool isValidInnerRef(const std::string& refStr) const;
    bool isValidRefType(FieldRefType type) const;

protected:
    ParseFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseFieldImpl(const ParseFieldImpl&);

    ParseProtocolImpl& protocol()
    {
        return m_protocol;
    }

    const ParseProtocolImpl& protocol() const
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
    virtual const ParseXmlWrap::NamesList& extraPropsNamesImpl() const;
    virtual const ParseXmlWrap::NamesList& extraPossiblePropsNamesImpl() const;
    virtual const ParseXmlWrap::NamesList& extraChildrenNamesImpl() const;
    virtual bool reuseImpl(const ParseFieldImpl& other);
    virtual bool parseImpl();
    virtual bool replaceMembersImpl(FieldsList& members);
    virtual bool verifySiblingsImpl(const FieldsList& fields) const;
    virtual std::size_t minLengthImpl() const = 0;
    virtual std::size_t maxLengthImpl() const;
    virtual std::size_t bitLengthImpl() const;
    virtual bool isComparableToValueImpl(const std::string& val) const;
    virtual bool isComparableToFieldImpl(const ParseFieldImpl& field) const;
    virtual bool strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const;
    virtual bool strToFpImpl(const std::string& ref, double& val) const;
    virtual bool strToBoolImpl(const std::string& ref, bool& val) const;
    virtual bool strToStringImpl(const std::string& ref, std::string& val) const;
    virtual bool strToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const;
    virtual bool validateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const;
    virtual bool verifySemanticTypeImpl(::xmlNodePtr node, SemanticType type) const;
    virtual bool verifyAliasedMemberImpl(const std::string& fieldName) const;
    virtual const ParseXmlWrap::NamesList& supportedMemberTypesImpl() const;
    virtual const FieldsList& membersImpl() const;
    virtual FieldRefInfo processInnerRefImpl(const std::string& refStr) const;
    virtual bool isValidRefTypeImpl(FieldRefType type) const;

    bool validateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool validateNoPropInstance(const std::string& str);
    bool validateAndUpdateStringPropValue(
            const std::string& str,
            std::string &value,
            bool mustHave = false,
            bool allowDeref = false);
    void reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    void checkAndReportDeprecatedPropertyValue(const std::string& propName);
    bool validateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave = false);
    bool validateAndUpdateOverrideTypePropValue(const std::string& propName, ParseOverrideType& value);

    static const ParseXmlWrap::NamesList& commonProps();
    static const ParseXmlWrap::NamesList& commonChildren();
    const ParseFieldImpl* findSibling(const FieldsList& fields, const std::string& sibName) const;
    static Kind getNonRefFieldKind(const ParseFieldImpl& field);
    bool checkDetachedPrefixAllowed() const;

    using StrToValueFieldConvertFunc = std::function<bool (const ParseFieldImpl& f, const std::string& ref)>;
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
    using CreateFunc = std::function<Ptr (::xmlNodePtr n, ParseProtocolImpl& p)>;
    using CreateMap = std::map<std::string, CreateFunc>;

    struct ReusableState
    {
        std::string m_name;
        std::string m_displayName;
        std::string m_description;
        PropsMap m_extraAttrs;
        ContentsList m_extraChildren;
        SemanticType m_semanticType = SemanticType::None;
        ParseOverrideType m_valueOverride = ParseOverrideType_Any;
        ParseOverrideType m_readOverride = ParseOverrideType_Any;
        ParseOverrideType m_writeOverride = ParseOverrideType_Any;
        ParseOverrideType m_refreshOverride = ParseOverrideType_Any;
        ParseOverrideType m_lengthOverride = ParseOverrideType_Any;
        ParseOverrideType m_validOverride = ParseOverrideType_Any;
        ParseOverrideType m_nameOverride = ParseOverrideType_Any;
        std::string m_copyCodeFrom;
        int m_validateMinLength = -1;
        bool m_pseudo = false;
        bool m_fixedValue = false;
        bool m_customizable = false;
        bool m_failOnInvalid = false;
        bool m_forceGen = false;
    };

    bool checkReuse();
    bool checkReplace();
    bool updateName();
    bool updateDescription();
    bool updateDisplayName();
    bool updateVersions();
    bool updateSemanticType();
    bool updatePseudo();
    bool updateFixedValue();
    bool updateDisplayReadOnly();
    bool updateDisplayHidden();
    bool updateCustomizable();
    bool updateFailOnInvalid();
    bool updateForceGen();
    bool updateValueOverride();
    bool updateReadOverride();
    bool updateWriteOverride();
    bool updateRefreshOverride();
    bool updateLengthOverride();
    bool updateValidOverride();
    bool updateNameOverride();
    bool updateCopyOverrideCodeFrom();
    bool updateValidateMinLength();
    bool updateExtraAttrs(const ParseXmlWrap::NamesList& names);
    bool updateExtraChildren(const ParseXmlWrap::NamesList& names);

    bool verifyName() const;

    static const CreateMap& createMap();

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;
    PropsMap m_props;
    ReusableState m_state;
};

using ParseFieldImplPtr = ParseFieldImpl::Ptr;

} // namespace parse

} // namespace commsdsl
