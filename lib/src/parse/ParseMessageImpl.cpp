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

#include "ParseMessageImpl.h"

#include "ParseNamespaceImpl.h"
#include "ParseOptionalFieldImpl.h"
#include "ParseProtocolImpl.h"
#include "ParseXmlWrap.h"
#include "parse_common.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <numeric>
#include <set>

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseXmlWrap::ParseNamesList& parseMessageSupportedTypes()
{
    static const ParseXmlWrap::ParseNamesList Names = ParseFieldImpl::parseSupportedTypes();
    return Names;
}

bool parseVerifyConstructInternal(const ParseOptCond& cond)
{
    if (cond.parseKind() == ParseOptCond::ParseKind::Expr) {
        ParseOptCondExpr exprCond(cond);

        if (exprCond.parseLeft().empty()) {
            assert(exprCond.parseOp().empty() || (exprCond.parseOp() == "!"));
            return true;
        }

        return exprCond.parseOp() == "=";
    }

    assert (cond.parseKind() == ParseOptCond::ParseKind::List);
    ParseOptCondList listCond(cond);
    if (listCond.parseType() != ParseOptCondList::ParseType::And) {
        return false;
    }

    auto conditions = listCond.parseConditions();
    if (conditions.empty()) {
        return false;
    }

    return 
        std::all_of(
            conditions.begin(), conditions.end(),
            [](const auto& c)
            {
                return parseVerifyConstructInternal(c);
            });
}

} // namespace

ParseMessageImpl::ParseMessageImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : m_node(node),
    m_protocol(protocol)
{
}

bool ParseMessageImpl::parse()
{
    m_props = ParseXmlWrap::parseNodeProps(m_node);

    if (!ParseXmlWrap::parseChildrenAsProps(m_node, parseCommonProps(), m_protocol.parseLogger(), m_props)) {
        return false;
    }

    if (!ParseXmlWrap::parseChildrenAsProps(m_node, parseExtraProps(), m_protocol.parseLogger(), m_props, false)) {
        return false;
    }    

    return
        parseCheckReuse() &&
        parseUpdateName() &&
        parseUpdateDisplayName() &&
        parseUpdateDescription() &&
        parseUpdateId() &&
        parseUpdateOrder() &&
        parseUpdateVersions() &&
        parseUpdatePlatforms() &&
        parseUpdateCustomizable() &&
        parseUpdateSender() &&
        parseUpdateValidateMinLength() &&
        parseUpdateFailOnInvalid() &&
        parseCopyFields() &&
        parseReplaceFields() &&
        parseUpdateFields() &&
        parseCopyAliases() &&
        parseUpdateAliases() &&
        parseUpdateReadOverride() &&
        parseUpdateWriteOverride() &&
        parseUpdateRefreshOverride() &&
        parseUpdateLengthOverride() &&
        parseUpdateValidOverride() &&
        parseUpdateNameOverride() &&   
        parseUpdateCopyOverrideCodeFrom() && 
        parseCopyConstruct() && 
        parseCopyReadCond() &&  
        parseCopyValidCond() &&
        parseUpdateSingleConstruct() &&
        parseUpdateMultiConstruct() && 
        parseUpdateSingleReadCond() &&
        parseUpdateMultiReadCond() && 
        parseUpdateSingleValidCond() &&
        parseUpdateMultiValidCond() &&         
        parseCopyConstructToReadCond() &&
        parseCopyConstructToValidCond() &&
        parseUpdateExtraAttrs() &&
        parseUpdateExtraChildren();
}

const std::string& ParseMessageImpl::parseName() const
{
    return m_state.m_name;
}

const std::string& ParseMessageImpl::parseDisplayName() const
{
    return m_state.m_displayName;
}

const std::string& ParseMessageImpl::parseDescription() const
{
    return m_state.m_description;
}

std::size_t ParseMessageImpl::parseMinLength() const
{
    return
        std::accumulate(
            m_state.m_fields.begin(), m_state.m_fields.end(), static_cast<std::size_t>(0U),
                [this](std::size_t soFar, auto& elem) -> std::size_t
                {
                    if (this->parseGetSinceVersion() < elem->parseGetSinceVersion()) {
                        return soFar;
                    }

                    return soFar + elem->parseMinLength();
                });
}

std::size_t ParseMessageImpl::parseMaxLength() const
{
    std::size_t soFar = 0U;
    for (auto& f : m_state.m_fields) {
        common::parseAddToLength(f->parseMaxLength(), soFar);
    }
    return soFar;
}

ParseMessageImpl::ParseFieldsList ParseMessageImpl::parseFieldsList() const
{
    ParseFieldsList result;
    result.reserve(m_state.m_fields.size());
    std::transform(
        m_state.m_fields.begin(), m_state.m_fields.end(), std::back_inserter(result),
        [](auto& f)
        {
            return ParseField(f.get());
        });
    return result;
}

ParseMessageImpl::ParseAliasesList ParseMessageImpl::parseAliasesList() const
{
    ParseAliasesList result;
    result.reserve(m_state.m_aliases.size());
    std::transform(
        m_state.m_aliases.begin(), m_state.m_aliases.end(), std::back_inserter(result),
        [](auto& a)
        {
            return ParseAlias(a.get());
        });
    return result;
}

std::string ParseMessageImpl::parseExternalRef(bool schemaRef) const
{
    assert(parseGetParent() != nullptr);
    assert(parseGetParent()->parseObjKind() == ParseObjKind::Namespace);

    auto& ns = static_cast<const ParseNamespaceImpl&>(*parseGetParent());
    auto nsRef = ns.parseExternalRef(schemaRef);
    if (nsRef.empty()) {
        return parseName();
    }

    return nsRef + '.' + parseName();
}

ParseObject::ParseObjKind ParseMessageImpl::parseObjKindImpl() const
{
    return ParseObjKind::Message;
}

ParseLogWrapper ParseMessageImpl::parseLogError() const
{
    return commsdsl::parse::parseLogError(m_protocol.parseLogger());
}

ParseLogWrapper ParseMessageImpl::parseLogWarning() const
{
    return commsdsl::parse::parseLogWarning(m_protocol.parseLogger());
}

ParseLogWrapper ParseMessageImpl::parseLogInfo() const
{
    return commsdsl::parse::parseLogInfo(m_protocol.parseLogger());
}

bool ParseMessageImpl::parseValidateSinglePropInstance(const std::string& str, bool mustHave)
{
    return ParseXmlWrap::parseValidateSinglePropInstance(m_node, m_props, str, m_protocol.parseLogger(), mustHave);
}

bool ParseMessageImpl::parseValidateAndUpdateStringPropValue(
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

    if (!m_protocol.parseStrToStringValue(iter->second, value)) {
        parseReportUnexpectedPropertyValue(str, iter->second);
        return false;
    }

    return true;
}

bool ParseMessageImpl::parseValidateAndUpdateOverrideTypePropValue(const std::string& propName, ParseOverrideType& value)
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

    auto valIter = Map.find(common::parseToLowerCopy(iter->second));
    if (valIter == Map.end()) {
        parseReportUnexpectedPropertyValue(propName, iter->second);
        return false;        
    }

    value = valIter->second;
    return true;
}

bool ParseMessageImpl::parseValidateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave)
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
            "Property \"" << common::parseAvailableLengthLimitStr() << "\" is not available for dslVersion= " << m_protocol.parseCurrSchema().parseDslVersion();                
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

void ParseMessageImpl::parseReportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue)
{
    ParseXmlWrap::parseReportUnexpectedPropertyValue(m_node, common::parseMessageStr(), propName, propValue, m_protocol.parseLogger());
}

const ParseXmlWrap::ParseNamesList& ParseMessageImpl::parseCommonProps()
{
    static const ParseXmlWrap::ParseNamesList CommonNames = {
        common::parseNameStr(),
        common::parseIdStr(),
        common::parseDisplayNameStr(),
        common::parseDescriptionStr(),
        common::parseSinceVersionStr(),
        common::parseDeprecatedStr(),
        common::parseRemovedStr(),
        common::parseCopyFieldsFromStr(),
        common::parseCopyFieldsAliasesStr(),
        common::parseOrderStr(),
        common::parsePlatformsStr(),
        common::parseCustomizableStr(),
        common::parseSenderStr(),
        common::parseValidateMinLengthStr(),
        common::parseReadOverrideStr(),
        common::parseWriteOverrideStr(),
        common::parseRefreshOverrideStr(),
        common::parseLengthOverrideStr(),
        common::parseValidOverrideStr(),
        common::parseNameOverrideStr(),
        common::parseCopyCodeFromStr(),
        common::parseConstructAsReadCondStr(),
        common::parseConstructAsValidCondStr(),
        common::parseFailOnInvalidStr(),
        common::parseReuseStr(),
        common::parseReuseCodeStr(),
        common::parseReuseAliasesStr(),
        common::parseCopyConstructFromStr(),
        common::parseCopyReadCondFromStr(),
        common::parseCopyValidCondFromStr(),
    };

    return CommonNames;
}

const ParseXmlWrap::ParseNamesList& ParseMessageImpl::parseExtraProps()
{
    static const ParseXmlWrap::ParseNamesList Names = {
        common::parseConstructStr(),
        common::parseReadCondStr(),
        common::parseValidCondStr(),
    };

    return Names;
}

const ParseXmlWrap::ParseNamesList& ParseMessageImpl::parseAllProps()
{
    static ParseXmlWrap::ParseNamesList Names;
    if (Names.empty()) {
        Names = parseCommonProps();
        auto& extras = parseExtraProps();
        Names.insert(Names.end(), extras.begin(), extras.end());
    }

    return Names;
}

ParseXmlWrap::ParseNamesList ParseMessageImpl::parseAllNames()
{
    auto names = parseAllProps();
    auto& fieldTypes = parseMessageSupportedTypes();
    names.insert(names.end(), fieldTypes.begin(), fieldTypes.end());

    names.push_back(common::parseFieldsStr());
    names.push_back(common::parseAliasStr());
    names.push_back(common::parseReplaceStr());
    return names;
}

bool ParseMessageImpl::parseCheckReuse()
{
    auto& propStr = common::parseReuseStr();
    if (!parseValidateSinglePropInstance(propStr)) {
        return false;
    }

    auto iter = m_props.find(propStr);
    if (iter == m_props.end()) {
        return true;
    }

    if (!m_protocol.parseIsMessageReuseSupported()) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Property \"" << propStr << "\" is not supported for <message> in DSL version " << m_protocol.parseCurrSchema().parseDslVersion() << ", ignoring...";
        return true;
    }

    auto& valueStr = iter->second;
    auto* msg = m_protocol.parseFindMessage(valueStr);
    if (msg == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "The message \"" << valueStr << "\" hasn't been recorded yet.";
        return false;
    }

    assert(msg != this);
    Base::parseReuseState(*msg);
    m_state = msg->m_state;

    do {
        bool reuseAliases = true;
        if (!parseValidateAndUpdateBoolPropValue(common::parseReuseAliasesStr(), reuseAliases)) {
            return false;
        }

        if (reuseAliases) {
            break;
        }

        m_state.m_aliases.clear();
    } while (false);    

    do {
        auto& codeProp = common::parseReuseCodeStr();
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

    return true;
}

bool ParseMessageImpl::parseUpdateName()
{
    bool mustHave = m_state.m_name.empty();
    if (!parseValidateAndUpdateStringPropValue(common::parseNameStr(), m_state.m_name, mustHave)) {
        return false;
    }

    if (!common::parseIsValidName(m_state.m_name)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Invalid value for name property \"" << m_state.m_name << "\".";
        return false;
    }

    return true;
}

bool ParseMessageImpl::parseUpdateDescription()
{
    return parseValidateAndUpdateStringPropValue(common::parseDescriptionStr(), m_state.m_description, false, true);
}

bool ParseMessageImpl::parseUpdateDisplayName()
{
    return parseValidateAndUpdateStringPropValue(common::parseDisplayNameStr(), m_state.m_displayName, false, true);
}

bool ParseMessageImpl::parseUpdateId()
{
    if (!parseValidateSinglePropInstance(common::parseIdStr(), true)) {
        return false;
    }

    auto iter = m_props.find(common::parseIdStr());
    std::intmax_t val = 0;
    if (m_protocol.parseStrToEnumValue(iter->second, val)) {
        m_state.m_id = static_cast<decltype(m_state.m_id)>(val);
        return true;
    }

    bool ok = false;
    m_state.m_id = common::parseStrToUintMax(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::parseIdStr(), iter->second);
        return false;
    }

    return true;
}

bool ParseMessageImpl::parseUpdateOrder()
{
    if (!parseValidateSinglePropInstance(common::parseOrderStr())) {
        return false;
    }

    auto iter = m_props.find(common::parseOrderStr());
    if (iter == m_props.end()) {
        assert(m_state.m_order == 0U);
        return true;
    }

    bool ok = false;
    m_state.m_order = common::parseStrToUnsigned(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::parseOrderStr(), iter->second);
        return false;
    }

    return true;
}

bool ParseMessageImpl::parseUpdateVersions()
{
    if (!parseValidateSinglePropInstance(common::parseSinceVersionStr())) {
        return false;
    }

    if (!parseValidateSinglePropInstance(common::parseDeprecatedStr())) {
        return false;
    }

    if (!parseValidateSinglePropInstance(common::parseRemovedStr())) {
        return false;
    }

    assert(parseGetParent() != nullptr);
    assert(parseGetParent()->parseObjKind() == ParseObjKind::Namespace);

    unsigned sinceVersion = 0U;
    unsigned deprecated = ParseProtocol::parseNotYetDeprecated();
    if (!ParseXmlWrap::parseGetAndCheckVersions(m_node, parseName(), m_props, sinceVersion, deprecated, m_protocol)) {
        return false;
    }

    bool deprecatedRemoved = false;
    do {
        auto deprecatedRemovedIter = m_props.find(common::parseRemovedStr());
        if (deprecatedRemovedIter == m_props.end()) {
            break;
        }

        bool ok = false;
        deprecatedRemoved = common::parseStrToBool(deprecatedRemovedIter->second, &ok);
        if (!ok) {
            parseReportUnexpectedPropertyValue(common::parseRemovedStr(), deprecatedRemovedIter->second);
            return false;
        }

        if (!deprecatedRemoved) {
            break;
        }

        if (deprecated == ParseProtocol::parseNotYetDeprecated()) {
            parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Property \"" << common::parseRemovedStr() << "\" is not applicable to "
                "non deprecated fields";
        }
    } while (false);

    parseSetSinceVersion(sinceVersion);
    parseSetDeprecated(deprecated);
    parseSetDeprecatedRemoved(deprecatedRemoved);
    return true;
}

bool ParseMessageImpl::parseUpdatePlatforms()
{
    if (!parseValidateSinglePropInstance(common::parsePlatformsStr())) {
        return false;
    }

    auto iter = m_props.find(common::parsePlatformsStr());
    if (iter == m_props.end()) {
        assert(m_state.m_platforms.empty());
        return true;
    }

    if (iter->second.empty()) {
        parseReportUnexpectedPropertyValue(common::parsePlatformsStr(), iter->second);
        return false;
    }

    auto op = iter->second[0];
    static const char Plus = '+';
    static const char Minus = '-';
    if ((op != Plus) && (op != Minus)) {
        parseReportUnexpectedPropertyValue(common::parsePlatformsStr(), iter->second);
        return false;
    }

    static const char Sep = ',';
    ParsePlatformsList platList;
    std::size_t pos = 1U;
    while (true) {
        if (iter->second.size() <= pos) {
            break;
        }

        auto nextSep = iter->second.find_first_of(Sep, pos);
        platList.emplace_back(iter->second , pos, nextSep - pos);
        if (nextSep == std::string::npos) {
            break;
        }

        pos = nextSep + 1;
    }

    if (platList.empty()) {
        parseReportUnexpectedPropertyValue(common::parsePlatformsStr(), iter->second);
        return false;
    }

    auto& allPlatforms = m_protocol.parseCurrSchema().parsePlatforms();
    for (auto& p : platList) {
        common::parseRemoveHeadingTrailingWhitespaces(p);
        if (p.empty()) {
            parseReportUnexpectedPropertyValue(common::parsePlatformsStr(), iter->second);
            return false;
        }

        auto platIter = std::lower_bound(allPlatforms.begin(), allPlatforms.end(), p);
        if ((platIter == allPlatforms.end()) || (*platIter != p)) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
                "Platform \"" << p << "\" hasn't been defined.";
            return false;
        }
    }

    // sort and erase duplicates
    std::sort(platList.begin(), platList.end());
    platList.erase(std::unique(platList.begin(), platList.end()), platList.end());

    if (op == Plus) {
        m_state.m_platforms = std::move(platList);
        return true;
    }

    assert(op == Minus);
    assert(platList.size() <= allPlatforms.size());
    m_state.m_platforms.reserve(allPlatforms.size() - platList.size());
    std::set_difference(
        allPlatforms.begin(), allPlatforms.end(),
        platList.begin(), platList.end(),
        std::back_inserter(m_state.m_platforms));

    if (m_state.m_platforms.empty()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Message \"" << parseName() << "\" is not supported in any platform.";
        return false;
    }

    return true;
}

bool ParseMessageImpl::parseUpdateCustomizable()
{
    return parseValidateAndUpdateBoolPropValue(common::parseCustomizableStr(), m_state.m_customizable);
}

bool ParseMessageImpl::parseUpdateSender()
{
    auto& propStr = common::parseSenderStr();
    if (!parseValidateSinglePropInstance(propStr)) {
        return false;
    }

    auto iter = m_props.find(propStr);
    if (iter == m_props.end()) {
        return true;
    }

    static const std::string Map[] = {
        common::parseBothStr(),
        common::parseClientStr(),
        common::parseServerStr()
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == static_cast<unsigned>(ParseSender::NumOfValues), "Invalid map");

    auto senderIter = std::find(std::begin(Map), std::end(Map), common::parseToLowerCopy(iter->second));
    if (senderIter == std::end(Map)) {
        parseReportUnexpectedPropertyValue(propStr, iter->second);
        return false;
    }

    m_state.m_sender = static_cast<decltype(m_state.m_sender)>(std::distance(std::begin(Map), senderIter));
    return true;
}

bool ParseMessageImpl::parseUpdateValidateMinLength()
{
    auto& propStr = common::parseValidateMinLengthStr();
    if (!parseValidateSinglePropInstance(propStr)) {
        return false;
    }

    auto iter = m_props.find(propStr);
    if (iter == m_props.end()) {
        return true;
    }

    if (!m_protocol.parseIsPropertySupported(propStr)) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Property \"" << propStr << "\" is not supported for DSL version " << m_protocol.parseCurrSchema().parseDslVersion() << ", ignoring...";
        return true;
    }

    bool ok = false;
    m_state.m_validateMinLength = static_cast<decltype(m_state.m_validateMinLength)>(common::parseStrToUnsigned(iter->second, &ok));
    if (!ok) {
        parseReportUnexpectedPropertyValue(propStr, iter->second);
        return false;
    }    
    return true;
}

bool ParseMessageImpl::parseUpdateFailOnInvalid()
{
    auto& propStr = common::parseFailOnInvalidStr();
    if (!parseValidateAndUpdateBoolPropValue(propStr, m_state.m_failOnInvalid)) {
        return false;
    }

    if (m_state.m_failOnInvalid && (!m_protocol.parseIsFailOnInvalidInMessageSupported())) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Property \"" << propStr << "\" is not supported for DSL version " << m_protocol.parseCurrSchema().parseDslVersion() << ", ignoring...";
        m_state.m_failOnInvalid = false;
        return true;
    }

    return true;
}

bool ParseMessageImpl::parseCopyFields()
{
    if (!parseValidateSinglePropInstance(common::parseCopyFieldsFromStr())) {
        return false;
    }

    auto iter = m_props.find(common::parseCopyFieldsFromStr());
    if (iter == m_props.end()) {
        return true;
    }

    if (!m_state.m_fields.empty()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Copying fields from multiple sources using various properties is not supported";
        return false;
    }
    
    do {
        m_copyFieldsFromMsg = m_protocol.parseFindMessage(iter->second);
        if (m_copyFieldsFromMsg != nullptr) {
            parseCloneFieldsFrom(*m_copyFieldsFromMsg);
            break;
        }

        if (!m_protocol.parseIsCopyFieldsFromBundleSupported()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Invalid reference to other message \"" << iter->second << "\".";
            return false;            
        }

        auto* copyFromField = m_protocol.parseFindField(iter->second);
        if ((copyFromField == nullptr) || (copyFromField->parseKind() != ParseField::ParseKind::Bundle)) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Invalid reference to other message or bundle \"" << iter->second << "\".";
            return false;
        }

        m_copyFieldsFromBundle = static_cast<const ParseBundleFieldImpl*>(copyFromField);
        parseCloneFieldsFrom(*m_copyFieldsFromBundle);
    } while (false);

    if (!m_state.m_fields.empty()) {
        m_state.m_fields.erase(
            std::remove_if(
                m_state.m_fields.begin(), m_state.m_fields.end(),
                [this](auto& elem)
                {
                    return
                        (elem->parseIsDeprecatedRemoved()) &&
                        (elem->parseGetDeprecated() <= this->parseGetSinceVersion());
                }),
            m_state.m_fields.end());

        for (auto& m : m_state.m_fields) {
            m->parseSetSinceVersion(std::max(parseGetSinceVersion(), m->parseGetSinceVersion()));
        }
    }
    return true;
}

bool ParseMessageImpl::parseReplaceFields()
{
    auto replaceNodes = ParseXmlWrap::parseGetChildren(parseGetNode(), common::parseReplaceStr());
    if (1U < replaceNodes.size()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Only single \"" << common::parseReplaceStr() << "\" child element is "
            "supported for \"" << common::parseMessageStr() << "\".";
        return false;
    }

    if (replaceNodes.empty()) {
        return true;
    }

    if (!m_protocol.parseIsMemberReplaceSupported()) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Replacing fields with \"" << common::parseReplaceStr() << "\" child element is unavaliable "
            "for selected DSL version, ignoring...";        
        return true;
    }    

    auto fieldsTypes = ParseXmlWrap::parseGetChildren(replaceNodes.front(), parseMessageSupportedTypes());
    if (fieldsTypes.size() != replaceNodes.size()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(replaceNodes.front()) <<
            "The \"" << common::parseReplaceStr() << "\" child node of \"" <<
            common::parseMessageStr() << "\" element must contain only supported field types.";
        return false;
    }    

    ParseFieldImpl::ParseFieldsList replMembers;
    replMembers.reserve(fieldsTypes.size());
    for (auto* fieldNode : fieldsTypes) {
        std::string memKind(reinterpret_cast<const char*>(fieldNode->name));
        auto field = ParseFieldImpl::parseCreate(memKind, fieldNode, m_protocol);
        if (!field) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            parseLogError() << ParseXmlWrap::parseLogPrefix(replaceNodes.front()) <<
                "Internal error, failed to create objects for fields to replace.";
            return false;
        }

        field->parseSetParent(this);
        if (!field->parse()) {
            return false;
        }

        if (!field->parseVerifySiblings(m_state.m_fields)) {
            return false;
        }        

        replMembers.push_back(std::move(field));
    }   

    for (auto& field : replMembers) {
        assert(field);
        auto iter = 
            std::find_if(
                m_state.m_fields.begin(), m_state.m_fields.end(),
                [&field](auto& currField)
                {
                    assert(currField);
                    return field->parseName() == currField->parseName();
                });

        if (iter == m_state.m_fields.end()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(field->parseGetNode()) <<
                "Cannot find reused field with name \"" << field->parseName() << "\" to replace.";
            return false;
        }

        (*iter) = std::move(field);
    }

    return true;       
}

bool ParseMessageImpl::parseCopyAliases()
{
    auto& propStr = common::parseCopyFieldsAliasesStr();
    if (!parseValidateSinglePropInstance(propStr)) {
        return false;
    }

    auto iter = m_props.find(propStr);
    if (iter != m_props.end() && (!m_protocol.parseIsFieldAliasSupported())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Unexpected property \"" << propStr << "\".";
        return false;
    }

    if (!m_protocol.parseIsFieldAliasSupported()) {
        return true;
    }

    bool copyAliases = true;
    if (iter != m_props.end()) {
        bool ok = false;
        copyAliases = common::parseStrToBool(iter->second, &ok);
        if (!ok) {
            parseReportUnexpectedPropertyValue(propStr, iter->second);
            return false;
        }
    }

    if (!copyAliases) {
        return true;
    }

    if ((iter != m_props.end()) && (m_copyFieldsFromMsg == nullptr) && (m_copyFieldsFromBundle == nullptr)) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Property \"" << propStr << "\" is inapplicable without \"" << common::parseCopyFieldsFromStr() << "\".";
        return true;
    }

    if (m_copyFieldsFromMsg != nullptr) {
        parseCloneAliasesFrom(*m_copyFieldsFromMsg);
    }
    else if (m_copyFieldsFromBundle != nullptr) {
        parseCloneAliasesFrom(*m_copyFieldsFromBundle);
    }

    if (!m_state.m_aliases.empty()) {
        m_state.m_aliases.erase(
            std::remove_if(
                m_state.m_aliases.begin(), m_state.m_aliases.end(),
                [this](auto& alias)
                {
                    auto& fieldName = alias->parseFieldName();
                    assert(!fieldName.empty());
                    auto fieldIter =
                        std::find_if(
                            m_state.m_fields.begin(), m_state.m_fields.end(),
                            [&fieldName](auto& f)
                            {
                                return fieldName == f->parseName();
                            });

                    return fieldIter == m_state.m_fields.end();
                }),
            m_state.m_aliases.end());
    }
    return true;
}

bool ParseMessageImpl::parseUpdateFields()
{
    do {
        auto fieldsNodes = ParseXmlWrap::parseGetChildren(parseGetNode(), common::parseFieldsStr());
        if (1U < fieldsNodes.size()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                          "Only single \"" << common::parseFieldsStr() << "\" child element is "
                          "supported for \"" << common::parseMessageStr() << "\".";
            return false;
        }

        auto fieldsTypes = ParseXmlWrap::parseGetChildren(parseGetNode(), parseMessageSupportedTypes());
        if ((!fieldsNodes.empty()) && (!fieldsTypes.empty())) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                          "The \"" << common::parseMessageStr() << "\" element does not support "
                          "list of stand alone fields as child elements together with \"" <<
                          common::parseFieldsStr() << "\" child element.";
            return false;
        }

        if ((fieldsNodes.empty()) && (fieldsTypes.empty())) {
            break;
        }

        if ((0U < fieldsTypes.size())) {
            assert(0U == fieldsNodes.size());
            auto allChildren = ParseXmlWrap::parseGetChildren(parseGetNode());
            if (allChildren.size() != fieldsTypes.size()) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                              "The field types of \"" << common::parseMessageStr() <<
                              "\" must be defined inside \"<" << common::parseFieldsStr() << ">\" child element "
                              "when there are other property describing children.";
                return false;
            }
        }

        if (0U < fieldsNodes.size()) {
            assert(0U == fieldsTypes.size());
            fieldsTypes = ParseXmlWrap::parseGetChildren(fieldsNodes.front());
            auto cleanMemberFieldsTypes = ParseXmlWrap::parseGetChildren(fieldsNodes.front(), parseMessageSupportedTypes());
            if (cleanMemberFieldsTypes.size() != fieldsTypes.size()) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(fieldsNodes.front()) <<
                    "The \"" << common::parseFieldsStr() << "\" child node of \"" <<
                    common::parseMessageStr() << "\" element must contain only supported field types.";
                return false;
            }

            // fieldsTypes is updated with the list from <fields>
        }

        m_state.m_fields.reserve(m_state.m_fields.size() + fieldsTypes.size());
        for (auto* fNode : fieldsTypes) {
            std::string fKind(reinterpret_cast<const char*>(fNode->name));
            auto field = ParseFieldImpl::parseCreate(fKind, fNode, m_protocol);
            if (!field) {
                [[maybe_unused]] static constexpr bool Should_not_happen = false;
                assert(Should_not_happen);
                parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Internal error, failed to create objects for member fields.";
                return false;
            }

            field->parseSetParent(this);
            if (!field->parse()) {
                return false;
            }

            if (!field->parseVerifySiblings(m_state.m_fields)) {
                return false;
            }

            m_state.m_fields.push_back(std::move(field));
        }

        if (!ParseFieldImpl::parseValidateMembersNames(m_state.m_fields, m_protocol.parseLogger())) {
            return false;
        }

        if (0 <= m_state.m_validateMinLength) {
            auto len = parseMinLength();
            if (static_cast<unsigned>(m_state.m_validateMinLength) != len) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                    "The calculated minimal length of the message is " << len <<
                    " while expected is " << m_state.m_validateMinLength << " (specified with \"" << common::parseValidateMinLengthStr() << "\" property).";                
                return false;
            }
        }

    } while (false);

    return true;
}

bool ParseMessageImpl::parseUpdateAliases()
{
    auto aliasNodes = ParseXmlWrap::parseGetChildren(parseGetNode(), common::parseAliasStr());

    if (aliasNodes.empty()) {
        return true;
    }

    if (!m_protocol.parseIsFieldAliasSupported()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(aliasNodes.front()) <<
              "Using \"" << common::parseAliasStr() << "\" nodes for too early \"" <<
              common::parseDslVersionStr() << "\".";
        return false;
    }

    m_state.m_aliases.reserve(m_state.m_aliases.size() + aliasNodes.size());
    for (auto* aNode : aliasNodes) {
        auto alias = ParseAliasImpl::parseCreate(aNode, m_protocol);
        if (!alias) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            parseLogError() << ParseXmlWrap::parseLogPrefix(alias->parseGetNode()) <<
                  "Internal error, failed to create objects for member aliases.";
            return false;
        }

        if (!alias->parse()) {
            return false;
        }

        if (!alias->parseVerifyAlias(m_state.m_aliases, m_state.m_fields)) {
            return false;
        }

        m_state.m_aliases.push_back(std::move(alias));
    }

    return true;
}

void ParseMessageImpl::parseCloneFieldsFrom(const ParseMessageImpl& other)
{
    m_state.m_fields.reserve(other.m_state.m_fields.size());
    for (auto& f : other.m_state.m_fields) {
        m_state.m_fields.push_back(f->parseClone());
    }
}

void ParseMessageImpl::parseCloneFieldsFrom(const ParseBundleFieldImpl& other)
{
    m_state.m_fields.reserve(other.parseMembers().size());
    for (auto& f : other.parseMembers()) {
        m_state.m_fields.push_back(f->parseClone());
    }
}

void ParseMessageImpl::parseCloneAliasesFrom(const ParseMessageImpl& other)
{
    m_state.m_aliases.reserve(other.m_state.m_aliases.size());
    for (auto& a : other.m_state.m_aliases) {
        m_state.m_aliases.push_back(a->parseClone());
    }
}

void ParseMessageImpl::parseCloneAliasesFrom(const ParseBundleFieldImpl& other)
{
    m_state.m_aliases.reserve(other.parseAliases().size());
    for (auto& a : other.parseAliases()) {
        m_state.m_aliases.push_back(a->parseClone());
    }
}

bool ParseMessageImpl::parseUpdateReadOverride()
{
    return parseValidateAndUpdateOverrideTypePropValue(common::parseReadOverrideStr(), m_state.m_readOverride);
}

bool ParseMessageImpl::parseUpdateWriteOverride()
{
    return parseValidateAndUpdateOverrideTypePropValue(common::parseWriteOverrideStr(), m_state.m_writeOverride);
}

bool ParseMessageImpl::parseUpdateRefreshOverride()
{
    return parseValidateAndUpdateOverrideTypePropValue(common::parseRefreshOverrideStr(), m_state.m_refreshOverride);
}

bool ParseMessageImpl::parseUpdateLengthOverride()
{
    return parseValidateAndUpdateOverrideTypePropValue(common::parseLengthOverrideStr(), m_state.m_lengthOverride);
}

bool ParseMessageImpl::parseUpdateValidOverride()
{
    return parseValidateAndUpdateOverrideTypePropValue(common::parseValidOverrideStr(), m_state.m_validOverride);
}

bool ParseMessageImpl::parseUpdateNameOverride()
{
    return parseValidateAndUpdateOverrideTypePropValue(common::parseNameOverrideStr(), m_state.m_nameOverride);
}

bool ParseMessageImpl::parseUpdateCopyOverrideCodeFrom()
{
    auto& prop = common::parseCopyCodeFromStr();
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

    auto* msg = m_protocol.parseFindMessage(iter->second);
    if (msg == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Message referenced by \"" << prop << "\" property (" + iter->second + ") is not found.";
        return false;        
    }

    m_state.m_copyCodeFrom = iter->second;
    return true;
}

bool ParseMessageImpl::parseCopyConstruct()
{
    auto& prop = common::parseCopyConstructFromStr();
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

    auto* msg = m_protocol.parseFindMessage(iter->second);
    if (msg == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Message referenced by \"" << prop << "\" property (" + iter->second + ") is not found.";
        return false;        
    }

    if (!msg->m_state.m_construct) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Message referenced by \"" << prop << "\" property (" + iter->second + ") does not specify construction conditions.";
        return false;        
    }

    auto newConstruct = msg->m_state.m_construct->parseClone();
    if (!newConstruct->parseVerify(ParseOptCondImpl::ParseFieldsList(), m_node, m_protocol)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Copied construct conditions cannot be applied to this message.";
        return false;
    }    

    m_state.m_construct = std::move(newConstruct);
    return true;
}

bool ParseMessageImpl::parseCopyReadCond()
{
    auto& prop = common::parseCopyReadCondFromStr();
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

    auto* msg = m_protocol.parseFindMessage(iter->second);
    if (msg == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Message referenced by \"" << prop << "\" property (" + iter->second + ") is not found.";
        return false;        
    }

    if (!msg->m_state.m_readCond) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Message referenced by \"" << prop << "\" property (" + iter->second + ") does not specify read conditions.";
        return false;        
    }

    auto newReadCond = msg->m_state.m_readCond->parseClone();
    if (!newReadCond->parseVerify(ParseOptCondImpl::ParseFieldsList(), m_node, m_protocol)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Copied read conditions cannot be applied to this message.";
        return false;
    }    

    m_state.m_readCond = std::move(newReadCond);
    return true;
}

bool ParseMessageImpl::parseCopyValidCond()
{
    auto& prop = common::parseCopyValidCondFromStr();
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

    const ParseMessageImpl* msg = nullptr;
    const ParseBundleFieldImpl* bundle = nullptr;
    do {
        msg = m_protocol.parseFindMessage(iter->second);
        if (msg != nullptr) {
            break;
        }

        auto otherField = m_protocol.parseFindField(iter->second);
        if (otherField == nullptr) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
                "Neither message nor bundle field referenced by \"" << prop << "\" property (" + iter->second + ") is not found.";
            return false;
        }

        if (otherField->parseKind() != ParseFieldImpl::ParseKind::Bundle) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
                "The \"" << prop << "\" property (" + iter->second + ") can reference only other message or a bundle field.";
            return false;
        }

        bundle = static_cast<const ParseBundleFieldImpl*>(otherField);
    } while (false);

    assert((msg != nullptr) || (bundle != nullptr));

    const ParseOptCondImpl* srcCondPtr = nullptr;
    if (msg != nullptr) {
        srcCondPtr = msg->m_state.m_validCond.get();
    }
    else if (bundle != nullptr) {
        srcCondPtr = bundle->parseValidCondImpl().get();
    }

    if (srcCondPtr == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Message / bundle referenced by \"" << prop << "\" property (" + iter->second + ") does not specify validity conditions.";
        return false;        
    }

    auto newCond = srcCondPtr->parseClone();
    if (!newCond->parseVerify(m_state.m_fields, m_node, m_protocol)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Copied validity conditions cannot be applied to this message.";
        return false;
    }    

    m_state.m_validCond = std::move(newCond);
    return true;
}

bool ParseMessageImpl::parseUpdateSingleConstruct()
{
    if (!parseUpdateSingleCondInternal(common::parseConstructStr(), m_state.m_construct)) {
        return false;
    }

    if (!m_state.m_construct) {
        return true;
    }

    if (!parseVerifyConstructInternal(ParseOptCond(m_state.m_construct.get()))) {
        m_state.m_construct.reset();
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Only bit checks and equality comparisons are supported in the \"" << common::parseConstructStr() << "\" property.";
        return false;
    }

    return true;
}

bool ParseMessageImpl::parseUpdateMultiConstruct()
{
    if (!parseUpdateMultiCondInternal(common::parseConstructStr(), m_state.m_construct)) {
        return false;
    }

    if (!m_state.m_construct) {
        return true;
    }

    if (!parseVerifyConstructInternal(ParseOptCond(m_state.m_construct.get()))) {
        m_state.m_construct.reset();
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Only \"" << common::parseAndStr() <<  
            "\" of the bit checks and equality comparisons are supported in the \"" << common::parseConstructStr() << "\" element.";
        return false;
    }    

    return true;
}

bool ParseMessageImpl::parseUpdateSingleReadCond()
{
    return parseUpdateSingleCondInternal(common::parseReadCondStr(), m_state.m_readCond);
}

bool ParseMessageImpl::parseUpdateMultiReadCond()
{
    return parseUpdateMultiCondInternal(common::parseReadCondStr(), m_state.m_readCond);
}

bool ParseMessageImpl::parseUpdateSingleValidCond()
{
    return parseUpdateSingleCondInternal(common::parseValidCondStr(), m_state.m_validCond, true);
}

bool ParseMessageImpl::parseUpdateMultiValidCond()
{
    return parseUpdateMultiCondInternal(common::parseValidCondStr(), m_state.m_validCond, true);
}

bool ParseMessageImpl::parseCopyConstructToReadCond()
{
    return 
        parseCopyCondInternal(
            common::parseConstructAsReadCondStr(),
            common::parseConstructStr(),
            m_state.m_construct,
            common::parseReadCondStr(),
            m_state.m_readCond);
}

bool ParseMessageImpl::parseCopyConstructToValidCond()
{
    return 
        parseCopyCondInternal(
            common::parseConstructAsValidCondStr(),
            common::parseConstructStr(),
            m_state.m_construct,
            common::parseValidCondStr(),
            m_state.m_validCond);
}

bool ParseMessageImpl::parseUpdateExtraAttrs()
{
    m_extraAttrs = ParseXmlWrap::parseGetExtraAttributes(m_node, parseAllProps(), m_protocol);
    return true;
}

bool ParseMessageImpl::parseUpdateExtraChildren()
{
    static const ParseXmlWrap::ParseNamesList ChildrenNames = parseAllNames();
    m_extraChildren = ParseXmlWrap::parseGetExtraChildren(m_node, ChildrenNames, m_protocol);
    return true;
}

bool ParseMessageImpl::parseUpdateSingleCondInternal(const std::string& prop, ParseOptCondImplPtr& cond, bool allowFieldsAccess)
{
    if (!parseValidateSinglePropInstance(prop)) {
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

    auto newCond = std::make_unique<ParseOptCondExprImpl>();
    if (!newCond->parse(iter->second, m_node, m_protocol)) {
        return false;
    }

    static const ParseOptCondImpl::ParseFieldsList NoFields;
    auto* fieldsPtr = &NoFields;
    if (allowFieldsAccess) {
        fieldsPtr = &m_state.m_fields;
    }    

    if (!newCond->parseVerify(*fieldsPtr, m_node, m_protocol)) {
        return false;
    }   

    cond = std::move(newCond);
    return true; 
}

bool ParseMessageImpl::parseUpdateMultiCondInternal(const std::string& prop, ParseOptCondImplPtr& cond, bool allowFieldsAccess)
{
    auto condNodes = ParseXmlWrap::parseGetChildren(m_node, prop, true);
    if (condNodes.empty()) {
        return true;
    }

    if (!m_protocol.parseIsPropertySupported(prop)) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "The property \"" << prop << "\" is not supported for dslVersion=" << 
                m_protocol.parseCurrSchema().parseDslVersion() << ".";        
        return true;
    }      

    if (condNodes.size() > 1U) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Cannot use more that one child to the \"" << prop << "\" element.";        
        return false;
    }

    static const ParseXmlWrap::ParseNamesList ElemNames = {
        common::parseAndStr(),
        common::parseOrStr()
    };

    auto condChildren = ParseXmlWrap::parseGetChildren(condNodes.front(), ElemNames);
    if (condChildren.size() != condNodes.size()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Only single \"" << common::parseAndStr() << "\" or \"" << common::parseOrStr() << "\" child of the \"" << prop << "\" element is supported.";           
        return false;
    }    

    auto iter = parseProps().find(prop);
    if (iter != parseProps().end()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(condNodes.front()) <<
            "Only single \"" << prop << "\" property is supported";
        return false;
    }

    auto newCond = std::make_unique<ParseOptCondListImpl>();
    newCond->parseOverrideCondStr(prop);
    if (!newCond->parse(condChildren.front(), m_protocol)) {
        return false;
    }

    static const ParseOptCondImpl::ParseFieldsList NoFields;
    auto* fieldsPtr = &NoFields;
    if (allowFieldsAccess) {
        fieldsPtr = &m_state.m_fields;
    }

    if (!newCond->parseVerify(*fieldsPtr, condChildren.front(), m_protocol)) {
        return false;
    }    

    cond = std::move(newCond);
    return true;
}

bool ParseMessageImpl::parseCopyCondInternal(
    const std::string& copyProp,
    const std::string& fromProp, 
    const ParseOptCondImplPtr& fromCond, 
    const std::string& toProp,
    ParseOptCondImplPtr& toCond,
    bool allowOverride)
{
    if (!parseValidateSinglePropInstance(copyProp)) {
        return false;
    }

    auto iter = m_props.find(copyProp);
    if (iter == m_props.end()) {
        return true;
    }    

    if (!m_protocol.parseIsPropertySupported(copyProp)) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "The property \"" << copyProp << "\" is not supported for dslVersion=" << 
                m_protocol.parseCurrSchema().parseDslVersion() << ".";        
        return true;
    }      

    bool ok = false;
    bool copyRequested = common::parseStrToBool(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(copyProp, iter->second);
        return false;
    }

    if (!copyRequested) {
        return true;
    }

    if (!fromCond) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "No \"" << fromProp << "\" conditions were defined to copy.";           
        return false;            
    }

    if (toCond && (!allowOverride)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Set of the \"" << copyProp << "\" property overrides existing \"" << toProp << "\" setting.";          
        return false;
    }

    toCond = fromCond->parseClone();
    return true;    
}

} // namespace parse

} // namespace commsdsl
