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

#include "ParseFieldImpl.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <map>
#include <set>

#include "ParseProtocolImpl.h"
#include "ParseIntFieldImpl.h"
#include "ParseFloatFieldImpl.h"
#include "ParseEnumFieldImpl.h"
#include "ParseSetFieldImpl.h"
#include "ParseBitfieldFieldImpl.h"
#include "ParseBundleFieldImpl.h"
#include "ParseStringFieldImpl.h"
#include "ParseDataFieldImpl.h"
#include "ParseListFieldImpl.h"
#include "ParseRefFieldImpl.h"
#include "ParseOptionalFieldImpl.h"
#include "ParseVariantFieldImpl.h"
#include "ParseNamespaceImpl.h"
#include "parse_common.h"

namespace commsdsl
{

namespace parse
{

namespace {

const unsigned MinDslVersionForLengthSemanticType  = 2U;

} // namespace

ParseFieldImpl::Ptr ParseFieldImpl::parseCreate(
    const std::string& kind,
    ::xmlNodePtr node,
    ParseProtocolImpl& protocol)
{
    auto& map = parseCreateMap();

    auto iter = map.find(kind);
    if (iter == map.end()) {
        return Ptr();
    }

    return iter->second(node, protocol);
}

bool ParseFieldImpl::parse()
{
    m_props = ParseXmlWrap::parseNodeProps(m_node);

    if (!ParseXmlWrap::parseChildrenAsProps(m_node, parseCommonProps(), m_protocol.parseLogger(), m_props)) {
        return false;
    }

    auto& extraPropsNames = parseExtraPropsNamesImpl();
    do {
        if (extraPropsNames.empty()) {
            break;
        }

        if (!ParseXmlWrap::parseChildrenAsProps(m_node, extraPropsNames, m_protocol.parseLogger(), m_props)) {
            return false;
        }

    } while (false);

    auto& extraPossiblePropsNames = parseExtraPossiblePropsNamesImpl();
    do {
        if (extraPossiblePropsNames.empty()) {
            break;
        }

        if (!ParseXmlWrap::parseChildrenAsProps(m_node, extraPossiblePropsNames, m_protocol.parseLogger(), m_props, false)) {
            return false;
        }

    } while (false);

    bool result =
        parseCheckReuse() &&
        parseCheckReplace() &&
        parseUpdateName() &&
        parseUpdateDisplayName() &&
        parseUpdateDescription() &&
        parseUpdateVersions() &&
        parseUpdateSemanticType() &&
        parseUpdatePseudo() &&
        parseUpdateFixedValue() &&
        parseUpdateDisplayReadOnly() &&
        parseUpdateDisplayHidden() &&
        parseUpdateCustomizable() &&
        parseUpdateFailOnInvalid() &&
        parseUpdateForceGen() &&
        parseUpdateValueOverride() &&
        parseUpdateReadOverride() &&
        parseUpdateWriteOverride() &&
        parseUpdateRefreshOverride() &&
        parseUpdateLengthOverride() &&
        parseUpdateValidOverride() &&
        parseUpdateNameOverride() &&
        parseUpdateCopyOverrideCodeFrom() &&
        parseUpdateValidateMinLength();

    if (!result) {
        return false;
    }

    if (!parseImpl()) {
        return false;
    }

    if ((!parseVerifySemanticType()) ||
        (!parseVerifyName()) ||
        (!parseVerifyMinLength())) {
        return false;
    }

    ParseXmlWrap::NamesList expectedProps = parseCommonProps();
    expectedProps.insert(expectedProps.end(), extraPropsNames.begin(), extraPropsNames.end());
    expectedProps.insert(expectedProps.end(), extraPossiblePropsNames.begin(), extraPossiblePropsNames.end());
    if (!parseUpdateExtraAttrs(expectedProps)) {
        return false;
    }

    auto& commonCh = parseCommonChildren();
    auto& extraChildren = parseExtraChildrenNamesImpl();
    ParseXmlWrap::NamesList expectedChildren = parseCommonProps();
    expectedChildren.insert(expectedChildren.end(), commonCh.begin(), commonCh.end());
    expectedChildren.insert(expectedChildren.end(), extraPropsNames.begin(), extraPropsNames.end());
    expectedChildren.insert(expectedChildren.end(), extraPossiblePropsNames.begin(), extraPossiblePropsNames.end());
    expectedChildren.insert(expectedChildren.end(), extraChildren.begin(), extraChildren.end());
    if (!parseUpdateExtraChildren(expectedChildren)) {
        return false;
    }
    return true;
}

const std::string& ParseFieldImpl::parseName() const
{
    return m_state.m_name;
}

const std::string& ParseFieldImpl::parseDisplayName() const
{
    return m_state.m_displayName;
}

const std::string& ParseFieldImpl::parseDescription() const
{
    return m_state.m_description;
}

const std::string& ParseFieldImpl::parseKindStr() const
{
    static const std::string* const Map[] = {
        /* Int */ &common::intStr(),
        /* Enum */ &common::enumStr(),
        /* Set */ &common::setStr(),
        /* Float */ &common::floatStr(),
        /* Bitfield */ &common::bitfieldStr(),
        /* Bundle */ &common::bundleStr(),
        /* String */ &common::stringStr(),
        /* Data */ &common::dataStr(),
        /* List */ &common::listStr(),
        /* Ref */ &common::refStr(),
        /* Optional */ &common::optionalStr(),
        /* Variant */ &common::variantStr()
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == static_cast<unsigned>(Kind::NumOfValues), "Invalid map");

    auto idx = static_cast<unsigned>(parseKind());
    assert(idx < MapSize);
    return *Map[idx];
}

ParseXmlWrap::NamesList ParseFieldImpl::parseSupportedTypes()
{
    ParseXmlWrap::NamesList result;
    auto& map = parseCreateMap();
    result.reserve(map.size());
    std::transform(
        map.begin(), map.end(), std::back_inserter(result),
        [](auto& elem)
        {
            return elem.first;
        });
    return result;
}

bool ParseFieldImpl::parseValidateMembersNames(
    const ParseFieldImpl::FieldsList& fields,
    ParseLogger& logger)
{
    std::set<std::string> usedNames;
    for (auto& f : fields) {
        if (usedNames.find(f->parseName()) != usedNames.end()) {
            commsdsl::parse::parseLogError(logger) << ParseXmlWrap::parseLogPrefix(f->parseGetNode()) <<
                "Member field with name \"" << f->parseName() << "\" has already been defined.";
            return false;
        }
        usedNames.insert(f->parseName());
    }
    return true;
}

bool ParseFieldImpl::parseValidateMembersNames(const ParseFieldImpl::FieldsList& fields)
{
    return parseValidateMembersNames(fields, parseProtocol().parseLogger());
}

bool ParseFieldImpl::parseIsBitfieldMember() const
{
    return (parseGetParent() != nullptr) &&
           (parseGetParent()->parseObjKind() == ObjKind::Field) &&
            (static_cast<const ParseFieldImpl*>(parseGetParent())->parseKind() == Kind::Bitfield);
}

bool ParseFieldImpl::parseIsBundleMember() const
{
    return (parseGetParent() != nullptr) &&
           (parseGetParent()->parseObjKind() == ObjKind::Field) &&
            (static_cast<const ParseFieldImpl*>(parseGetParent())->parseKind() == Kind::Bundle);
}

bool ParseFieldImpl::parseIsMessageMember() const
{
    return (parseGetParent() != nullptr) &&
           (parseGetParent()->parseObjKind() == ObjKind::Message);
}

std::string ParseFieldImpl::parseExternalRef(bool schemaRef) const
{
    if ((parseGetParent() == nullptr) || (parseGetParent()->parseObjKind() != ObjKind::Namespace)) {
        return common::emptyString();
    }

    auto& ns = static_cast<const ParseNamespaceImpl&>(*parseGetParent());
    auto nsRef = ns.parseExternalRef(schemaRef);
    if (nsRef.empty()) {
        return parseName();
    }

    return nsRef + '.' + parseName();
}

bool ParseFieldImpl::parseIsComparableToValue(const std::string& val) const
{
    static const SemanticType NumericSemanticTypes[] = {
        SemanticType::Version,
        SemanticType::Length,
    };

    auto iter = std::find(std::begin(NumericSemanticTypes), std::end(NumericSemanticTypes), parseSemanticType());

    if (iter != std::end(NumericSemanticTypes)) {
        bool ok = false;
        [[maybe_unused]] auto value = common::strToIntMax(val, &ok);
        return ok;        
    }

    return parseIsComparableToValueImpl(val);
}

bool ParseFieldImpl::parseIsComparableToField(const ParseFieldImpl& field) const
{
    if (field.parseKind() == Kind::Ref) {
        auto& refField = static_cast<const ParseRefFieldImpl&>(field);
        auto* referee = refField.parseFieldImpl();
        if (referee == nullptr) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            return false;
        }

        return parseIsComparableToField(*referee);
    }

    if (parseKind() == field.parseKind()) {
        return true;
    }

    return parseIsComparableToFieldImpl(field);
}

bool ParseFieldImpl::parseVerifySemanticType() const
{
    return parseVerifySemanticType(parseGetNode(), parseSemanticType());
}

bool ParseFieldImpl::parseVerifySemanticType(::xmlNodePtr node, SemanticType type) const
{
    if (type == SemanticType::None) {
        return true;
    }

    if (parseVerifySemanticTypeImpl(node, type)) {
        return true;
    }

    if (type == SemanticType::Version) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(node) <<
            "Semantic type \"" << common::versionStr() << "\" is applicable only to \"" <<
            common::intStr() << "\" fields or \"" << common::refStr() << "\" to them.";
        return false;
    }

    if (type == SemanticType::MessageId) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(node) <<
            "Semantic type \"" << common::messageIdStr() << "\" is applicable only to \"" <<
            common::enumStr() << "\" fields.";
        return false;
    }

    if (type == SemanticType::Length) {
        if (!m_protocol.parseIsSemanticTypeLengthSupported()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(node) <<
                "Semantic type \"" << common::lengthStr() << "\" supported only since "
                "DSL v" << MinDslVersionForLengthSemanticType << ", please update \"" <<
                common::dslVersionStr() << "\" property of your schema.";
            return false;
        }

        parseLogError() << ParseXmlWrap::parseLogPrefix(node) <<
            "Semantic type \"" << common::lengthStr() << "\" is not applicable to this field type.";
        return false;
    }

    [[maybe_unused]] static constexpr bool Unexpected_semantic_type = false;
    assert(Unexpected_semantic_type);
    return true;
}

bool ParseFieldImpl::parseVerifyAliasedMember(const std::string& fieldName)
{
    if (fieldName.empty()) {
        return true;
    }

    return parseVerifyAliasedMemberImpl(fieldName);
}

bool ParseFieldImpl::parseVerifyMinLength() const
{
    if (m_state.m_validateMinLength < 0) {
        return true;
    }

    auto len = parseMinLength();
    if (static_cast<unsigned>(m_state.m_validateMinLength) != len) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The calculated minimal length of the field is " << len <<
            " while expected is " << m_state.m_validateMinLength << " (specified with \"" << common::validateMinLengthStr() << "\" property).";                
        return false;
    }    

    return true;
}

std::string ParseFieldImpl::parseSchemaPos() const
{
    return ParseXmlWrap::parseLogPrefix(m_node);
}

ParseFieldImpl::FieldRefInfo ParseFieldImpl::parseProcessSiblingRef(const FieldsList& siblings, const std::string& refStr)
{
    FieldRefInfo info;

    if ((!refStr.empty()) && ((refStr[0] == '#') || (refStr[0] == '?'))) {
        info = parseProcessSiblingRef(siblings, refStr.substr(1));
        do {
            if ((info.m_field == nullptr) || 
                (info.m_refType != FieldRefType::FieldRefType_Field)) {
                info = FieldRefInfo();
                break;
            }

            if (refStr[0] == '#') {
                info.m_refType = FieldRefType::FieldRefType_Size;
                break;
            }

            assert(refStr[0] == '?');
            info.m_refType = FieldRefType::FieldRefType_Exists;
            break;
        } while (false);

        if ((info.m_field != nullptr) && (!info.m_field->parseIsValidRefType(info.m_refType))) {
            info = FieldRefInfo();
        }

        return info;
    }

    auto dotPos = refStr.find_first_of('.');
    std::string fieldName(refStr, 0, dotPos);
    if (fieldName.empty()) {
        return info;
    }

    auto iter =
        std::find_if(
            siblings.begin(), siblings.end(),
            [&fieldName](auto& f)
            {
                return f->parseName() == fieldName;
            });

    if (iter == siblings.end()) {
        return info;
    }

    auto nextPos = refStr.size();
    if (dotPos < refStr.size()) {
        nextPos = dotPos + 1U;
    }

    return (*iter)->parseProcessInnerRef(refStr.substr(nextPos));
}

ParseFieldImpl::FieldRefInfo ParseFieldImpl::parseProcessInnerRef(const std::string& refStr) const
{
    if (refStr.empty()) {
        FieldRefInfo info;
        info.m_field = this;
        info.m_refType = FieldRefType_Field;
        return info;
    }

    auto& memFields = parseMembers();
    if (!memFields.empty()) {
        return parseProcessSiblingRef(memFields, refStr);
    }    

    return parseProcessInnerRefImpl(refStr);
}

bool ParseFieldImpl::parseIsValidInnerRef(const std::string& refStr) const
{
    auto info = parseProcessInnerRef(refStr);
    return info.m_field != nullptr;
}

bool ParseFieldImpl::parseIsValidRefType(FieldRefType type) const
{
    if (type == FieldRefType_Invalid) {
        return false;
    }

    if (type == FieldRefType_Field) {
        return true;
    }

    if ((type == FieldRefType_Exists) && 
        ((parseGetSinceVersion() > 0U) || (parseGetDeprecated() < ParseProtocol::parseNotYetDeprecated()))) {
        return true;
    }

    return parseIsValidRefTypeImpl(type);
}

ParseFieldImpl::ParseFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : m_node(node),
    m_protocol(protocol)
{
}

ParseFieldImpl::ParseFieldImpl(const ParseFieldImpl&) = default;

LogWrapper ParseFieldImpl::parseLogError() const
{
    return commsdsl::parse::parseLogError(m_protocol.parseLogger());
}

LogWrapper ParseFieldImpl::parseLogWarning() const
{
    return commsdsl::parse::parseLogWarning(m_protocol.parseLogger());
}

LogWrapper ParseFieldImpl::parseLogInfo() const
{
    return commsdsl::parse::parseLogInfo(m_protocol.parseLogger());
}

ParseObject::ObjKind ParseFieldImpl::parseObjKindImpl() const
{
    return ObjKind::Field;
}

const ParseXmlWrap::NamesList& ParseFieldImpl::parseExtraPropsNamesImpl() const
{
    return ParseXmlWrap::parseEmptyNamesList();
}

const ParseXmlWrap::NamesList&ParseFieldImpl::parseExtraPossiblePropsNamesImpl() const
{
    return ParseXmlWrap::parseEmptyNamesList();
}

const ParseXmlWrap::NamesList& ParseFieldImpl::parseExtraChildrenNamesImpl() const
{
    static const ParseXmlWrap::NamesList Names;
    return Names;
}

bool ParseFieldImpl::parseReuseImpl([[maybe_unused]] const ParseFieldImpl& other)
{
    [[maybe_unused]] static constexpr bool Should_not_happen = false;
    assert(Should_not_happen);
    return false;
}

bool ParseFieldImpl::parseImpl()
{
    return true;
}

bool ParseFieldImpl::parseReplaceMembersImpl([[maybe_unused]] FieldsList& members)
{
    parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
        "The field of kind \"" << parseKindStr() << "\" does not support replacing its members.";
    return false;
}

bool ParseFieldImpl::parseVerifySiblingsImpl([[maybe_unused]] const ParseFieldImpl::FieldsList& fields) const
{
    return true;
}

std::size_t ParseFieldImpl::parseMaxLengthImpl() const
{
    return this->parseMinLengthImpl();
}

std::size_t ParseFieldImpl::parseBitLengthImpl() const
{
    return 0U;
}

bool ParseFieldImpl::parseIsComparableToValueImpl([[maybe_unused]] const std::string& val) const
{
    return false;
}

bool ParseFieldImpl::parseIsComparableToFieldImpl([[maybe_unused]] const ParseFieldImpl& field) const
{
    return false;
}

bool ParseFieldImpl::parseStrToNumericImpl(
    [[maybe_unused]] const std::string& ref, 
    [[maybe_unused]] std::intmax_t& val, 
    [[maybe_unused]] bool& isBigUnsigned) const
{
    if (parseProtocol().parseIsFieldValueReferenceSupported()) {
        parseLogWarning() <<
            "Extracting integral numeric value from \"" << parseKindStr() <<
            "\" field is not supported.";
    }

    return false;
}

bool ParseFieldImpl::parseStrToFpImpl(const std::string& ref, double& val) const
{
    std::intmax_t intVal = 0;
    bool isBigUnsigned = false;
    if (!parseStrToNumeric(ref, intVal, isBigUnsigned)) {
        return false;
    }

    if (!isBigUnsigned) {
        val = static_cast<double>(intVal);
        return true;
    }

    val = static_cast<double>(static_cast<std::uintmax_t>(intVal));
    return true;
}

bool ParseFieldImpl::parseStrToBoolImpl([[maybe_unused]] const std::string& ref, [[maybe_unused]] bool& val) const
{
    if (parseProtocol().parseIsFieldValueReferenceSupported()) {
        parseLogWarning() <<
            "Extracting boolean value from \"" << parseKindStr() <<
            "\" field is not supported.";
    }

    return false;
}

bool ParseFieldImpl::parseStrToStringImpl([[maybe_unused]] const std::string& ref, [[maybe_unused]] std::string& val) const
{
    if (parseProtocol().parseIsFieldValueReferenceSupported()) {
        parseLogWarning() <<
            "Extracting string value from \"" << parseKindStr() <<
            "\" field is not supported.";
    }

    return false;
}

bool ParseFieldImpl::parseStrToDataImpl([[maybe_unused]] const std::string& ref, [[maybe_unused]] std::vector<std::uint8_t>& val) const
{
    if (parseProtocol().parseIsFieldValueReferenceSupported()) {
        parseLogWarning() <<
            "Extracting data value from \"" << parseKindStr() <<
            "\" field is not supported.";
    }

    return false;
}

bool ParseFieldImpl::parseValidateBitLengthValueImpl(::xmlNodePtr node, [[maybe_unused]] std::size_t bitLength) const
{
    parseLogError() << ParseXmlWrap::parseLogPrefix(node) <<
        "The field of kind \"" << parseKindStr() << "\" cannot be used or referenced as a member of \"" <<
        common::bitfieldStr() << "\".";    
    return false;
}

bool ParseFieldImpl::parseVerifySemanticTypeImpl([[maybe_unused]] ::xmlNodePtr node, [[maybe_unused]] SemanticType type) const
{
    return false;
}

bool ParseFieldImpl::parseVerifyAliasedMemberImpl([[maybe_unused]] const std::string& fieldName) const
{
    return false;
}

const ParseXmlWrap::NamesList& ParseFieldImpl::parseSupportedMemberTypesImpl() const
{
    static const ParseXmlWrap::NamesList List;
    return List;
}

const ParseFieldImpl::FieldsList& ParseFieldImpl::parseMembersImpl() const
{
    static const FieldsList List;
    return List;
}

ParseFieldImpl::FieldRefInfo ParseFieldImpl::parseProcessInnerRefImpl([[maybe_unused]] const std::string& refStr) const
{
    assert(!refStr.empty());
    FieldRefInfo info;
    return info;
}

bool ParseFieldImpl::parseIsValidRefTypeImpl([[maybe_unused]] FieldRefType type) const
{
    return false;
}

bool ParseFieldImpl::parseValidateSinglePropInstance(const std::string& str, bool mustHave)
{
    return ParseXmlWrap::parseValidateSinglePropInstance(m_node, m_props, str, parseProtocol().parseLogger(), mustHave);
}

bool ParseFieldImpl::parseValidateNoPropInstance(const std::string& str)
{
    return ParseXmlWrap::parseValidateNoPropInstance(m_node, m_props, str, parseProtocol().parseLogger());
}

bool ParseFieldImpl::parseValidateAndUpdateStringPropValue(
    const std::string& str,
    std::string& value,
    bool mustHave,
    bool allowDeref)
{
    if (!parseValidateSinglePropInstance(str, mustHave)) {
        return false;
    }

    auto iter = m_props.find(str);
    if (iter == m_props.end()) {
        assert(!mustHave);
        return true;
    }

    if (!allowDeref) {
        value = iter->second;
        return true;
    }

    if (!parseProtocol().parseStrToStringValue(iter->second, value)) {
        parseReportUnexpectedPropertyValue(str, iter->second);
        return false;
    }

    return true;
}

void ParseFieldImpl::parseReportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue)
{
    ParseXmlWrap::parseReportUnexpectedPropertyValue(m_node, parseName(), propName, propValue, parseProtocol().parseLogger());
}

void ParseFieldImpl::parseCheckAndReportDeprecatedPropertyValue(const std::string& propName)
{
    auto iter = m_props.find(propName);
    if (iter == m_props.end()) {
        return;
    }

    if (m_protocol.parseIsPropertyDeprecated(propName)) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Property \"" << propName << "\" is deprecated in DSL version " << parseProtocol().parseCurrSchema().parseDslVersion();                
    }
}

bool ParseFieldImpl::parseValidateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave)
{
    if (!parseValidateSinglePropInstance(propName, mustHave)) {
        return false;
    }

    auto iter = m_props.find(propName);
    if (iter == m_props.end()) {
        return true;
    }

    if (!m_protocol.parseIsPropertySupported(propName)) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Property \"" << propName << "\" is not available for DSL version " << parseProtocol().parseCurrSchema().parseDslVersion();                
        return true;
    }

    bool ok = false;
    value = common::parseStrToBool(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(propName, iter->second);
        return false;
    }

    return true;
}

bool ParseFieldImpl::parseValidateAndUpdateOverrideTypePropValue(const std::string& propName, ParseOverrideType& value)
{
    if (!parseValidateSinglePropInstance(propName, false)) {
        return false;
    }

    auto iter = m_props.find(propName);
    if (iter == m_props.end()) {
        value = ParseOverrideType_Any;
        return true;
    }    

    if (!m_protocol.parseIsOverrideTypeSupported()) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The property \"" << propName << "\" is not supported for dslVersion=" << 
                m_protocol.parseCurrSchema().parseDslVersion() << ".";        
        return true;
    }

    static const std::map<std::string, ParseOverrideType> Map = {
        {std::string(), ParseOverrideType_Any},
        {"any", ParseOverrideType_Any},
        {"replace", ParseOverrideType_Replace},
        {"extend", ParseOverrideType_Extend},
        {"none", ParseOverrideType_None},
    };

    auto valIter = Map.find(common::toLowerCopy(iter->second));
    if (valIter == Map.end()) {
        parseReportUnexpectedPropertyValue(propName, iter->second);
        return false;        
    }

    value = valIter->second;
    return true;
}

const ParseXmlWrap::NamesList& ParseFieldImpl::parseCommonProps()
{
    static const ParseXmlWrap::NamesList CommonNames = {
        common::nameStr(),
        common::displayNameStr(),
        common::descriptionStr(),
        common::sinceVersionStr(),
        common::deprecatedStr(),
        common::removedStr(),
        common::reuseStr(),
        common::semanticTypeStr(),
        common::pseudoStr(),
        common::fixedValueStr(),
        common::displayReadOnlyStr(),
        common::displayHiddenStr(),
        common::customizableStr(),
        common::failOnInvalidStr(),
        common::forceGenStr(),
        common::valueOverrideStr(),
        common::readOverrideStr(),
        common::writeOverrideStr(),
        common::refreshOverrideStr(),
        common::lengthOverrideStr(),
        common::validOverrideStr(),
        common::nameOverrideStr(),
        common::copyCodeFromStr(),
        common::reuseCodeStr(),
        common::validateMinLengthStr(),
    };

    return CommonNames;
}

const ParseXmlWrap::NamesList& ParseFieldImpl::parseCommonChildren()
{
    static const ParseXmlWrap::NamesList CommonChildren = {
        common::metaStr(),
        common::replaceStr()
    };

    return CommonChildren;
}

const ParseFieldImpl* ParseFieldImpl::parseFindSibling(const FieldsList& fields, const std::string& sibName) const
{
    auto pos = sibName.find_first_of(".");
    auto actSibName = sibName.substr(0, pos);

    auto iter =
        std::find_if(
            fields.begin(), fields.end(),
            [&actSibName](auto& f)
            {
                return f->parseName() == actSibName;
            });

    if (iter == fields.end()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The holding bundle/message does not contain field named \"" <<
            sibName << "\".";

        return nullptr;
    }

    auto* firstLevelSibling = iter->get();
    if (sibName.size() <= pos) {
        return firstLevelSibling;
    }

    auto rest = sibName.substr(pos + 1);
    return parseFindSibling(firstLevelSibling->parseMembers(), rest);
}

ParseFieldImpl::Kind ParseFieldImpl::parseGetNonRefFieldKind(const ParseFieldImpl& field)
{
    const ParseFieldImpl* referee = &field;
    while (referee->parseKind() == Kind::Ref) {
        auto* castedField = static_cast<const ParseRefFieldImpl*>(referee);
        referee = castedField->parseFieldImpl();
        assert(referee != nullptr);
    }

    return referee->parseKind();
}

bool ParseFieldImpl::parseCheckDetachedPrefixAllowed() const
{
    auto* parent = parseGetParent();
    bool result = false;
    do {
        if (parent == nullptr) {
            break;
        }

        auto objK = parent->parseObjKind();
        if (objK == ObjKind::Message) {
            result = true;
            break;
        }

        if (objK != ObjKind::Field) {
            break;
        }

        auto* parentField = static_cast<const ParseFieldImpl*>(parent);
        result = (parentField->parseKind() == Kind::Bundle);
    } while (false);

    if (!result) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Detached prefixes are allowed only for members of \"" << common::bundleStr() << "\" field "
            "or \"" << common::messageStr() << "\" object.";
    }
    return result;
}

bool ParseFieldImpl::parseStrToValueOnFields(
    const std::string &ref,
    const FieldsList &fields,
    ParseFieldImpl::StrToValueFieldConvertFunc &&func) const
{
    if (!parseProtocol().parseIsFieldValueReferenceSupported()) {
        return false;
    }

    auto firstDotPos = ref.find_first_of('.');
    std::string firstName(ref, 0, firstDotPos);

    auto iter = std::find_if(
        fields.begin(), fields.end(),
        [&firstName](auto& m)
        {
            return m->parseName() == firstName;
        });

    if (iter == fields.end()) {
        return false;
    }

    std::string restName;
    if (firstDotPos != std::string::npos) {
        restName.assign(ref, firstDotPos + 1, std::string::npos);
    }

    return func(**iter, restName);

}

bool ParseFieldImpl::parseStrToNumericOnFields(
    const std::string &ref,
    const FieldsList &fields,
    std::intmax_t &val,
    bool &isBigUnsigned) const
{
    if (ref.empty()) {
        return ParseFieldImpl::parseStrToNumericImpl(ref, val, isBigUnsigned);
    }

    return
        parseStrToValueOnFields(
            ref, fields,
            [&val, &isBigUnsigned](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToNumeric(str, val, isBigUnsigned);
            });

}

bool ParseFieldImpl::parseStrToFpOnFields(
    const std::string &ref,
    const FieldsList &fields,
    double &val) const
{
    if (ref.empty()) {
        return ParseFieldImpl::parseStrToFpImpl(ref, val);
    }

    return
        parseStrToValueOnFields(
            ref, fields,
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToFp(str, val);
    });
}

bool ParseFieldImpl::parseStrToBoolOnFields(
    const std::string &ref,
    const FieldsList &fields,
    bool &val) const
{
    if (ref.empty()) {
        return ParseFieldImpl::parseStrToBoolImpl(ref, val);
    }

    return
        parseStrToValueOnFields(
            ref, fields,
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToBool(str, val);
    });
}

bool ParseFieldImpl::parseStrToStringOnFields(
    const std::string &ref,
    const FieldsList &fields,
    std::string &val) const
{
    if (ref.empty()) {
        return ParseFieldImpl::parseStrToStringImpl(ref, val);
    }

    return
        parseStrToValueOnFields(
            ref, fields,
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToString(str, val);
            });

}

bool ParseFieldImpl::parseStrToDataOnFields(
    const std::string &ref,
    const FieldsList &fields,
    std::vector<std::uint8_t> &val) const
{
    if (ref.empty()) {
        return ParseFieldImpl::parseStrToDataImpl(ref, val);
    }

    return
        parseStrToValueOnFields(
            ref, fields,
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToData(str, val);
            });
}

bool ParseFieldImpl::parseCheckReuse()
{
    if (!parseValidateSinglePropInstance(common::reuseStr())) {
        return false;
    }

    auto iter = m_props.find(common::reuseStr());
    if (iter == m_props.end()) {
        return true;
    }

    auto& valueStr = iter->second;
    auto* field = m_protocol.parseFindField(valueStr);
    if (field == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "The field \"" << valueStr << "\" hasn't been recorded yet.";
        return false;
    }

    if (field->parseKind() != parseKind()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Cannot reuse field of different kind (\"" << valueStr << "\").";
        return false;
    }

    assert(field != this);
    Base::parseReuseState(*field);
    m_state = field->m_state;

    assert(parseGetSinceVersion() == 0U);
    assert(parseGetDeprecated() == ParseProtocol::parseNotYetDeprecated());
    assert(!parseIsDeprecatedRemoved());

    do {
        auto& codeProp = common::reuseCodeStr();
        if (!parseValidateSinglePropInstance(codeProp, false)) {
            return false;
        }

        m_state.m_copyCodeFrom.clear();
        auto codeIter = m_props.find(codeProp);
        if (codeIter == m_props.end()) {
            break;
        }  

        bool copyCode = false;
        if (!parseValidateAndUpdateBoolPropValue(codeProp, copyCode)) {
            return false;
        }

        if (!copyCode) {
            break;
        }

        m_state.m_copyCodeFrom = valueStr; 
    } while (false);
    return parseReuseImpl(*field);
}

bool ParseFieldImpl::parseCheckReplace()
{
    auto replaceNodes = ParseXmlWrap::parseGetChildren(parseGetNode(), common::replaceStr());
    if (1U < replaceNodes.size()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Only single \"" << common::replaceStr() << "\" child element is "
            "supported for a field.";
        return false;
    }

    if (replaceNodes.empty()) {
        return true;
    }

    if (!m_protocol.parseIsMemberReplaceSupported()) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Replacing members with \"" << common::replaceStr() << "\" child element is unavaliable "
            "for selected DSL version, ignoring...";        
        return true;
    }

    auto memberFieldsTypes = ParseXmlWrap::parseGetChildren(replaceNodes.front());
    auto cleanMemberFieldsTypes = ParseXmlWrap::parseGetChildren(replaceNodes.front(), parseSupportedMemberTypesImpl());
    if (cleanMemberFieldsTypes.size() != memberFieldsTypes.size()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(replaceNodes.front()) <<
            "The \"" << common::replaceStr() << "\" child node element must contain only supported member fields.";
        return false;
    }

    FieldsList replMembers;
    replMembers.reserve(memberFieldsTypes.size());
    for (auto* fieldNode : memberFieldsTypes) {
        std::string fieldKind(reinterpret_cast<const char*>(fieldNode->name));
        auto field = ParseFieldImpl::parseCreate(fieldKind, fieldNode, parseProtocol());
        if (!field) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            parseLogError() << ParseXmlWrap::parseLogPrefix(replaceNodes.front()) <<
                "Internal error, failed to create objects for member fields to replace.";
            return false;
        }

        field->parseSetParent(this);
        if (!field->parse()) {
            return false;
        }

        replMembers.push_back(std::move(field));
    }  

    assert(!replMembers.empty());
    return parseReplaceMembersImpl(replMembers);
}

bool ParseFieldImpl::parseUpdateName()
{
    return parseValidateAndUpdateStringPropValue(common::nameStr(), m_state.m_name);
}

bool ParseFieldImpl::parseUpdateDescription()
{
    return parseValidateAndUpdateStringPropValue(common::descriptionStr(), m_state.m_description, false, true);
}

bool ParseFieldImpl::parseUpdateDisplayName()
{
    return parseValidateAndUpdateStringPropValue(common::displayNameStr(), m_state.m_displayName, false, true);
}

bool ParseFieldImpl::parseUpdateVersions()
{
    if (!parseValidateSinglePropInstance(common::sinceVersionStr())) {
        return false;
    }

    if (!parseValidateSinglePropInstance(common::deprecatedStr())) {
        return false;
    }

    if (!parseValidateSinglePropInstance(common::removedStr())) {
        return false;
    }

    unsigned sinceVersion = 0U;
    if ((parseGetParent() != nullptr) && (parseGetParent()->parseObjKind() != ObjKind::Namespace)) {
        sinceVersion = parseGetParent()->parseGetSinceVersion();
    }

    unsigned deprecated = ParseProtocol::parseNotYetDeprecated();
    if ((parseGetParent() != nullptr) && (parseGetParent()->parseObjKind() != ObjKind::Namespace)) {
        deprecated = parseGetParent()->parseGetDeprecated();
    }

    if (!ParseXmlWrap::parseGetAndCheckVersions(m_node, parseName(), m_props, sinceVersion, deprecated, parseProtocol())) {
        return false;
    }

    do {
        if ((parseGetParent() != nullptr) &&
            ((parseGetParent()->parseObjKind() == ObjKind::Field) || (parseGetParent()->parseObjKind() == ObjKind::Message))) {
            break;
        }

        if (sinceVersion != 0U) {
            parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Property \"" << common::sinceVersionStr() << "\" is not applicable to "
                "this field, ignoring provided value";
            sinceVersion = 0U;
        }

        if (deprecated != ParseProtocol::parseNotYetDeprecated()) {
            parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Property \"" << common::deprecatedStr() << "\" is not applicable to "
                "this field, ignoring provided value";
            deprecated = ParseProtocol::parseNotYetDeprecated();
        }

    } while (false);

    bool deprecatedRemoved = false;
    do {
        auto deprecatedRemovedIter = m_props.find(common::removedStr());
        if (deprecatedRemovedIter == m_props.end()) {
            break;
        }

        bool ok = false;
        deprecatedRemoved = common::parseStrToBool(deprecatedRemovedIter->second, &ok);
        if (!ok) {
            parseReportUnexpectedPropertyValue(common::removedStr(), deprecatedRemovedIter->second);
            return false;
        }

        if (!deprecatedRemoved) {
            break;
        }

        if (deprecated == ParseProtocol::parseNotYetDeprecated()) {
            parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Property \"" << common::removedStr() << "\" is not applicable to "
                "non deprecated fields";
        }
    } while (false);

    parseSetSinceVersion(sinceVersion);
    parseSetDeprecated(deprecated);
    parseSetDeprecatedRemoved(deprecatedRemoved);
    return true;
}

bool ParseFieldImpl::parseUpdateSemanticType()
{
    if (!parseValidateSinglePropInstance(common::semanticTypeStr())) {
        return false;
    }

    auto iter = m_props.find(common::semanticTypeStr());
    if (iter == m_props.end()) {
        return true;
    }

    static const std::string Map[] = {
        /* None */ common::noneStr(),
        /* Version */ common::versionStr(),
        /* MessageId */ common::toLowerCopy(common::messageIdStr()),
        /* Length */ common::lengthStr(),
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == static_cast<std::size_t>(SemanticType::NumOfValues),
        "Invalid map");

    if (iter->second.empty()) {
        m_state.m_semanticType = SemanticType::None;
        return true;
    }

    auto typeVal = common::toLowerCopy(iter->second);
    auto valIter = std::find(std::begin(Map), std::end(Map), typeVal);
    if (valIter == std::end(Map)) {
        parseReportUnexpectedPropertyValue(common::semanticTypeStr(), iter->second);
        return false;
    }

    m_state.m_semanticType =
        static_cast<SemanticType>(std::distance(std::begin(Map), valIter));

    return true;
}

bool ParseFieldImpl::parseUpdatePseudo()
{
    return parseValidateAndUpdateBoolPropValue(common::pseudoStr(), m_state.m_pseudo);
}

bool ParseFieldImpl::parseUpdateFixedValue()
{
    return parseValidateAndUpdateBoolPropValue(common::fixedValueStr(), m_state.m_fixedValue);
}

bool ParseFieldImpl::parseUpdateDisplayReadOnly()
{
    parseCheckAndReportDeprecatedPropertyValue(common::displayReadOnlyStr());
    return true;
}

bool ParseFieldImpl::parseUpdateDisplayHidden()
{
    parseCheckAndReportDeprecatedPropertyValue(common::displayHiddenStr());
    return true;
}

bool ParseFieldImpl::parseUpdateCustomizable()
{
    return parseValidateAndUpdateBoolPropValue(common::customizableStr(), m_state.m_customizable);
}

bool ParseFieldImpl::parseUpdateFailOnInvalid()
{
    return parseValidateAndUpdateBoolPropValue(common::failOnInvalidStr(), m_state.m_failOnInvalid);
}

bool ParseFieldImpl::parseUpdateForceGen()
{
    return parseValidateAndUpdateBoolPropValue(common::forceGenStr(), m_state.m_forceGen);
}

bool ParseFieldImpl::parseUpdateValueOverride()
{
    return parseValidateAndUpdateOverrideTypePropValue(common::valueOverrideStr(), m_state.m_valueOverride);
}

bool ParseFieldImpl::parseUpdateReadOverride()
{
    return parseValidateAndUpdateOverrideTypePropValue(common::readOverrideStr(), m_state.m_readOverride);
}

bool ParseFieldImpl::parseUpdateWriteOverride()
{
    return parseValidateAndUpdateOverrideTypePropValue(common::writeOverrideStr(), m_state.m_writeOverride);
}

bool ParseFieldImpl::parseUpdateRefreshOverride()
{
    return parseValidateAndUpdateOverrideTypePropValue(common::refreshOverrideStr(), m_state.m_refreshOverride);
}

bool ParseFieldImpl::parseUpdateLengthOverride()
{
    return parseValidateAndUpdateOverrideTypePropValue(common::lengthOverrideStr(), m_state.m_lengthOverride);
}

bool ParseFieldImpl::parseUpdateValidOverride()
{
    return parseValidateAndUpdateOverrideTypePropValue(common::validOverrideStr(), m_state.m_validOverride);
}

bool ParseFieldImpl::parseUpdateNameOverride()
{
    return parseValidateAndUpdateOverrideTypePropValue(common::nameOverrideStr(), m_state.m_nameOverride);
}

bool ParseFieldImpl::parseUpdateCopyOverrideCodeFrom()
{
    auto& prop = common::copyCodeFromStr();
    if (!parseValidateSinglePropInstance(prop, false)) {
        return false;
    }

    auto iter = m_props.find(prop);
    if (iter == m_props.end()) {
        return true;
    }  

    if (!m_protocol.parseIsPropertySupported(prop)) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "The property \"" << prop << "\" is not supported for dslVersion=" << 
                m_protocol.parseCurrSchema().parseDslVersion() << ".";        
        return true;
    }    

    auto* field = m_protocol.parseFindField(iter->second);
    if (field == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Field referenced by \"" << prop << "\" property (" + iter->second + ") is not found.";
        return false;        
    }

    m_state.m_copyCodeFrom = iter->second;
    return true;
}

bool ParseFieldImpl::parseUpdateValidateMinLength()
{
    auto& propStr = common::validateMinLengthStr();
    if (!parseValidateSinglePropInstance(propStr)) {
        return false;
    }

    auto iter = m_props.find(propStr);
    if (iter == m_props.end()) {
        return true;
    }

    if (!m_protocol.parseIsValidateMinLengthForFieldsSupported()) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Property \"" << propStr << "\" for fields is not supported for DSL version " << m_protocol.parseCurrSchema().parseDslVersion() << ", ignoring...";
        return true;
    }

    bool ok = false;
    m_state.m_validateMinLength = static_cast<decltype(m_state.m_validateMinLength)>(common::strToUnsigned(iter->second, &ok));
    if (!ok) {
        parseReportUnexpectedPropertyValue(propStr, iter->second);
        return false;
    }    
    return true;
}

bool ParseFieldImpl::parseUpdateExtraAttrs(const ParseXmlWrap::NamesList& names)
{
    auto extraAttrs = ParseXmlWrap::parseGetExtraAttributes(m_node, names, m_protocol);
    if (extraAttrs.empty()) {
        return true;
    }

    if (m_state.m_extraAttrs.empty()) {
        m_state.m_extraAttrs = std::move(extraAttrs);
        return true;
    }

    std::move(extraAttrs.begin(), extraAttrs.end(), std::inserter(m_state.m_extraAttrs, m_state.m_extraAttrs.end()));
    return true;
}

bool ParseFieldImpl::parseUpdateExtraChildren(const ParseXmlWrap::NamesList& names)
{
    auto extraChildren = ParseXmlWrap::parseGetExtraChildren(m_node, names, m_protocol);
    if (extraChildren.empty()) {
        return true;
    }

    if (m_state.m_extraChildren.empty()) {
        m_state.m_extraChildren = std::move(extraChildren);
        return true;
    }

    m_state.m_extraChildren.reserve(m_state.m_extraChildren.size() + extraChildren.size());
    std::move(extraChildren.begin(), extraChildren.end(), std::back_inserter(m_state.m_extraChildren));
    return true;
}


const ParseFieldImpl::CreateMap& ParseFieldImpl::parseCreateMap()
{
    static const CreateMap Map = {
        std::make_pair(
            common::intStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return Ptr(new ParseIntFieldImpl(n, p));
            }),
        std::make_pair(
            common::floatStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return Ptr(new ParseFloatFieldImpl(n, p));
            }),
        std::make_pair(
            common::enumStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return Ptr(new ParseEnumFieldImpl(n, p));
            }),
        std::make_pair(
            common::setStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return Ptr(new ParseSetFieldImpl(n, p));
            }),
        std::make_pair(
            common::bitfieldStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return Ptr(new ParseBitfieldFieldImpl(n, p));
            }),
        std::make_pair(
            common::bundleStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return Ptr(new ParseBundleFieldImpl(n, p));
            }),
        std::make_pair(
            common::stringStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return Ptr(new ParseStringFieldImpl(n, p));
            }),
        std::make_pair(
            common::dataStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return Ptr(new ParseDataFieldImpl(n, p));
            }),
        std::make_pair(
            common::listStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return Ptr(new ParseListFieldImpl(n, p));
            }),
        std::make_pair(
            common::refStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return Ptr(new ParseRefFieldImpl(n, p));
            }),
        std::make_pair(
            common::optionalStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return Ptr(new ParseOptionalFieldImpl(n, p));
            }),
        std::make_pair(
            common::variantStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return Ptr(new ParseVariantFieldImpl(n, p));
            })
    };

    return Map;
}

bool ParseFieldImpl::parseVerifyName() const
{
    if (m_state.m_name.empty()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Missing value for mandatory property \"" << common::nameStr() << "\" for \"" << m_node->name << "\" element.";
        return false;
    }

    if (!common::isValidName(m_state.m_name)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Invalid value for name property \"" << m_state.m_name << "\".";
        return false;
    }

    return true;
}

} // namespace parse

} // namespace commsdsl
