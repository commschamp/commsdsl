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

    [[nodiscard]] static ParsePtr parseCreate(const std::string& kind, ::xmlNodePtr node, ParseProtocolImpl& protocol);
    [[nodiscard]] ParsePtr parseClone() const
    {
        return parseCloneImpl();
    }

    [[nodiscard]] ::xmlNodePtr parseGetNode() const
    {
        return m_node;
    }

    [[nodiscard]] bool parse();
    [[nodiscard]] bool parseVerifySiblings(const ParseFieldsList& fields) const
    {
        return parseVerifySiblingsImpl(fields);
    }

    [[nodiscard]] const ParsePropsMap& parseProps() const
    {
        return m_props;
    }

    [[nodiscard]] const std::string& parseName() const;
    [[nodiscard]] const std::string& parseDisplayName() const;
    [[nodiscard]] const std::string& parseDescription() const;

    [[nodiscard]] ParseKind parseKind() const
    {
        return parseKindImpl();
    }

    [[nodiscard]] const std::string& parseKindStr() const;

    [[nodiscard]] ParseSemanticType parseSemanticType() const
    {
        return m_state.m_semanticType;
    }

    [[nodiscard]] bool parseIsPseudo() const
    {
        return m_state.m_pseudo;
    }

    [[nodiscard]] bool parseIsFixedValue() const
    {
        return m_state.m_fixedValue;
    }

    [[nodiscard]] bool parseIsCustomizable() const
    {
        return m_state.m_customizable;
    }

    [[nodiscard]] bool parseIsFailOnInvalid() const
    {
        return m_state.m_failOnInvalid;
    }

    [[nodiscard]] bool parseIsForceGen() const
    {
        return m_state.m_forceGen;
    }

    [[nodiscard]] ParseOverrideType parseValueOverride() const
    {
        return m_state.m_valueOverride;
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

    [[nodiscard]] std::size_t parseMinLength() const
    {
        return parseMinLengthImpl();
    }

    [[nodiscard]] std::size_t parseMaxLength() const
    {
        return parseMaxLengthImpl();
    }

    [[nodiscard]] std::size_t parseBitLength() const
    {
        return parseBitLengthImpl();
    }

    [[nodiscard]] static ParseXmlWrap::ParseNamesList parseSupportedTypes();

    [[nodiscard]] static bool parseValidateMembersNames(
            const ParseFieldsList& fields,
            ParseLogger& logger);

    [[nodiscard]] bool parseValidateMembersNames(const ParseFieldsList& fields);

    [[nodiscard]] const ParseXmlWrap::ParseNamesList& parseExtraPropsNames() const
    {
        return parseExtraPropsNamesImpl();
    }

    [[nodiscard]] const ParseXmlWrap::ParseNamesList& parseExtraPossiblePropsNames() const
    {
        return parseExtraPossiblePropsNamesImpl();
    }

    [[nodiscard]] const ParseXmlWrap::ParseNamesList& parseExtraChildrenNames() const
    {
        return parseExtraChildrenNamesImpl();
    }

    [[nodiscard]] bool parseIsBitfieldMember() const;
    [[nodiscard]] bool parseIsBundleMember() const;
    [[nodiscard]] bool parseIsMessageMember() const;

    [[nodiscard]] std::string parseExternalRef(bool schemaRef) const;
    [[nodiscard]] std::string parseInnerRef(bool schemaRef) const;

    [[nodiscard]] bool parseIsComparableToValue(const std::string& val) const;
    [[nodiscard]] bool parseIsComparableToField(const ParseFieldImpl& field) const;

    [[nodiscard]] const ParsePropsMap& parseExtraAttributes() const
    {
        return m_state.m_extraAttrs;
    }

    [[nodiscard]] ParsePropsMap& parseExtraAttributes()
    {
        return m_state.m_extraAttrs;
    }

    [[nodiscard]] const ParseContentsList& parseExtraChildren() const
    {
        return m_state.m_extraChildren;
    }

    [[nodiscard]] ParseContentsList& parseExtraChildren()
    {
        return m_state.m_extraChildren;
    }

    [[nodiscard]] bool parseStrToNumeric(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
    {
        return parseStrToNumericImpl(ref, val, isBigUnsigned);
    }

    [[nodiscard]] bool parseStrToFp(const std::string& ref, double& val) const
    {
        return parseStrToFpImpl(ref, val);
    }

    [[nodiscard]] bool parseStrToBool(const std::string& ref, bool& val) const
    {
        return parseStrToBoolImpl(ref, val);
    }

    [[nodiscard]] bool parseStrToString(const std::string& ref, std::string& val) const
    {
        return parseStrToStringImpl(ref, val);
    }

    [[nodiscard]] bool parseStrToData(const std::string& ref, std::vector<std::uint8_t>& val) const
    {
        return parseStrToDataImpl(ref, val);
    }

    [[nodiscard]] bool parseValidateBitLengthValue(::xmlNodePtr node, std::size_t bitLength) const
    {
        return parseValidateBitLengthValueImpl(node, bitLength);
    }

    [[nodiscard]] bool parseValidateBitLengthValue(std::size_t bitLength) const
    {
        return parseValidateBitLengthValue(m_node, bitLength);
    }

    [[nodiscard]] bool parseVerifySemanticType() const;
    [[nodiscard]] bool parseVerifySemanticType(::xmlNodePtr node, ParseSemanticType type) const;
    [[nodiscard]] bool parseVerifyAliasedMember(const std::string& fieldName);
    [[nodiscard]] bool parseVerifyMinLength() const;

    [[nodiscard]] std::string parseSchemaPos() const;

    const ParseFieldsList& parseMembers() const
    {
        return parseMembersImpl();
    }

    [[nodiscard]] static ParseFieldRefInfo parseProcessSiblingRef(const ParseFieldsList& siblings, const std::string& refStr);

    [[nodiscard]] ParseFieldRefInfo parseProcessInnerRef(const std::string& refStr) const;

    [[nodiscard]] bool parseIsValidInnerRef(const std::string& refStr) const;
    [[nodiscard]] bool parseIsValidRefType(ParseFieldRefType type) const;

protected:
    ParseFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol);
    ParseFieldImpl(const ParseFieldImpl&);

    [[nodiscard]] ParseProtocolImpl& parseProtocol()
    {
        return m_protocol;
    }

    [[nodiscard]] const ParseProtocolImpl& parseProtocol() const
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

    [[nodiscard]] ParseLogWrapper parseLogError() const;
    [[nodiscard]] ParseLogWrapper parseLogWarning() const;
    [[nodiscard]] ParseLogWrapper parseLogInfo() const;

    [[nodiscard]] virtual ParseObjKind parseObjKindImpl() const override final;
    [[nodiscard]] virtual ParseKind parseKindImpl() const = 0;
    [[nodiscard]] virtual ParsePtr parseCloneImpl() const = 0;
    [[nodiscard]] virtual const ParseXmlWrap::ParseNamesList& parseExtraPropsNamesImpl() const;
    [[nodiscard]] virtual const ParseXmlWrap::ParseNamesList& parseExtraPossiblePropsNamesImpl() const;
    [[nodiscard]] virtual const ParseXmlWrap::ParseNamesList& parseExtraChildrenNamesImpl() const;
    [[nodiscard]] virtual bool parseReuseImpl(const ParseFieldImpl& other);
    [[nodiscard]] virtual bool parseImpl();
    [[nodiscard]] virtual bool parseReplaceMembersImpl(ParseFieldsList& members);
    [[nodiscard]] virtual bool parseVerifySiblingsImpl(const ParseFieldsList& fields) const;
    [[nodiscard]] virtual std::size_t parseMinLengthImpl() const = 0;
    [[nodiscard]] virtual std::size_t parseMaxLengthImpl() const;
    [[nodiscard]] virtual std::size_t parseBitLengthImpl() const;
    [[nodiscard]] virtual bool parseIsComparableToValueImpl(const std::string& val) const;
    [[nodiscard]] virtual bool parseIsComparableToFieldImpl(const ParseFieldImpl& field) const;
    [[nodiscard]] virtual bool parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const;
    [[nodiscard]] virtual bool parseStrToFpImpl(const std::string& ref, double& val) const;
    [[nodiscard]] virtual bool parseStrToBoolImpl(const std::string& ref, bool& val) const;
    [[nodiscard]] virtual bool parseStrToStringImpl(const std::string& ref, std::string& val) const;
    [[nodiscard]] virtual bool parseStrToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const;
    [[nodiscard]] virtual bool parseValidateBitLengthValueImpl(::xmlNodePtr node, std::size_t bitLength) const;
    [[nodiscard]] virtual bool parseVerifySemanticTypeImpl(::xmlNodePtr node, ParseSemanticType type) const;
    [[nodiscard]] virtual bool parseVerifyAliasedMemberImpl(const std::string& fieldName) const;
    [[nodiscard]] virtual const ParseXmlWrap::ParseNamesList& parseSupportedMemberTypesImpl() const;
    [[nodiscard]] virtual const ParseFieldsList& parseMembersImpl() const;
    [[nodiscard]] virtual ParseFieldRefInfo parseProcessInnerRefImpl(const std::string& refStr) const;
    [[nodiscard]] virtual bool parseIsValidRefTypeImpl(ParseFieldRefType type) const;

    [[nodiscard]] bool parseValidateSinglePropInstance(const std::string& str, bool mustHave = false);
    [[nodiscard]] bool parseValidateNoPropInstance(const std::string& str);
    [[nodiscard]] bool parseValidateAndUpdateStringPropValue(
            const std::string& str,
            std::string &value,
            bool mustHave = false,
            bool allowDeref = false);
    void parseReportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    void parseCheckAndReportDeprecatedPropertyValue(const std::string& propName);
    [[nodiscard]] bool parseValidateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave = false);
    [[nodiscard]] bool parseValidateAndUpdateUnsignedPropValue(const std::string& propName, unsigned& value, bool mustHave = false);
    [[nodiscard]] bool parseValidateAndUpdateOverrideTypePropValue(const std::string& propName, ParseOverrideType& value);

    [[nodiscard]] static const ParseXmlWrap::ParseNamesList& parseCommonProps();
    [[nodiscard]] static const ParseXmlWrap::ParseNamesList& parseCommonChildren();
    [[nodiscard]] const ParseFieldImpl* parseFindSibling(const ParseFieldsList& fields, const std::string& sibName) const;
    [[nodiscard]] static ParseKind parseGetNonRefFieldKind(const ParseFieldImpl& field);
    [[nodiscard]] bool parseCheckDetachedPrefixAllowed() const;

    using ParseStrToValueFieldConvertFunc = std::function<bool (const ParseFieldImpl& f, const std::string& ref)>;
    [[nodiscard]] bool parseStrToValueOnFields(
        const std::string& ref,
        const ParseFieldsList& fields,
        ParseStrToValueFieldConvertFunc&& func) const;

    [[nodiscard]] bool parseStrToNumericOnFields(
        const std::string& ref,
        const ParseFieldsList& fields,
        std::intmax_t& val,
        bool& isBigUnsigned) const;

    [[nodiscard]] bool parseStrToFpOnFields(
        const std::string& ref,
        const ParseFieldsList& fields,
        double& val) const;

    [[nodiscard]] bool parseStrToBoolOnFields(
        const std::string& ref,
        const ParseFieldsList& fields,
        bool& val) const;

    [[nodiscard]] bool parseStrToStringOnFields(
        const std::string& ref,
        const ParseFieldsList& fields,
        std::string& val) const;

    [[nodiscard]] bool parseStrToDataOnFields(
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
        unsigned m_readAlign = 0;
        unsigned m_writePadAlign = 0;
        bool m_pseudo = false;
        bool m_fixedValue = false;
        bool m_customizable = false;
        bool m_failOnInvalid = false;
        bool m_forceGen = false;
        bool m_writePadInLength = false;
    };

    [[nodiscard]] bool parseCheckReuse();
    [[nodiscard]] bool parseCheckReplace();
    [[nodiscard]] bool parseUpdateName();
    [[nodiscard]] bool parseUpdateDescription();
    [[nodiscard]] bool parseUpdateDisplayName();
    [[nodiscard]] bool parseUpdateVersions();
    [[nodiscard]] bool parseUpdateSemanticType();
    [[nodiscard]] bool parseUpdatePseudo();
    [[nodiscard]] bool parseUpdateFixedValue();
    [[nodiscard]] bool parseUpdateDisplayReadOnly();
    [[nodiscard]] bool parseUpdateDisplayHidden();
    [[nodiscard]] bool parseUpdateCustomizable();
    [[nodiscard]] bool parseUpdateFailOnInvalid();
    [[nodiscard]] bool parseUpdateForceGen();
    [[nodiscard]] bool parseUpdateValueOverride();
    [[nodiscard]] bool parseUpdateReadOverride();
    [[nodiscard]] bool parseUpdateWriteOverride();
    [[nodiscard]] bool parseUpdateRefreshOverride();
    [[nodiscard]] bool parseUpdateLengthOverride();
    [[nodiscard]] bool parseUpdateValidOverride();
    [[nodiscard]] bool parseUpdateNameOverride();
    [[nodiscard]] bool parseUpdateCopyOverrideCodeFrom();
    [[nodiscard]] bool parseUpdateValidateMinLength();
    [[nodiscard]] bool parseUpdateReadAlign();
    [[nodiscard]] bool parseUpdateWritePadAlign();
    [[nodiscard]] bool parseUpdateWritePadInLength();
    [[nodiscard]] bool parseUpdateExtraAttrs(const ParseXmlWrap::ParseNamesList& names);
    [[nodiscard]] bool parseUpdateExtraChildren(const ParseXmlWrap::ParseNamesList& names);

    [[nodiscard]] static const ParseCreateMap& parseCreateMap();

    [[nodiscard]] bool parseVerifyName() const;

    ::xmlNodePtr m_node = nullptr;
    ParseProtocolImpl& m_protocol;
    ParsePropsMap m_props;
    ParseReusableState m_state;
};

using ParseFieldImplPtr = ParseFieldImpl::ParsePtr;

} // namespace parse

} // namespace commsdsl
