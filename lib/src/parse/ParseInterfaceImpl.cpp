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

#include "ParseInterfaceImpl.h"

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

const ParseXmlWrap::NamesList& parseInterfaceSupportedTypes()
{
    static const ParseXmlWrap::NamesList Names = ParseFieldImpl::parseSupportedTypes();
    return Names;
}

} // namespace

ParseInterfaceImpl::ParseInterfaceImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : m_node(node),
    m_protocol(protocol)
{
}

bool ParseInterfaceImpl::parse()
{
    m_props = ParseXmlWrap::parseNodeProps(m_node);

    if (!ParseXmlWrap::parseChildrenAsProps(m_node, parseCommonProps(), m_protocol.parseLogger(), m_props)) {
        return false;
    }

    return
        parseCheckReuse() &&
        parseUpdateName() &&
        parseUpdateDescription() &&
        parseCopyFields() &&
        parseUpdateFields() &&
        parseCopyAliases() &&
        parseUpdateAliases() &&
        parseUpdateExtraAttrs() &&
        parseUpdateExtraChildren();
}

const std::string& ParseInterfaceImpl::parseName() const
{
    return m_state.m_name;
}

const std::string& ParseInterfaceImpl::parseDescription() const
{
    return m_state.m_description;
}

const std::string& ParseInterfaceImpl::parseCopyCodeFrom() const
{
    return m_state.m_copyCodeFrom;
}

ParseInterfaceImpl::FieldsList ParseInterfaceImpl::parseFieldsList() const
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

ParseInterfaceImpl::AliasesList ParseInterfaceImpl::parseAliasesList() const
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

std::string ParseInterfaceImpl::parseExternalRef(bool schemaRef) const
{
    assert(parseGetParent() != nullptr);
    assert(parseGetParent()->parseObjKind() == ObjKind::Namespace);

    auto& ns = static_cast<const ParseNamespaceImpl&>(*parseGetParent());
    auto nsRef = ns.parseExternalRef(schemaRef);
    
    if (nsRef.empty()) {
        return parseName();
    }

    return nsRef + '.' + parseName();
}

std::size_t ParseInterfaceImpl::parseFindFieldIdx(const std::string& name) const
{
    auto iter =
        std::find_if(
            m_state.m_fields.begin(), m_state.m_fields.end(),
            [&name](auto& fPtr)
            {
                return fPtr->parseName() == name;
            });
    if (iter == m_state.m_fields.end()) {
        return std::numeric_limits<std::size_t>::max();
    }

    return static_cast<std::size_t>(std::distance(m_state.m_fields.begin(), iter));
}

ParseInterfaceImpl::ImplFieldsList ParseInterfaceImpl::parseAllImplFields() const
{
    ImplFieldsList result;
    result.reserve(m_state.m_fields.size());
    for (auto& fPtr : m_state.m_fields) {
        result.push_back(fPtr.get());
    }

    return result;
}

ParseInterfaceImpl::FieldRefInfo ParseInterfaceImpl::processInnerFieldRef(const std::string refStr) const
{
    if ((!refStr.empty()) && ((refStr[0] == '#') || (refStr[0] == '?'))) {
        auto info = processInnerFieldRef(refStr.substr(1));
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

    auto dotPos = refStr.find_first_of(".");
    const auto fieldName = refStr.substr(0, dotPos);
    auto iter = 
        std::find_if(
            m_state.m_fields.begin(), m_state.m_fields.end(),
            [&fieldName](auto& fieldPtr)
            {
                return fieldName == fieldPtr->parseName();
            });

    if (iter == m_state.m_fields.end()) {
        return FieldRefInfo();
    }

    std::string nextRef;
    if (dotPos < refStr.size()) {
        nextRef = refStr.substr(dotPos + 1U);
    }

    return (*iter)->parseProcessInnerRef(nextRef);

}

ParseObject::ObjKind ParseInterfaceImpl::parseObjKindImpl() const
{
    return ObjKind::Interface;
}

LogWrapper ParseInterfaceImpl::parseLogError() const
{
    return commsdsl::parse::parseLogError(m_protocol.parseLogger());
}

LogWrapper ParseInterfaceImpl::parseLogWarning() const
{
    return commsdsl::parse::parseLogWarning(m_protocol.parseLogger());
}

LogWrapper ParseInterfaceImpl::parseLogInfo() const
{
    return commsdsl::parse::parseLogInfo(m_protocol.parseLogger());
}

bool ParseInterfaceImpl::parseValidateSinglePropInstance(const std::string& str, bool mustHave)
{
    return ParseXmlWrap::parseValidateSinglePropInstance(m_node, m_props, str, m_protocol.parseLogger(), mustHave);
}

bool ParseInterfaceImpl::parseValidateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave)
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
            "Property \"" << common::availableLengthLimitStr() << "\" is not available for dslVersion= " << m_protocol.parseCurrSchema().parseDslVersion();                
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

bool ParseInterfaceImpl::parseValidateAndUpdateStringPropValue(
    const std::string& str,
    std::string& value,
    bool mustHave)
{
    if (!parseValidateSinglePropInstance(str, mustHave)) {
        return false;
    }

    auto iter = m_props.find(str);
    if (iter != m_props.end()) {
        value = iter->second;
    }

    assert(iter != m_props.end() || (!mustHave));
    return true;
}

void ParseInterfaceImpl::parseReportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue)
{
    ParseXmlWrap::parseReportUnexpectedPropertyValue(m_node, parseName(), propName, propValue, m_protocol.parseLogger());
}

const ParseXmlWrap::NamesList& ParseInterfaceImpl::parseCommonProps()
{
    static const ParseXmlWrap::NamesList CommonNames = {
        common::nameStr(),
        common::descriptionStr(),
        common::copyFieldsFromStr(),
        common::copyFieldsAliasesStr(),
        common::reuseStr(),
        common::reuseCodeStr(),
        common::reuseAliasesStr(),
    };

    return CommonNames;
}

ParseXmlWrap::NamesList ParseInterfaceImpl::parseAllNames()
{
    auto names = parseCommonProps();
    auto& fieldTypes = parseInterfaceSupportedTypes();
    names.insert(names.end(), fieldTypes.begin(), fieldTypes.end());
    names.push_back(common::fieldsStr());
    names.push_back(common::aliasStr());
    return names;
}

bool ParseInterfaceImpl::parseCheckReuse()
{
    auto& propStr = common::reuseStr();
    if (!parseValidateSinglePropInstance(propStr)) {
        return false;
    }

    auto iter = m_props.find(propStr);
    if (iter == m_props.end()) {
        return true;
    }

    if (!m_protocol.parseIsInterfaceReuseSupported()) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Property \"" << propStr << "\" is not supported for <interface> in DSL version " << m_protocol.parseCurrSchema().parseDslVersion() << ", ignoring...";
        return true;
    }

    auto& valueStr = iter->second;
    auto* iFace = m_protocol.parseFindInterface(valueStr);
    if (iFace == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "The message \"" << valueStr << "\" hasn't been recorded yet.";
        return false;
    }

    assert(iFace != this);
    Base::parseReuseState(*iFace);
    m_state = iFace->m_state;

    do {
        bool reuseAliases = true;
        if (!parseValidateAndUpdateBoolPropValue(common::reuseAliasesStr(), reuseAliases)) {
            return false;
        }

        if (reuseAliases) {
            break;
        }

        m_state.m_aliases.clear();
    } while (false);     

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
    return true;
}

bool ParseInterfaceImpl::parseUpdateName()
{
    bool mustHave = m_state.m_name.empty();
    if (!parseValidateAndUpdateStringPropValue(common::nameStr(), m_state.m_name, mustHave)) {
        return false;
    }

    if (!common::isValidName(m_state.m_name)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Invalid value for name property \"" << m_state.m_name << "\".";
        return false;
    }

    return true;
}

bool ParseInterfaceImpl::parseUpdateDescription()
{
    return parseValidateAndUpdateStringPropValue(common::descriptionStr(), m_state.m_description);
}

bool ParseInterfaceImpl::parseCopyFields()
{
    if (!parseValidateSinglePropInstance(common::copyFieldsFromStr())) {
        return false;
    }

    auto iter = parseProps().find(common::copyFieldsFromStr());
    if (iter == parseProps().end()) {
        return true;
    }

    if (!m_state.m_fields.empty()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Copying fields from multiple sources using various properties is not supported";
        return false;
    }    

    do {
        m_copyFieldsFromInterface = m_protocol.parseFindInterface(iter->second);
        if (m_copyFieldsFromInterface != nullptr) {
            parseCloneFieldsFrom(*m_copyFieldsFromInterface);
            break;
        }

        if (!m_protocol.parseIsCopyFieldsFromBundleSupported()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Invalid reference to other interface \"" << iter->second << "\".";
            return false;            
        }

        auto* copyFromField = m_protocol.parseFindField(iter->second);
        if ((copyFromField == nullptr) || (copyFromField->parseKind() != ParseField::Kind::Bundle)) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Invalid reference to other interface or bundle \"" << iter->second << "\".";
            return false;
        }        

        m_copyFieldsFromBundle = static_cast<const ParseBundleFieldImpl*>(copyFromField);
        parseCloneFieldsFrom(*m_copyFieldsFromBundle);
    } while (false);

    return true;
}

bool ParseInterfaceImpl::parseUpdateFields()
{
    do {
        auto fieldsNodes = ParseXmlWrap::parseGetChildren(parseGetNode(), common::fieldsStr());
        if (1U < fieldsNodes.size()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                          "Only single \"" << common::fieldsStr() << "\" child element is "
                          "supported for \"" << common::interfaceStr() << "\".";
            return false;
        }

        auto fieldsTypes = ParseXmlWrap::parseGetChildren(parseGetNode(), parseInterfaceSupportedTypes());
        if ((!fieldsNodes.empty()) && (!fieldsTypes.empty())) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                          "The \"" << common::interfaceStr() << "\" element does not support "
                          "list of stand alone fields as child elements together with \"" <<
                          common::fieldsStr() << "\" child element.";
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
                              "The field types of \"" << common::interfaceStr() <<
                              "\" must be defined inside \"<" << common::fieldsStr() << ">\" child element "
                              "when there are other property describing children.";
                return false;
            }
        }

        if (0U < fieldsNodes.size()) {
            assert(0U == fieldsTypes.size());
            fieldsTypes = ParseXmlWrap::parseGetChildren(fieldsNodes.front());
            auto cleanMemberFieldsTypes = ParseXmlWrap::parseGetChildren(fieldsNodes.front(), parseInterfaceSupportedTypes());
            if (cleanMemberFieldsTypes.size() != fieldsTypes.size()) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(fieldsNodes.front()) <<
                    "The \"" << common::fieldsStr() << "\" child node of \"" <<
                    common::interfaceStr() << "\" element must contain only supported field types.";
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

    } while (false);

    return true;
}

bool ParseInterfaceImpl::parseCopyAliases()
{
    auto& propStr = common::copyFieldsAliasesStr();
    if (!parseValidateSinglePropInstance(propStr)) {
        return false;
    }

    auto iter = parseProps().find(propStr);
    if (iter != parseProps().end() && (!m_protocol.parseIsFieldAliasSupported())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Unexpected property \"" << propStr << "\".";
        return false;
    }

    if (!m_protocol.parseIsFieldAliasSupported()) {
        return true;
    }

    bool copyAliases = true;
    if (iter != parseProps().end()) {
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

    if ((iter != parseProps().end()) && (m_copyFieldsFromInterface == nullptr) && (m_copyFieldsFromBundle == nullptr)) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Property \"" << propStr << "\" is inapplicable without \"" << common::copyFieldsFromStr() << "\".";
        return true;
    }

    if (m_copyFieldsFromInterface != nullptr) {
        parseCloneAliasesFrom(*m_copyFieldsFromInterface);
    }
    else if (m_copyFieldsFromBundle != nullptr) {
        parseCloneAliasesFrom(*m_copyFieldsFromBundle);
    }    
    else {
        return true;
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

void ParseInterfaceImpl::parseCloneFieldsFrom(const ParseInterfaceImpl& other)
{
    m_state.m_fields.reserve(other.m_state.m_fields.size());
    for (auto& f : other.m_state.m_fields) {
        m_state.m_fields.push_back(f->parseClone());
    }
}

void ParseInterfaceImpl::parseCloneFieldsFrom(const ParseBundleFieldImpl& other)
{
    m_state.m_fields.reserve(other.parseMembers().size());
    for (auto& f : other.parseMembers()) {
        m_state.m_fields.push_back(f->parseClone());
    }
}

void ParseInterfaceImpl::parseCloneAliasesFrom(const ParseInterfaceImpl& other)
{
    m_state.m_aliases.reserve(other.m_state.m_aliases.size());
    for (auto& a : other.m_state.m_aliases) {
        m_state.m_aliases.push_back(a->parseClone());
    }
}

void ParseInterfaceImpl::parseCloneAliasesFrom(const ParseBundleFieldImpl& other)
{
    m_state.m_aliases.reserve(other.parseAliases().size());
    for (auto& a : other.parseAliases()) {
        m_state.m_aliases.push_back(a->parseClone());
    }
}

bool ParseInterfaceImpl::parseUpdateAliases()
{
    auto aliasNodes = ParseXmlWrap::parseGetChildren(parseGetNode(), common::aliasStr());

    if (aliasNodes.empty()) {
        return true;
    }

    if (!m_protocol.parseIsFieldAliasSupported()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(aliasNodes.front()) <<
              "Using \"" << common::aliasStr() << "\" nodes for too early \"" <<
              common::dslVersionStr() << "\".";
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

bool ParseInterfaceImpl::parseUpdateExtraAttrs()
{
    m_extraAttrs = ParseXmlWrap::parseGetExtraAttributes(m_node, parseCommonProps(), m_protocol);
    return true;
}

bool ParseInterfaceImpl::parseUpdateExtraChildren()
{
    static const ParseXmlWrap::NamesList ChildrenNames = parseAllNames();
    m_extraChildren = ParseXmlWrap::parseGetExtraChildren(m_node, ChildrenNames, m_protocol);
    return true;
}


} // namespace parse

} // namespace commsdsl
