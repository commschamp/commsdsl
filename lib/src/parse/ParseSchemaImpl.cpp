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

#include "ParseSchemaImpl.h"

#include "ParseNamespaceImpl.h"
#include "ParseProtocolImpl.h"
#include "parse_common.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <numeric>
#include <type_traits>

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseXmlWrap::ParseNamesList PropNames = {
    common::parseNameStr(),
    common::parseDisplayNameStr(),
    common::parseIdStr(),
    common::parseVersionStr(),
    common::parseDslVersionStr(),
    common::parseEndianStr(),
    common::parseDescriptionStr(),
    common::parseNonUniqueMsgIdAllowedStr()
};

ParseXmlWrap::ParseNamesList parseGetChildrenList()
{
    ParseXmlWrap::ParseNamesList result = PropNames;
    auto& nsNames = ParseNamespaceImpl::expectedChildrenNames();
    result.insert(result.end(), nsNames.begin(), nsNames.end());
    result.push_back(common::parsePlatformsStr());
    result.push_back(common::parsePlatformStr());
    return result;
}

} // namespace

ParseSchemaImpl::ParseSchemaImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : m_node(node),
    m_protocol(protocol)
{
}

bool ParseSchemaImpl::parseProcessNode()
{

    m_props = ParseXmlWrap::parseNodeProps(m_node);
    if (!ParseXmlWrap::parseChildrenAsProps(m_node, PropNames, m_protocol.parseLogger(), m_props)) {
        return false;
    }

    if ((!parseUpdateStringProperty(m_props, common::parseNameStr(), m_name)) ||
        (!parseUpdateStringProperty(m_props, common::parseDescriptionStr(), m_description)) ||
        (!parseUpdateUnsignedProperty(m_props, common::parseIdStr(), m_id)) ||
        (!parseUpdateUnsignedProperty(m_props, common::parseVersionStr(), m_version)) ||
        (!parseUpdateUnsignedProperty(m_props, common::parseDslVersionStr(), m_dslVersion)) ||
        (!parseUpdateEndianProperty(m_props, common::parseEndianStr(), m_endian)) ||
        (!parseUpdateBooleanProperty(m_props, common::parseNonUniqueMsgIdAllowedStr(), m_nonUniqueMsgIdAllowed)) ||
        (!parseUpdateDisplayName()) ||
        (!parseUpdateExtraAttrs()) ||
        (!parseUpdateExtraChildren())) {
        return false;
    }

    if ((!m_name.empty()) && (!common::parseIsValidName(m_name))) {
        parseLogError(m_protocol.parseLogger()) << ParseXmlWrap::parseLogPrefix(m_node) <<
              "Property \"" << common::parseNameStr() << "\" has unexpected value (" << m_name << ").";
        return false;
    }

    return true;
}

ParseSchemaImpl::ParseNamespacesList ParseSchemaImpl::parseNamespacesList() const
{
    ParseNamespacesList result;
    result.reserve(m_namespaces.size());
    for (auto& n : m_namespaces) {
        result.emplace_back(n.second.get());
    }
    return result;
}

const ParseFieldImpl* ParseSchemaImpl::parseFindField(const std::string& ref, bool checkRef) const
{
    std::string fieldName;
    auto ns = parseGetNsFromPath(ref, checkRef, fieldName);
    if (ns == nullptr) {
        return nullptr;
    }
    return ns->parseFindField(fieldName);
}

const ParseMessageImpl* ParseSchemaImpl::parseFindMessage(const std::string& ref, bool checkRef) const
{
    std::string msgName;
    auto ns = parseGetNsFromPath(ref, checkRef, msgName);
    if (ns == nullptr) {
        return nullptr;
    }
    return ns->parseFindMessage(msgName);
}

const ParseInterfaceImpl* ParseSchemaImpl::parseFindInterface(const std::string& ref, bool checkRef) const
{
    std::string name;
    auto ns = parseGetNsFromPath(ref, checkRef, name);
    if (ns == nullptr) {
        return nullptr;
    }

    return ns->parseFindInterface(name);
}

ParseSchemaImpl::ParseMessagesList ParseSchemaImpl::parseAllMessages() const
{
    auto total =
        std::accumulate(
            m_namespaces.begin(), m_namespaces.end(), static_cast<std::size_t>(0U),
            [](std::size_t soFar, auto& ns) -> std::size_t
            {
                return soFar + ns.second->parseMessages().size();
            });

    ParseNamespaceImpl::ParseMessagesList messages;
    messages.reserve(total);
    for (auto& ns : m_namespaces) {
        auto nsMsgs = ns.second->parseMessagesList();
        messages.insert(messages.end(), nsMsgs.begin(), nsMsgs.end());
    }

    std::sort(
        messages.begin(), messages.end(),
        [](const auto& msg1, const auto& msg2)
        {
            assert(msg1.parseValid());
            assert(msg2.parseValid());
            auto id1 = msg1.parseId();
            auto id2 = msg2.parseId();
            if (id1 != id2) {
                return id1 < id2;
            }

            return msg1.parseOrder() < msg2.parseOrder();
        });

    return messages;
}

ParseSchemaImpl::ParseInterfacesList ParseSchemaImpl::parseAllInterfaces() const
{
    auto total =
        std::accumulate(
            m_namespaces.begin(), m_namespaces.end(), static_cast<std::size_t>(0U),
            [](std::size_t soFar, auto& ns) -> std::size_t
            {
                return soFar + ns.second->parseInterfaces().size();
            });

    ParseNamespaceImpl::ParseInterfacesList interfaces;
    interfaces.reserve(total);
    for (auto& ns : m_namespaces) {
        auto nsMsgs = ns.second->parseInterfacesList();
        interfaces.insert(interfaces.end(), nsMsgs.begin(), nsMsgs.end());
    }

    return interfaces;
}

ParseSchemaImpl::ParseImplInterfacesList ParseSchemaImpl::parseAllImplInterfaces() const
{
    ParseImplInterfacesList interfaces;
    for (auto& ns : m_namespaces) {
        auto nsMsgs = ns.second->parseAllImplInterfaces();
        interfaces.insert(interfaces.end(), nsMsgs.begin(), nsMsgs.end());
    }

    return interfaces;
}

bool ParseSchemaImpl::parseAddPlatform(const std::string& name)
{
    auto platIter = std::lower_bound(m_platforms.begin(), m_platforms.end(), name);
    if ((platIter != m_platforms.end()) && (*platIter == name)) {
        return false;
    }

    m_platforms.insert(platIter, name);
    return true;
}

void ParseSchemaImpl::parseAddNamespace(ParseNamespaceImplPtr ns)
{
    assert(ns->parseGetParent() == this);
    auto& nsName = ns->parseName();
    m_namespaces.insert(std::make_pair(nsName, std::move(ns)));
}

ParseNamespaceImpl& ParseSchemaImpl::parseDefaultNamespace()
{
    auto& globalNsPtr = m_namespaces[common::parseEmptyString()]; // create if needed
    if (!globalNsPtr) {
        globalNsPtr.reset(new ParseNamespaceImpl(nullptr, m_protocol));
        globalNsPtr->parseSetParent(this);
    }
        
    return *globalNsPtr;
}

bool ParseSchemaImpl::parseValidateAllMessages()
{
    return 
        std::all_of(
            m_namespaces.begin(), m_namespaces.end(),
            [this](auto& elem)
            {
                assert(elem.second);
                return elem.second->parseValidateAllMessages(parseNonUniqueMsgIdAllowed());
            });
}

unsigned ParseSchemaImpl::parseCountMessageIds() const
{
    return
        std::accumulate(
            m_namespaces.begin(), m_namespaces.end(), unsigned(0U),
            [](unsigned soFar, auto& n)
            {
                return soFar + n.second->parseCountMessageIds();
            });
}

std::string ParseSchemaImpl::parseExternalRef() const
{
    return common::parseSchemaRefPrefix() + parseName();
}

ParseSchemaImpl::ParseFieldRefInfosList ParseSchemaImpl::parseProcessInterfaceFieldRef(const std::string& refStr) const
{
    ParseFieldRefInfosList result;
    for (auto& nsInfo : m_namespaces) {
        auto nsResult = nsInfo.second->parseProcessInterfaceFieldRef(refStr);
        if (nsResult.empty()) {
            continue;
        }

        result.reserve(result.size() + nsResult.size());
        std::move(nsResult.begin(), nsResult.end(), std::back_inserter(result));
    }

    return result;
}

ParseSchemaImpl::ParseObjKind ParseSchemaImpl::parseObjKindImpl() const
{
    return ParseObjKind::Schema;
}

bool ParseSchemaImpl::parseUpdateStringProperty(const ParsePropsMap& map, const std::string& name, std::string& prop)
{
    auto iter = map.find(name);
    if (iter == map.end()) {
        prop.clear();
        return true;
    }

    prop = iter->second;
    return true;
}

bool ParseSchemaImpl::parseUpdateUnsignedProperty(const ParsePropsMap& map, const std::string& name, unsigned& prop)
{
    auto iter = map.find(name);
    if (iter == map.end()) {
        prop = 0U;
        return true;
    }

    bool ok = false;
    unsigned val = common::parseStrToUnsigned(iter->second, &ok);
    if (!ok) {
        parseLogError(m_protocol.parseLogger()) << m_node->doc->URL << ':' << m_node->line <<
            ": Invalid value of \"" << name << "\" property for \"" << m_node->name << "\" element.";
        return false;
    }

    prop = val;
    return true;
}

bool ParseSchemaImpl::parseUpdateEndianProperty(const ParsePropsMap& map, const std::string& name, ParseEndian& prop)
{
    auto& endianStr = common::parseGetStringProp(map, name);
    prop = common::parseEndian(endianStr, ParseEndian_Little);
    if (prop == ParseEndian_NumOfValues) {
        parseLogError(m_protocol.parseLogger()) << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Invalid value of \"" << name << "\" property for \"" << m_node->name << "\" element.";
        return false;
    }

    return true;
}

bool ParseSchemaImpl::parseUpdateBooleanProperty(const ParseSchemaImpl::ParsePropsMap& map, const std::string& name, bool& prop)
{
    auto iter = map.find(name);
    if (iter == map.end()) {
        prop = false;
        return true;
    }

    bool ok = false;
    bool val = common::parseStrToBool(iter->second, &ok);
    if (!ok) {
        parseLogError(m_protocol.parseLogger()) << m_node->doc->URL << ':' << m_node->line <<
            ": Invalid value of \"" << name << "\" property for \"" << m_node->name << "\" element.";
        return false;
    }

    prop = val;
    return true;
}

bool ParseSchemaImpl::parseUpdateExtraAttrs()
{
    m_extraAttrs = ParseXmlWrap::parseGetExtraAttributes(m_node, PropNames, m_protocol);
    return true;
}

bool ParseSchemaImpl::parseUpdateExtraChildren()
{
    static const ParseXmlWrap::ParseNamesList ChildrenNames = parseGetChildrenList();
    m_extraChildren = ParseXmlWrap::parseGetExtraChildren(m_node, ChildrenNames, m_protocol);
    return true;
}

bool ParseSchemaImpl::parseUpdateDisplayName()
{
    auto& propName = common::parseDisplayNameStr();
    if (!parseUpdateStringProperty(m_props, propName, m_displayName)) {
        return false;
    }

    if ((!m_displayName.empty()) && (0U < m_dslVersion) && (m_dslVersion < 7U)) {
        // The check must be explicit here and not via protocol feature check. 
        // The current schema for the protocol object is not set yet.
        parseLogWarning(m_protocol.parseLogger()) << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The property \"" << propName << "\" of schema is not supported for dslVersion=" << 
                m_dslVersion << ".";        
        m_displayName.clear();
    }
    
    return true;
}

const ParseNamespaceImpl* ParseSchemaImpl::parseGetNsFromPath(const std::string& ref, bool checkRef, std::string& remName) const
{
    if (checkRef) {
        if (!common::parseIsValidRefName(ref)) {
            parseLogInfo(m_protocol.parseLogger()) << "Invalid ref name: " << ref;
            return nullptr;
        }
    }
    else {
        assert(common::parseIsValidRefName(ref));
    }


    auto nameSepPos = ref.find_last_of('.');
    const ParseNamespaceImpl* ns = nullptr;
    do {
        if (nameSepPos == std::string::npos) {
            auto iter = m_namespaces.find(common::parseEmptyString());
            if (iter == m_namespaces.end()) {
                return nullptr;
            }

            remName = ref;
            ns = iter->second.get();
            assert(ns != nullptr);
            break;
        }
        
        auto signedNameSepPos = static_cast<std::ptrdiff_t>(nameSepPos);
        remName.assign(ref.begin() + signedNameSepPos + 1, ref.end());
        std::size_t nsNamePos = 0;
        assert(nameSepPos != std::string::npos);
        while (nsNamePos < nameSepPos) {
            auto nextDotPos = ref.find_first_of('.', nsNamePos);
            assert(nextDotPos != std::string::npos);
            std::string nsName(
                    ref.begin() + static_cast<std::ptrdiff_t>(nsNamePos), 
                    ref.begin() + static_cast<std::ptrdiff_t>(nextDotPos));
            if (nsName.empty()) {
                return nullptr;
            }

            auto* nsMap = &m_namespaces;
            if (ns != nullptr) {
                nsMap = &(ns->parseNamespacesMap());
            }

            auto iter = nsMap->find(nsName);
            if (iter == nsMap->end()) {
                return nullptr;
            }

            assert(iter->second);
            ns = iter->second.get();
            nsNamePos = nextDotPos + 1;
        }

    } while (false);

    assert(ns != nullptr);
    return ns;
}

} // namespace parse

} // namespace commsdsl
