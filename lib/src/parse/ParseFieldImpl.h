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

#include "commsdsl/parse/ParseField.h"
#include "ParseXmlWrap.h"
#include "ParseLogger.h"
#include "ParseObject.h"

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace commsdsl
{

namespace parse
{

class ParseProtocolImpl;
class ParseFieldImpl : public ParseObject
{
    using Base = ParseObject;
public:
    using ParsePtr = std::unique_ptr<ParseFieldImpl>;
    using ParsePropsMap = ParseXmlWrap::ParsePropsMap;
    using ParseContentsList = ParseXmlWrap::ParseContentsList;
    using ParseFieldsList = std::vector<ParsePtr>;
    using ParseKind = ParseField::ParseKind;
    using ParseSemanticType = ParseField::ParseSemanticType;

    enum ParseFieldRefType
    {
        FieldRefType_Invalid,
        FieldRefType_Field,
        FieldRefType_InnerValue,
        FieldRefType_Size,
        FieldRefType_Exists,
        FieldRefType_ValuesLimit
    };

    struct ParseFieldRefInfo
    {
        const ParseFieldImpl* m_field = nullptr;
        std::string m_valueName;
        ParseFieldRefType m_refType = FieldRefType_Invalid;
    };

    using ParseFieldRefInfosList = std::vector<ParseFieldRefInfo>;

    virtual ~ParseFieldImpl() = default;

    static ParsePtr parseCreate(const std::string& kind, ::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParsePtr parseClone() const
    {
        return parseCloneImpl();
    }

    ::xmlNodePtr parseGetNode() const
    {
        return m_node;
    }

    bool parse();
    bool parseVerifySiblings(const ParseFieldsList& fields) const
    {
        return parseVerifySiblingsImpl(fields);
    }

    const ParsePropsMap& parseProps() const
    {
        return m_props;
    }

    const std::string& parseName() const;
    const std::string& parseDisplayName() const;
    const std::string& parseDescription() const;

    ParseKind parseKind() const
    {
        return parseKindImpl();
    }

    const std::string& parseKindStr() const;

    ParseSemanticType parseSemanticType() const
    {
        return m_state.m_semanticType;
    }

    bool parseIsPseudo() const
    {
        return m_state.m_pseudo;
    }

    bool parseIsFixedValue() const
    {
        return m_state.m_fixedValue;
    }

    bool parseIsCustomizable() const
    {
        return m_state.m_customizable;
    }

    bool parseIsFailOnInvalid() const
    {
        return m_state.m_failOnInvalid;
    }

    bool parseIsForceGen() const
    {
        return m_state.m_forceGen;
    }

    ParseOverrideType parseValueOverride() const
    {
        return m_state.m_valueOverride;
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

    std::size_t parseMinLength() const
    {
        return parseMinLengthImpl();
    }

    std::size_t parseMaxLength() const
    {
        return parseMaxLengthImpl();
    }

    std::size_t parseBitLength() const
    {
        return parseBitLengthImpl();
    }

    static ParseXmlWrap::ParseNamesList parseSupportedTypes();

    static bool parseValidateMembersNames(
            const ParseFieldsList& fields,
            ParseLogger& logger);

    bool parseValidateMembersNames(const ParseFieldsList& fields);

    const ParseXmlWrap::ParseNamesList& parseExtraPropsNames() const
    {
        return parseExtraPropsNamesImpl();
    }

    const ParseXmlWrap::ParseNamesList& parseExtraPossiblePropsNames() const
    {
        return parseExtraPossiblePropsNamesImpl();
    }

    const ParseXmlWrap::ParseNamesList& parseExtraChildrenNames() const
    {
        return parseExtraChildrenNamesImpl();
    }

    bool parseIsBitfieldMember() const;
    bool parseIsBundleMember() const;
    bool parseIsMessageMember() const;

    std::string parseExternalRef(bool schemaRef) const;

    bool parseIsComparableToValue(const std::string& val) const;
    bool parseIsComparableToField(const ParseFieldImpl& field) const;

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

    bool parseStrToNumeric(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
    {
        return parseStrToNumericImpl(ref, val, isBigUnsigned);
    }

    bool parseStrToFp(const std::string& ref, double& val) const
    {
        return parseStrToFpImpl(ref, val);
    }

    bool parseStrToBool(const std::string& ref, bool& val) const
    {
        return parseStrToBoolImpl(ref, val);
    }

    bool parseStrToString(const std::string& ref, std::string& val) const
    {
        return parseStrToStringImpl(ref, val);
    }

    bool parseStrToData(const std::string& ref, std::vector<std::uint8_t>& val) const
    {
        return parseStrToDataImpl(ref, val);
    }

    bool parseValidateBitLengthValue(::xmlNodePtr node, std::size_t bitLength) const
    {
        return parseValidateBitLengthValueImpl(node, bitLength);
    }

    bool parseValidateBitLengthValue(std::size_t bitLength) const
    {
        return parseValidateBitLengthValue(m_node, bitLength);
    }

    bool parseVerifySemanticType() const;
    bool parseVerifySemanticType(::xmlNodePtr node, ParseSemanticType type) const;
    bool parseVerifyAliasedMember(const std::string& fieldName);
    bool parseVerifyMinLength() const;

    std::string parseSchemaPos() const;

    const ParseFieldsList& parseMembers() const
    {
        return parseMembersImpl();
    }

    static ParseFieldRefInfo parseProcessSiblingRef(const ParseFieldsList& siblings, const std::string& refStr);

    ParseFieldRefInfo parseProcessInnerRef(const std::string& refStr) const;

    bool parseIsValidInnerRef(const std::string& refStr) const;
    bool parseIsValidRefType(ParseFieldRefType type) const;

protected:
    ParseFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseFieldImpl(const ParseFieldImpl&);

    ParseProtocolImpl& parseProtocol()
    {
        return m_protocol;
    }

    const ParseProtocolImpl& parseProtocol() const
    {
        return m_protocol;
    }

    void parseSetName(const std::string& val)
    {
        m_state.m_name = val;
    }

    void parseSetDisplayName(const std::string& val)
    {
        m_state.m_displayName = val;
    }

    void parseSetSemanticType(ParseSemanticType val)
    {
        m_state.m_semanticType = val;
    }

    ParseLogWrapper parseLogError() const;
    ParseLogWrapper parseLogWarning() const;
    ParseLogWrapper parseLogInfo() const;

    virtual ParseObjKind parseObjKindImpl() const override final;
    virtual ParseKind parseKindImpl() const = 0;
    virtual ParsePtr parseCloneImpl() const = 0;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraPropsNamesImpl() const;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraPossiblePropsNamesImpl() const;
    virtual const ParseXmlWrap::ParseNamesList& parseExtraChildrenNamesImpl() const;
    virtual bool parseReuseImpl(const ParseFieldImpl& other);
    virtual bool parseImpl();
    virtual bool parseReplaceMembersImpl(ParseFieldsList& members);
    virtual bool parseVerifySiblingsImpl(const ParseFieldsList& fields) const;
    virtual std::size_t parseMinLengthImpl() const = 0;
    virtual std::size_t parseMaxLengthImpl() const;
    virtual std::size_t parseBitLengthImpl() const;
    virtual bool parseIsComparableToValueImpl(const std::string& val) const;
    virtual bool parseIsComparableToFieldImpl(const ParseFieldImpl& field) const;
    virtual bool parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const;
    virtual bool parseStrToFpImpl(const std::string& ref, double& val) const;
    virtual bool parseStrToBoolImpl(const std::string& ref, bool& val) const;
    virtual bool parseStrToStringImpl(const std::string& ref, std::string& val) const;
    virtual bool parseStrToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const;
    virtual bool parseValidateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const;
    virtual bool parseVerifySemanticTypeImpl(::xmlNodePtr node, ParseSemanticType type) const;
    virtual bool parseVerifyAliasedMemberImpl(const std::string& fieldName) const;
    virtual const ParseXmlWrap::ParseNamesList& parseSupportedMemberTypesImpl() const;
    virtual const ParseFieldsList& parseMembersImpl() const;
    virtual ParseFieldRefInfo parseProcessInnerRefImpl(const std::string& refStr) const;
    virtual bool parseIsValidRefTypeImpl(ParseFieldRefType type) const;

    bool parseValidateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool parseValidateNoPropInstance(const std::string& str);
    bool parseValidateAndUpdateStringPropValue(
            const std::string& str,
            std::string &value,
            bool mustHave = false,
            bool allowDeref = false);
    void parseReportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    void parseCheckAndReportDeprecatedPropertyValue(const std::string& propName);
    bool parseValidateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave = false);
    bool parseValidateAndUpdateOverrideTypePropValue(const std::string& propName, ParseOverrideType& value);

    static const ParseXmlWrap::ParseNamesList& parseCommonProps();
    static const ParseXmlWrap::ParseNamesList& parseCommonChildren();
    const ParseFieldImpl* parseFindSibling(const ParseFieldsList& fields, const std::string& sibName) const;
    static ParseKind parseGetNonRefFieldKind(const ParseFieldImpl& field);
    bool parseCheckDetachedPrefixAllowed() const;

    using ParseStrToValueFieldConvertFunc = std::function<bool (const ParseFieldImpl& f, const std::string& ref)>;
    bool parseStrToValueOnFields(
        const std::string& ref,
        const ParseFieldsList& fields,
        ParseStrToValueFieldConvertFunc&& func) const;

    bool parseStrToNumericOnFields(
        const std::string& ref,
        const ParseFieldsList& fields,
        std::intmax_t& val,
        bool& isBigUnsigned) const;

    bool parseStrToFpOnFields(
        const std::string& ref,
        const ParseFieldsList& fields,
        double& val) const;

    bool parseStrToBoolOnFields(
        const std::string& ref,
        const ParseFieldsList& fields,
        bool& val) const;

    bool parseStrToStringOnFields(
        const std::string& ref,
        const ParseFieldsList& fields,
        std::string& val) const;

    bool parseStrToDataOnFields(
        const std::string& ref,
        const ParseFieldsList& fields,
        std::vector<std::uint8_t>& val) const;

private:
    using ParseCreateFunc = std::function<ParsePtr (::xmlNodePtr n, ParseProtocolImpl& p)>;
    using ParseCreateMap = std::map<std::string, ParseCreateFunc>;

    struct ParseReusableState
    {
        std::string m_name;
        std::string m_displayName;
        std::string m_description;
        ParsePropsMap m_extraAttrs;
        ParseContentsList m_extraChildren;
        ParseSemanticType m_semanticType = ParseSemanticType::None;
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

    bool parseCheckReuse();
    bool parseCheckReplace();
    bool parseUpdateName();
    bool parseUpdateDescription();
    bool parseUpdateDisplayName();
    bool parseUpdateVersions();
    bool parseUpdateSemanticType();
    bool parseUpdatePseudo();
    bool parseUpdateFixedValue();
    bool parseUpdateDisplayReadOnly();
    bool parseUpdateDisplayHidden();
    bool parseUpdateCustomizable();
    bool parseUpdateFailOnInvalid();
    bool parseUpdateForceGen();
    bool parseUpdateValueOverride();
    bool parseUpdateReadOverride();
    bool parseUpdateWriteOverride();
    bool parseUpdateRefreshOverride();
    bool parseUpdateLengthOverride();
    bool parseUpdateValidOverride();
    bool parseUpdateNameOverride();
    bool parseUpdateCopyOverrideCodeFrom();
    bool parseUpdateValidateMinLength();
    bool parseUpdateExtraAttrs(const ParseXmlWrap::ParseNamesList& names);
    bool parseUpdateExtraChildren(const ParseXmlWrap::ParseNamesList& names);

    bool parseVerifyName() const;

    static const ParseCreateMap& parseCreateMap();

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;
    ParsePropsMap m_props;
    ParseReusableState m_state;
};

using ParseFieldImplPtr = ParseFieldImpl::ParsePtr;

} // namespace parse

} // namespace commsdsl
