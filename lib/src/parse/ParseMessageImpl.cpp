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

#include <cassert>
#include <limits>
#include <algorithm>
#include <set>
#include <numeric>
#include <iterator>

#include "ParseProtocolImpl.h"
#include "ParseNamespaceImpl.h"
#include "parse_common.h"
#include "ParseOptionalFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseXmlWrap::NamesList& messageSupportedTypes()
{
    static const ParseXmlWrap::NamesList Names = ParseFieldImpl::supportedTypes();
    return Names;
}

bool verifyConstructInternal(const ParseOptCond& cond)
{
    if (cond.kind() == ParseOptCond::Kind::Expr) {
        ParseOptCondExpr exprCond(cond);

        if (exprCond.left().empty()) {
            assert(exprCond.op().empty() || (exprCond.op() == "!"));
            return true;
        }

        return exprCond.op() == "=";
    }

    assert (cond.kind() == ParseOptCond::Kind::List);
    ParseOptCondList listCond(cond);
    if (listCond.type() != ParseOptCondList::Type::And) {
        return false;
    }

    auto conditions = listCond.conditions();
    if (conditions.empty()) {
        return false;
    }

    return 
        std::all_of(
            conditions.begin(), conditions.end(),
            [](const auto& c)
            {
                return verifyConstructInternal(c);
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

    if (!ParseXmlWrap::parseChildrenAsProps(m_node, commonProps(), m_protocol.logger(), m_props)) {
        return false;
    }

    if (!ParseXmlWrap::parseChildrenAsProps(m_node, extraProps(), m_protocol.logger(), m_props, false)) {
        return false;
    }    

    return
        checkReuse() &&
        updateName() &&
        updateDisplayName() &&
        updateDescription() &&
        updateId() &&
        updateOrder() &&
        updateVersions() &&
        updatePlatforms() &&
        updateCustomizable() &&
        updateSender() &&
        updateValidateMinLength() &&
        updateFailOnInvalid() &&
        copyFields() &&
        replaceFields() &&
        updateFields() &&
        copyAliases() &&
        updateAliases() &&
        updateReadOverride() &&
        updateWriteOverride() &&
        updateRefreshOverride() &&
        updateLengthOverride() &&
        updateValidOverride() &&
        updateNameOverride() &&   
        updateCopyOverrideCodeFrom() && 
        copyConstruct() && 
        copyReadCond() &&  
        copyValidCond() &&
        updateSingleConstruct() &&
        updateMultiConstruct() && 
        updateSingleReadCond() &&
        updateMultiReadCond() && 
        updateSingleValidCond() &&
        updateMultiValidCond() &&         
        copyConstructToReadCond() &&
        copyConstructToValidCond() &&
        updateExtraAttrs() &&
        updateExtraChildren();
}

const std::string& ParseMessageImpl::name() const
{
    return m_state.m_name;
}

const std::string& ParseMessageImpl::displayName() const
{
    return m_state.m_displayName;
}

const std::string& ParseMessageImpl::description() const
{
    return m_state.m_description;
}

std::size_t ParseMessageImpl::minLength() const
{
    return
        std::accumulate(
            m_state.m_fields.begin(), m_state.m_fields.end(), static_cast<std::size_t>(0U),
                [this](std::size_t soFar, auto& elem) -> std::size_t
                {
                    if (this->getSinceVersion() < elem->getSinceVersion()) {
                        return soFar;
                    }

                    return soFar + elem->minLength();
                });
}

std::size_t ParseMessageImpl::maxLength() const
{
    std::size_t soFar = 0U;
    for (auto& f : m_state.m_fields) {
        common::addToLength(f->maxLength(), soFar);
    }
    return soFar;
}

ParseMessageImpl::FieldsList ParseMessageImpl::fieldsList() const
{
    FieldsList result;
    result.reserve(m_state.m_fields.size());
    std::transform(
        m_state.m_fields.begin(), m_state.m_fields.end(), std::back_inserter(result),
        [](auto& f)
        {
            return ParseField(f.get());
        });
    return result;
}

ParseMessageImpl::AliasesList ParseMessageImpl::aliasesList() const
{
    AliasesList result;
    result.reserve(m_state.m_aliases.size());
    std::transform(
        m_state.m_aliases.begin(), m_state.m_aliases.end(), std::back_inserter(result),
        [](auto& a)
        {
            return ParseAlias(a.get());
        });
    return result;
}

std::string ParseMessageImpl::externalRef(bool schemaRef) const
{
    assert(getParent() != nullptr);
    assert(getParent()->objKind() == ObjKind::Namespace);

    auto& ns = static_cast<const ParseNamespaceImpl&>(*getParent());
    auto nsRef = ns.externalRef(schemaRef);
    if (nsRef.empty()) {
        return name();
    }

    return nsRef + '.' + name();
}

ParseObject::ObjKind ParseMessageImpl::objKindImpl() const
{
    return ObjKind::Message;
}

LogWrapper ParseMessageImpl::logError() const
{
    return commsdsl::parse::logError(m_protocol.logger());
}

LogWrapper ParseMessageImpl::logWarning() const
{
    return commsdsl::parse::logWarning(m_protocol.logger());
}

LogWrapper ParseMessageImpl::logInfo() const
{
    return commsdsl::parse::logInfo(m_protocol.logger());
}

bool ParseMessageImpl::validateSinglePropInstance(const std::string& str, bool mustHave)
{
    return ParseXmlWrap::validateSinglePropInstance(m_node, m_props, str, m_protocol.logger(), mustHave);
}

bool ParseMessageImpl::validateAndUpdateStringPropValue(
    const std::string& str,
    std::string& value,
    bool mustHave,
    bool allowDeref)
{
    if (!validateSinglePropInstance(str, mustHave)) {
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

    if (!m_protocol.strToStringValue(iter->second, value)) {
        reportUnexpectedPropertyValue(str, iter->second);
        return false;
    }

    return true;
}

bool ParseMessageImpl::validateAndUpdateOverrideTypePropValue(const std::string& propName, ParseOverrideType& value)
{
    if (!validateSinglePropInstance(propName, false)) {
        return false;
    }

    auto iter = m_props.find(propName);
    if (iter == m_props.end()) {
        value = ParseOverrideType_Any;
        return true;
    }    

    if (!m_protocol.isOverrideTypeSupported()) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
            "The property \"" << propName << "\" is not supported for dslVersion=" << 
                m_protocol.currSchema().dslVersion() << ".";        
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
        reportUnexpectedPropertyValue(propName, iter->second);
        return false;        
    }

    value = valIter->second;
    return true;
}

bool ParseMessageImpl::validateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave)
{
    if (!validateSinglePropInstance(propName, mustHave)) {
        return false;
    }

    auto iter = m_props.find(propName);
    if (iter == m_props.end()) {
        return true;
    }

    if (!m_protocol.isPropertySupported(propName)) {
        logWarning() << ParseXmlWrap::logPrefix(m_node) <<
            "Property \"" << common::availableLengthLimitStr() << "\" is not available for dslVersion= " << m_protocol.currSchema().dslVersion();                
        return true;
    }

    bool ok = false;
    value = common::strToBool(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(propName, iter->second);
        return false;
    }

    return true;
}

void ParseMessageImpl::reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue)
{
    ParseXmlWrap::reportUnexpectedPropertyValue(m_node, common::messageStr(), propName, propValue, m_protocol.logger());
}

const ParseXmlWrap::NamesList& ParseMessageImpl::commonProps()
{
    static const ParseXmlWrap::NamesList CommonNames = {
        common::nameStr(),
        common::idStr(),
        common::displayNameStr(),
        common::descriptionStr(),
        common::sinceVersionStr(),
        common::deprecatedStr(),
        common::removedStr(),
        common::copyFieldsFromStr(),
        common::copyFieldsAliasesStr(),
        common::orderStr(),
        common::platformsStr(),
        common::customizableStr(),
        common::senderStr(),
        common::validateMinLengthStr(),
        common::readOverrideStr(),
        common::writeOverrideStr(),
        common::refreshOverrideStr(),
        common::lengthOverrideStr(),
        common::validOverrideStr(),
        common::nameOverrideStr(),
        common::copyCodeFromStr(),
        common::constructAsReadCondStr(),
        common::constructAsValidCondStr(),
        common::failOnInvalidStr(),
        common::reuseStr(),
        common::reuseCodeStr(),
        common::reuseAliasesStr(),
        common::copyConstructFromStr(),
        common::copyReadCondFromStr(),
        common::copyValidCondFromStr(),
    };

    return CommonNames;
}

const ParseXmlWrap::NamesList& ParseMessageImpl::extraProps()
{
    static const ParseXmlWrap::NamesList Names = {
        common::constructStr(),
        common::readCondStr(),
        common::validCondStr(),
    };

    return Names;
}

const ParseXmlWrap::NamesList& ParseMessageImpl::allProps()
{
    static ParseXmlWrap::NamesList Names;
    if (Names.empty()) {
        Names = commonProps();
        auto& extras = extraProps();
        Names.insert(Names.end(), extras.begin(), extras.end());
    }

    return Names;
}

ParseXmlWrap::NamesList ParseMessageImpl::allNames()
{
    auto names = allProps();
    auto& fieldTypes = messageSupportedTypes();
    names.insert(names.end(), fieldTypes.begin(), fieldTypes.end());

    names.push_back(common::fieldsStr());
    names.push_back(common::aliasStr());
    names.push_back(common::replaceStr());
    return names;
}

bool ParseMessageImpl::checkReuse()
{
    auto& propStr = common::reuseStr();
    if (!validateSinglePropInstance(propStr)) {
        return false;
    }

    auto iter = m_props.find(propStr);
    if (iter == m_props.end()) {
        return true;
    }

    if (!m_protocol.isMessageReuseSupported()) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
            "Property \"" << propStr << "\" is not supported for <message> in DSL version " << m_protocol.currSchema().dslVersion() << ", ignoring...";
        return true;
    }

    auto& valueStr = iter->second;
    auto* msg = m_protocol.findMessage(valueStr);
    if (msg == nullptr) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
                      "The message \"" << valueStr << "\" hasn't been recorded yet.";
        return false;
    }

    assert(msg != this);
    Base::reuseState(*msg);
    m_state = msg->m_state;

    do {
        bool reuseAliases = true;
        if (!validateAndUpdateBoolPropValue(common::reuseAliasesStr(), reuseAliases)) {
            return false;
        }

        if (reuseAliases) {
            break;
        }

        m_state.m_aliases.clear();
    } while (false);    

    do {
        auto& codeProp = common::reuseCodeStr();
        if (!validateSinglePropInstance(codeProp, false)) {
            return false;
        }

        m_state.m_copyCodeFrom.clear();
        auto codeIter = m_props.find(codeProp);
        if (codeIter == m_props.end()) {
            break;
        }  

        bool copyCode = false;
        if (!validateAndUpdateBoolPropValue(codeProp, copyCode)) {
            return false;
        }

        if (!copyCode) {
            break;
        }

        m_state.m_copyCodeFrom = valueStr; 
    } while (false);

    return true;
}

bool ParseMessageImpl::updateName()
{
    bool mustHave = m_state.m_name.empty();
    if (!validateAndUpdateStringPropValue(common::nameStr(), m_state.m_name, mustHave)) {
        return false;
    }

    if (!common::isValidName(m_state.m_name)) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
                      "Invalid value for name property \"" << m_state.m_name << "\".";
        return false;
    }

    return true;
}

bool ParseMessageImpl::updateDescription()
{
    return validateAndUpdateStringPropValue(common::descriptionStr(), m_state.m_description, false, true);
}

bool ParseMessageImpl::updateDisplayName()
{
    return validateAndUpdateStringPropValue(common::displayNameStr(), m_state.m_displayName, false, true);
}

bool ParseMessageImpl::updateId()
{
    if (!validateSinglePropInstance(common::idStr(), true)) {
        return false;
    }

    auto iter = m_props.find(common::idStr());
    std::intmax_t val = 0;
    if (m_protocol.strToEnumValue(iter->second, val)) {
        m_state.m_id = static_cast<decltype(m_state.m_id)>(val);
        return true;
    }

    bool ok = false;
    m_state.m_id = common::strToUintMax(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::idStr(), iter->second);
        return false;
    }

    return true;
}

bool ParseMessageImpl::updateOrder()
{
    if (!validateSinglePropInstance(common::orderStr())) {
        return false;
    }

    auto iter = m_props.find(common::orderStr());
    if (iter == m_props.end()) {
        assert(m_state.m_order == 0U);
        return true;
    }

    bool ok = false;
    m_state.m_order = common::strToUnsigned(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::orderStr(), iter->second);
        return false;
    }

    return true;
}

bool ParseMessageImpl::updateVersions()
{
    if (!validateSinglePropInstance(common::sinceVersionStr())) {
        return false;
    }

    if (!validateSinglePropInstance(common::deprecatedStr())) {
        return false;
    }

    if (!validateSinglePropInstance(common::removedStr())) {
        return false;
    }

    assert(getParent() != nullptr);
    assert(getParent()->objKind() == ObjKind::Namespace);

    unsigned sinceVersion = 0U;
    unsigned deprecated = ParseProtocol::notYetDeprecated();
    if (!ParseXmlWrap::getAndCheckVersions(m_node, name(), m_props, sinceVersion, deprecated, m_protocol)) {
        return false;
    }

    bool deprecatedRemoved = false;
    do {
        auto deprecatedRemovedIter = m_props.find(common::removedStr());
        if (deprecatedRemovedIter == m_props.end()) {
            break;
        }

        bool ok = false;
        deprecatedRemoved = common::strToBool(deprecatedRemovedIter->second, &ok);
        if (!ok) {
            reportUnexpectedPropertyValue(common::removedStr(), deprecatedRemovedIter->second);
            return false;
        }

        if (!deprecatedRemoved) {
            break;
        }

        if (deprecated == ParseProtocol::notYetDeprecated()) {
            logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
                "Property \"" << common::removedStr() << "\" is not applicable to "
                "non deprecated fields";
        }
    } while (false);

    setSinceVersion(sinceVersion);
    setDeprecated(deprecated);
    setDeprecatedRemoved(deprecatedRemoved);
    return true;
}

bool ParseMessageImpl::updatePlatforms()
{
    if (!validateSinglePropInstance(common::platformsStr())) {
        return false;
    }

    auto iter = m_props.find(common::platformsStr());
    if (iter == m_props.end()) {
        assert(m_state.m_platforms.empty());
        return true;
    }

    if (iter->second.empty()) {
        reportUnexpectedPropertyValue(common::platformsStr(), iter->second);
        return false;
    }

    auto op = iter->second[0];
    static const char Plus = '+';
    static const char Minus = '-';
    if ((op != Plus) && (op != Minus)) {
        reportUnexpectedPropertyValue(common::platformsStr(), iter->second);
        return false;
    }

    static const char Sep = ',';
    PlatformsList platList;
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
        reportUnexpectedPropertyValue(common::platformsStr(), iter->second);
        return false;
    }

    auto& allPlatforms = m_protocol.currSchema().platforms();
    for (auto& p : platList) {
        common::removeHeadingTrailingWhitespaces(p);
        if (p.empty()) {
            reportUnexpectedPropertyValue(common::platformsStr(), iter->second);
            return false;
        }

        auto platIter = std::lower_bound(allPlatforms.begin(), allPlatforms.end(), p);
        if ((platIter == allPlatforms.end()) || (*platIter != p)) {
            logError() << ParseXmlWrap::logPrefix(m_node) <<
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
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "Message \"" << name() << "\" is not supported in any platform.";
        return false;
    }

    return true;
}

bool ParseMessageImpl::updateCustomizable()
{
    return validateAndUpdateBoolPropValue(common::customizableStr(), m_state.m_customizable);
}

bool ParseMessageImpl::updateSender()
{
    auto& propStr = common::senderStr();
    if (!validateSinglePropInstance(propStr)) {
        return false;
    }

    auto iter = m_props.find(propStr);
    if (iter == m_props.end()) {
        return true;
    }

    static const std::string Map[] = {
        common::bothStr(),
        common::clientStr(),
        common::serverStr()
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == static_cast<unsigned>(Sender::NumOfValues), "Invalid map");

    auto senderIter = std::find(std::begin(Map), std::end(Map), common::toLowerCopy(iter->second));
    if (senderIter == std::end(Map)) {
        reportUnexpectedPropertyValue(propStr, iter->second);
        return false;
    }

    m_state.m_sender = static_cast<decltype(m_state.m_sender)>(std::distance(std::begin(Map), senderIter));
    return true;
}

bool ParseMessageImpl::updateValidateMinLength()
{
    auto& propStr = common::validateMinLengthStr();
    if (!validateSinglePropInstance(propStr)) {
        return false;
    }

    auto iter = m_props.find(propStr);
    if (iter == m_props.end()) {
        return true;
    }

    if (!m_protocol.isPropertySupported(propStr)) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
            "Property \"" << propStr << "\" is not supported for DSL version " << m_protocol.currSchema().dslVersion() << ", ignoring...";
        return true;
    }

    bool ok = false;
    m_state.m_validateMinLength = static_cast<decltype(m_state.m_validateMinLength)>(common::strToUnsigned(iter->second, &ok));
    if (!ok) {
        reportUnexpectedPropertyValue(propStr, iter->second);
        return false;
    }    
    return true;
}

bool ParseMessageImpl::updateFailOnInvalid()
{
    auto& propStr = common::failOnInvalidStr();
    if (!validateAndUpdateBoolPropValue(propStr, m_state.m_failOnInvalid)) {
        return false;
    }

    if (m_state.m_failOnInvalid && (!m_protocol.isFailOnInvalidInMessageSupported())) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
            "Property \"" << propStr << "\" is not supported for DSL version " << m_protocol.currSchema().dslVersion() << ", ignoring...";
        m_state.m_failOnInvalid = false;
        return true;
    }

    return true;
}

bool ParseMessageImpl::copyFields()
{
    if (!validateSinglePropInstance(common::copyFieldsFromStr())) {
        return false;
    }

    auto iter = m_props.find(common::copyFieldsFromStr());
    if (iter == m_props.end()) {
        return true;
    }

    if (!m_state.m_fields.empty()) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Copying fields from multiple sources using various properties is not supported";
        return false;
    }
    
    do {
        m_copyFieldsFromMsg = m_protocol.findMessage(iter->second);
        if (m_copyFieldsFromMsg != nullptr) {
            cloneFieldsFrom(*m_copyFieldsFromMsg);
            break;
        }

        if (!m_protocol.isCopyFieldsFromBundleSupported()) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
                "Invalid reference to other message \"" << iter->second << "\".";
            return false;            
        }

        auto* copyFromField = m_protocol.findField(iter->second);
        if ((copyFromField == nullptr) || (copyFromField->kind() != ParseField::Kind::Bundle)) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
                "Invalid reference to other message or bundle \"" << iter->second << "\".";
            return false;
        }

        m_copyFieldsFromBundle = static_cast<const ParseBundleFieldImpl*>(copyFromField);
        cloneFieldsFrom(*m_copyFieldsFromBundle);
    } while (false);

    if (!m_state.m_fields.empty()) {
        m_state.m_fields.erase(
            std::remove_if(
                m_state.m_fields.begin(), m_state.m_fields.end(),
                [this](auto& elem)
                {
                    return
                        (elem->isDeprecatedRemoved()) &&
                        (elem->getDeprecated() <= this->getSinceVersion());
                }),
            m_state.m_fields.end());

        for (auto& m : m_state.m_fields) {
            m->setSinceVersion(std::max(getSinceVersion(), m->getSinceVersion()));
        }
    }
    return true;
}

bool ParseMessageImpl::replaceFields()
{
    auto replaceNodes = ParseXmlWrap::getChildren(getNode(), common::replaceStr());
    if (1U < replaceNodes.size()) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Only single \"" << common::replaceStr() << "\" child element is "
            "supported for \"" << common::messageStr() << "\".";
        return false;
    }

    if (replaceNodes.empty()) {
        return true;
    }

    if (!m_protocol.isMemberReplaceSupported()) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
            "Replacing fields with \"" << common::replaceStr() << "\" child element is unavaliable "
            "for selected DSL version, ignoring...";        
        return true;
    }    

    auto fieldsTypes = ParseXmlWrap::getChildren(replaceNodes.front(), messageSupportedTypes());
    if (fieldsTypes.size() != replaceNodes.size()) {
        logError() << ParseXmlWrap::logPrefix(replaceNodes.front()) <<
            "The \"" << common::replaceStr() << "\" child node of \"" <<
            common::messageStr() << "\" element must contain only supported field types.";
        return false;
    }    

    ParseFieldImpl::FieldsList replMembers;
    replMembers.reserve(fieldsTypes.size());
    for (auto* fieldNode : fieldsTypes) {
        std::string memKind(reinterpret_cast<const char*>(fieldNode->name));
        auto field = ParseFieldImpl::create(memKind, fieldNode, m_protocol);
        if (!field) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            logError() << ParseXmlWrap::logPrefix(replaceNodes.front()) <<
                "Internal error, failed to create objects for fields to replace.";
            return false;
        }

        field->setParent(this);
        if (!field->parse()) {
            return false;
        }

        if (!field->verifySiblings(m_state.m_fields)) {
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
                    return field->name() == currField->name();
                });

        if (iter == m_state.m_fields.end()) {
            logError() << ParseXmlWrap::logPrefix(field->getNode()) <<
                "Cannot find reused field with name \"" << field->name() << "\" to replace.";
            return false;
        }

        (*iter) = std::move(field);
    }

    return true;       
}

bool ParseMessageImpl::copyAliases()
{
    auto& propStr = common::copyFieldsAliasesStr();
    if (!validateSinglePropInstance(propStr)) {
        return false;
    }

    auto iter = m_props.find(propStr);
    if (iter != m_props.end() && (!m_protocol.isFieldAliasSupported())) {
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "Unexpected property \"" << propStr << "\".";
        return false;
    }

    if (!m_protocol.isFieldAliasSupported()) {
        return true;
    }

    bool copyAliases = true;
    if (iter != m_props.end()) {
        bool ok = false;
        copyAliases = common::strToBool(iter->second, &ok);
        if (!ok) {
            reportUnexpectedPropertyValue(propStr, iter->second);
            return false;
        }
    }

    if (!copyAliases) {
        return true;
    }

    if ((iter != m_props.end()) && (m_copyFieldsFromMsg == nullptr) && (m_copyFieldsFromBundle == nullptr)) {
        logWarning() << ParseXmlWrap::logPrefix(m_node) <<
            "Property \"" << propStr << "\" is inapplicable without \"" << common::copyFieldsFromStr() << "\".";
        return true;
    }

    if (m_copyFieldsFromMsg != nullptr) {
        cloneAliasesFrom(*m_copyFieldsFromMsg);
    }
    else if (m_copyFieldsFromBundle != nullptr) {
        cloneAliasesFrom(*m_copyFieldsFromBundle);
    }

    if (!m_state.m_aliases.empty()) {
        m_state.m_aliases.erase(
            std::remove_if(
                m_state.m_aliases.begin(), m_state.m_aliases.end(),
                [this](auto& alias)
                {
                    auto& fieldName = alias->fieldName();
                    assert(!fieldName.empty());
                    auto fieldIter =
                        std::find_if(
                            m_state.m_fields.begin(), m_state.m_fields.end(),
                            [&fieldName](auto& f)
                            {
                                return fieldName == f->name();
                            });

                    return fieldIter == m_state.m_fields.end();
                }),
            m_state.m_aliases.end());
    }
    return true;
}

bool ParseMessageImpl::updateFields()
{
    do {
        auto fieldsNodes = ParseXmlWrap::getChildren(getNode(), common::fieldsStr());
        if (1U < fieldsNodes.size()) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
                          "Only single \"" << common::fieldsStr() << "\" child element is "
                          "supported for \"" << common::messageStr() << "\".";
            return false;
        }

        auto fieldsTypes = ParseXmlWrap::getChildren(getNode(), messageSupportedTypes());
        if ((!fieldsNodes.empty()) && (!fieldsTypes.empty())) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
                          "The \"" << common::messageStr() << "\" element does not support "
                          "list of stand alone fields as child elements together with \"" <<
                          common::fieldsStr() << "\" child element.";
            return false;
        }

        if ((fieldsNodes.empty()) && (fieldsTypes.empty())) {
            break;
        }

        if ((0U < fieldsTypes.size())) {
            assert(0U == fieldsNodes.size());
            auto allChildren = ParseXmlWrap::getChildren(getNode());
            if (allChildren.size() != fieldsTypes.size()) {
                logError() << ParseXmlWrap::logPrefix(getNode()) <<
                              "The field types of \"" << common::messageStr() <<
                              "\" must be defined inside \"<" << common::fieldsStr() << ">\" child element "
                              "when there are other property describing children.";
                return false;
            }
        }

        if (0U < fieldsNodes.size()) {
            assert(0U == fieldsTypes.size());
            fieldsTypes = ParseXmlWrap::getChildren(fieldsNodes.front());
            auto cleanMemberFieldsTypes = ParseXmlWrap::getChildren(fieldsNodes.front(), messageSupportedTypes());
            if (cleanMemberFieldsTypes.size() != fieldsTypes.size()) {
                logError() << ParseXmlWrap::logPrefix(fieldsNodes.front()) <<
                    "The \"" << common::fieldsStr() << "\" child node of \"" <<
                    common::messageStr() << "\" element must contain only supported field types.";
                return false;
            }

            // fieldsTypes is updated with the list from <fields>
        }

        m_state.m_fields.reserve(m_state.m_fields.size() + fieldsTypes.size());
        for (auto* fNode : fieldsTypes) {
            std::string fKind(reinterpret_cast<const char*>(fNode->name));
            auto field = ParseFieldImpl::create(fKind, fNode, m_protocol);
            if (!field) {
                [[maybe_unused]] static constexpr bool Should_not_happen = false;
                assert(Should_not_happen);
                logError() << ParseXmlWrap::logPrefix(getNode()) <<
                      "Internal error, failed to create objects for member fields.";
                return false;
            }

            field->setParent(this);
            if (!field->parse()) {
                return false;
            }

            if (!field->verifySiblings(m_state.m_fields)) {
                return false;
            }

            m_state.m_fields.push_back(std::move(field));
        }

        if (!ParseFieldImpl::validateMembersNames(m_state.m_fields, m_protocol.logger())) {
            return false;
        }

        if (0 <= m_state.m_validateMinLength) {
            auto len = minLength();
            if (static_cast<unsigned>(m_state.m_validateMinLength) != len) {
                logError() << ParseXmlWrap::logPrefix(getNode()) <<
                    "The calculated minimal length of the message is " << len <<
                    " while expected is " << m_state.m_validateMinLength << " (specified with \"" << common::validateMinLengthStr() << "\" property).";                
                return false;
            }
        }

    } while (false);

    return true;
}

bool ParseMessageImpl::updateAliases()
{
    auto aliasNodes = ParseXmlWrap::getChildren(getNode(), common::aliasStr());

    if (aliasNodes.empty()) {
        return true;
    }

    if (!m_protocol.isFieldAliasSupported()) {
        logError() << ParseXmlWrap::logPrefix(aliasNodes.front()) <<
              "Using \"" << common::aliasStr() << "\" nodes for too early \"" <<
              common::dslVersionStr() << "\".";
        return false;
    }

    m_state.m_aliases.reserve(m_state.m_aliases.size() + aliasNodes.size());
    for (auto* aNode : aliasNodes) {
        auto alias = ParseAliasImpl::create(aNode, m_protocol);
        if (!alias) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            logError() << ParseXmlWrap::logPrefix(alias->getNode()) <<
                  "Internal error, failed to create objects for member aliases.";
            return false;
        }

        if (!alias->parse()) {
            return false;
        }

        if (!alias->verifyAlias(m_state.m_aliases, m_state.m_fields)) {
            return false;
        }

        m_state.m_aliases.push_back(std::move(alias));
    }

    return true;
}

void ParseMessageImpl::cloneFieldsFrom(const ParseMessageImpl& other)
{
    m_state.m_fields.reserve(other.m_state.m_fields.size());
    for (auto& f : other.m_state.m_fields) {
        m_state.m_fields.push_back(f->clone());
    }
}

void ParseMessageImpl::cloneFieldsFrom(const ParseBundleFieldImpl& other)
{
    m_state.m_fields.reserve(other.members().size());
    for (auto& f : other.members()) {
        m_state.m_fields.push_back(f->clone());
    }
}

void ParseMessageImpl::cloneAliasesFrom(const ParseMessageImpl& other)
{
    m_state.m_aliases.reserve(other.m_state.m_aliases.size());
    for (auto& a : other.m_state.m_aliases) {
        m_state.m_aliases.push_back(a->clone());
    }
}

void ParseMessageImpl::cloneAliasesFrom(const ParseBundleFieldImpl& other)
{
    m_state.m_aliases.reserve(other.aliases().size());
    for (auto& a : other.aliases()) {
        m_state.m_aliases.push_back(a->clone());
    }
}

bool ParseMessageImpl::updateReadOverride()
{
    return validateAndUpdateOverrideTypePropValue(common::readOverrideStr(), m_state.m_readOverride);
}

bool ParseMessageImpl::updateWriteOverride()
{
    return validateAndUpdateOverrideTypePropValue(common::writeOverrideStr(), m_state.m_writeOverride);
}

bool ParseMessageImpl::updateRefreshOverride()
{
    return validateAndUpdateOverrideTypePropValue(common::refreshOverrideStr(), m_state.m_refreshOverride);
}

bool ParseMessageImpl::updateLengthOverride()
{
    return validateAndUpdateOverrideTypePropValue(common::lengthOverrideStr(), m_state.m_lengthOverride);
}

bool ParseMessageImpl::updateValidOverride()
{
    return validateAndUpdateOverrideTypePropValue(common::validOverrideStr(), m_state.m_validOverride);
}

bool ParseMessageImpl::updateNameOverride()
{
    return validateAndUpdateOverrideTypePropValue(common::nameOverrideStr(), m_state.m_nameOverride);
}

bool ParseMessageImpl::updateCopyOverrideCodeFrom()
{
    auto& prop = common::copyCodeFromStr();
    if (!validateSinglePropInstance(prop, false)) {
        return false;
    }

    auto iter = m_props.find(prop);
    if (iter == m_props.end()) {
        return true;
    }  

    if (!m_protocol.isPropertySupported(prop)) {
        logWarning() << ParseXmlWrap::logPrefix(m_node) <<
            "The property \"" << prop << "\" is not supported for dslVersion=" << 
                m_protocol.currSchema().dslVersion() << ".";        
        return true;
    }    

    auto* msg = m_protocol.findMessage(iter->second);
    if (msg == nullptr) {
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "Message referenced by \"" << prop << "\" property (" + iter->second + ") is not found.";
        return false;        
    }

    m_state.m_copyCodeFrom = iter->second;
    return true;
}

bool ParseMessageImpl::copyConstruct()
{
    auto& prop = common::copyConstructFromStr();
    if (!validateSinglePropInstance(prop, false)) {
        return false;
    }

    auto iter = m_props.find(prop);
    if (iter == m_props.end()) {
        return true;
    }  

    if (!m_protocol.isPropertySupported(prop)) {
        logWarning() << ParseXmlWrap::logPrefix(m_node) <<
            "The property \"" << prop << "\" is not supported for dslVersion=" << 
                m_protocol.currSchema().dslVersion() << ".";        
        return true;
    }    

    auto* msg = m_protocol.findMessage(iter->second);
    if (msg == nullptr) {
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "Message referenced by \"" << prop << "\" property (" + iter->second + ") is not found.";
        return false;        
    }

    if (!msg->m_state.m_construct) {
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "Message referenced by \"" << prop << "\" property (" + iter->second + ") does not specify construction conditions.";
        return false;        
    }

    auto newConstruct = msg->m_state.m_construct->clone();
    if (!newConstruct->verify(ParseOptCondImpl::FieldsList(), m_node, m_protocol)) {
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "Copied construct conditions cannot be applied to this message.";
        return false;
    }    

    m_state.m_construct = std::move(newConstruct);
    return true;
}

bool ParseMessageImpl::copyReadCond()
{
    auto& prop = common::copyReadCondFromStr();
    if (!validateSinglePropInstance(prop, false)) {
        return false;
    }

    auto iter = m_props.find(prop);
    if (iter == m_props.end()) {
        return true;
    }  

    if (!m_protocol.isPropertySupported(prop)) {
        logWarning() << ParseXmlWrap::logPrefix(m_node) <<
            "The property \"" << prop << "\" is not supported for dslVersion=" << 
                m_protocol.currSchema().dslVersion() << ".";        
        return true;
    }    

    auto* msg = m_protocol.findMessage(iter->second);
    if (msg == nullptr) {
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "Message referenced by \"" << prop << "\" property (" + iter->second + ") is not found.";
        return false;        
    }

    if (!msg->m_state.m_readCond) {
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "Message referenced by \"" << prop << "\" property (" + iter->second + ") does not specify read conditions.";
        return false;        
    }

    auto newReadCond = msg->m_state.m_readCond->clone();
    if (!newReadCond->verify(ParseOptCondImpl::FieldsList(), m_node, m_protocol)) {
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "Copied read conditions cannot be applied to this message.";
        return false;
    }    

    m_state.m_readCond = std::move(newReadCond);
    return true;
}

bool ParseMessageImpl::copyValidCond()
{
    auto& prop = common::copyValidCondFromStr();
    if (!validateSinglePropInstance(prop, false)) {
        return false;
    }

    auto iter = m_props.find(prop);
    if (iter == m_props.end()) {
        return true;
    }  

    if (!m_protocol.isPropertySupported(prop)) {
        logWarning() << ParseXmlWrap::logPrefix(m_node) <<
            "The property \"" << prop << "\" is not supported for dslVersion=" << 
                m_protocol.currSchema().dslVersion() << ".";        
        return true;
    }    

    const ParseMessageImpl* msg = nullptr;
    const ParseBundleFieldImpl* bundle = nullptr;
    do {
        msg = m_protocol.findMessage(iter->second);
        if (msg != nullptr) {
            break;
        }

        auto otherField = m_protocol.findField(iter->second);
        if (otherField == nullptr) {
            logError() << ParseXmlWrap::logPrefix(m_node) <<
                "Neither message nor bundle field referenced by \"" << prop << "\" property (" + iter->second + ") is not found.";
            return false;
        }

        if (otherField->kind() != ParseFieldImpl::Kind::Bundle) {
            logError() << ParseXmlWrap::logPrefix(m_node) <<
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
        srcCondPtr = bundle->validCondImpl().get();
    }

    if (srcCondPtr == nullptr) {
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "Message / bundle referenced by \"" << prop << "\" property (" + iter->second + ") does not specify validity conditions.";
        return false;        
    }

    auto newCond = srcCondPtr->clone();
    if (!newCond->verify(m_state.m_fields, m_node, m_protocol)) {
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "Copied validity conditions cannot be applied to this message.";
        return false;
    }    

    m_state.m_validCond = std::move(newCond);
    return true;
}

bool ParseMessageImpl::updateSingleConstruct()
{
    if (!updateSingleCondInternal(common::constructStr(), m_state.m_construct)) {
        return false;
    }

    if (!m_state.m_construct) {
        return true;
    }

    if (!verifyConstructInternal(ParseOptCond(m_state.m_construct.get()))) {
        m_state.m_construct.reset();
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "Only bit checks and equality comparisons are supported in the \"" << common::constructStr() << "\" property.";
        return false;
    }

    return true;
}

bool ParseMessageImpl::updateMultiConstruct()
{
    if (!updateMultiCondInternal(common::constructStr(), m_state.m_construct)) {
        return false;
    }

    if (!m_state.m_construct) {
        return true;
    }

    if (!verifyConstructInternal(ParseOptCond(m_state.m_construct.get()))) {
        m_state.m_construct.reset();
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "Only \"" << common::andStr() <<  
            "\" of the bit checks and equality comparisons are supported in the \"" << common::constructStr() << "\" element.";
        return false;
    }    

    return true;
}

bool ParseMessageImpl::updateSingleReadCond()
{
    return updateSingleCondInternal(common::readCondStr(), m_state.m_readCond);
}

bool ParseMessageImpl::updateMultiReadCond()
{
    return updateMultiCondInternal(common::readCondStr(), m_state.m_readCond);
}

bool ParseMessageImpl::updateSingleValidCond()
{
    return updateSingleCondInternal(common::validCondStr(), m_state.m_validCond, true);
}

bool ParseMessageImpl::updateMultiValidCond()
{
    return updateMultiCondInternal(common::validCondStr(), m_state.m_validCond, true);
}

bool ParseMessageImpl::copyConstructToReadCond()
{
    return 
        copyCondInternal(
            common::constructAsReadCondStr(),
            common::constructStr(),
            m_state.m_construct,
            common::readCondStr(),
            m_state.m_readCond);
}

bool ParseMessageImpl::copyConstructToValidCond()
{
    return 
        copyCondInternal(
            common::constructAsValidCondStr(),
            common::constructStr(),
            m_state.m_construct,
            common::validCondStr(),
            m_state.m_validCond);
}

bool ParseMessageImpl::updateExtraAttrs()
{
    m_extraAttrs = ParseXmlWrap::getExtraAttributes(m_node, allProps(), m_protocol);
    return true;
}

bool ParseMessageImpl::updateExtraChildren()
{
    static const ParseXmlWrap::NamesList ChildrenNames = allNames();
    m_extraChildren = ParseXmlWrap::getExtraChildren(m_node, ChildrenNames, m_protocol);
    return true;
}

bool ParseMessageImpl::updateSingleCondInternal(const std::string& prop, ParseOptCondImplPtr& cond, bool allowFieldsAccess)
{
    if (!validateSinglePropInstance(prop)) {
        return false;
    }

    auto iter = m_props.find(prop);
    if (iter == m_props.end()) {
        return true;
    }

    if (!m_protocol.isPropertySupported(prop)) {
        logWarning() << ParseXmlWrap::logPrefix(m_node) <<
            "The property \"" << prop << "\" is not supported for dslVersion=" << 
                m_protocol.currSchema().dslVersion() << ".";        
        return true;
    }          

    auto newCond = std::make_unique<ParseOptCondExprImpl>();
    if (!newCond->parse(iter->second, m_node, m_protocol)) {
        return false;
    }

    static const ParseOptCondImpl::FieldsList NoFields;
    auto* fieldsPtr = &NoFields;
    if (allowFieldsAccess) {
        fieldsPtr = &m_state.m_fields;
    }    

    if (!newCond->verify(*fieldsPtr, m_node, m_protocol)) {
        return false;
    }   

    cond = std::move(newCond);
    return true; 
}

bool ParseMessageImpl::updateMultiCondInternal(const std::string& prop, ParseOptCondImplPtr& cond, bool allowFieldsAccess)
{
    auto condNodes = ParseXmlWrap::getChildren(m_node, prop, true);
    if (condNodes.empty()) {
        return true;
    }

    if (!m_protocol.isPropertySupported(prop)) {
        logWarning() << ParseXmlWrap::logPrefix(m_node) <<
            "The property \"" << prop << "\" is not supported for dslVersion=" << 
                m_protocol.currSchema().dslVersion() << ".";        
        return true;
    }      

    if (condNodes.size() > 1U) {
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "Cannot use more that one child to the \"" << prop << "\" element.";        
        return false;
    }

    static const ParseXmlWrap::NamesList ElemNames = {
        common::andStr(),
        common::orStr()
    };

    auto condChildren = ParseXmlWrap::getChildren(condNodes.front(), ElemNames);
    if (condChildren.size() != condNodes.size()) {
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "Only single \"" << common::andStr() << "\" or \"" << common::orStr() << "\" child of the \"" << prop << "\" element is supported.";           
        return false;
    }    

    auto iter = props().find(prop);
    if (iter != props().end()) {
        logError() << ParseXmlWrap::logPrefix(condNodes.front()) <<
            "Only single \"" << prop << "\" property is supported";
        return false;
    }

    auto newCond = std::make_unique<ParseOptCondListImpl>();
    newCond->overrideCondStr(prop);
    if (!newCond->parse(condChildren.front(), m_protocol)) {
        return false;
    }

    static const ParseOptCondImpl::FieldsList NoFields;
    auto* fieldsPtr = &NoFields;
    if (allowFieldsAccess) {
        fieldsPtr = &m_state.m_fields;
    }

    if (!newCond->verify(*fieldsPtr, condChildren.front(), m_protocol)) {
        return false;
    }    

    cond = std::move(newCond);
    return true;
}

bool ParseMessageImpl::copyCondInternal(
    const std::string& copyProp,
    const std::string& fromProp, 
    const ParseOptCondImplPtr& fromCond, 
    const std::string& toProp,
    ParseOptCondImplPtr& toCond,
    bool allowOverride)
{
    if (!validateSinglePropInstance(copyProp)) {
        return false;
    }

    auto iter = m_props.find(copyProp);
    if (iter == m_props.end()) {
        return true;
    }    

    if (!m_protocol.isPropertySupported(copyProp)) {
        logWarning() << ParseXmlWrap::logPrefix(m_node) <<
            "The property \"" << copyProp << "\" is not supported for dslVersion=" << 
                m_protocol.currSchema().dslVersion() << ".";        
        return true;
    }      

    bool ok = false;
    bool copyRequested = common::strToBool(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(copyProp, iter->second);
        return false;
    }

    if (!copyRequested) {
        return true;
    }

    if (!fromCond) {
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "No \"" << fromProp << "\" conditions were defined to copy.";           
        return false;            
    }

    if (toCond && (!allowOverride)) {
        logError() << ParseXmlWrap::logPrefix(m_node) <<
            "Set of the \"" << copyProp << "\" property overrides existing \"" << toProp << "\" setting.";          
        return false;
    }

    toCond = fromCond->clone();
    return true;    
}

} // namespace parse

} // namespace commsdsl
