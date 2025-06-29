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

#include <algorithm>
#include <cassert>
#include <iterator>
#include <numeric>
#include <type_traits>

#include "parse_common.h"
#include "ParseProtocolImpl.h"
#include "ParseNamespaceImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseXmlWrap::NamesList PropNames = {
    common::nameStr(),
    common::idStr(),
    common::versionStr(),
    common::dslVersionStr(),
    common::endianStr(),
    common::descriptionStr(),
    common::nonUniqueMsgIdAllowedStr()
};

ParseXmlWrap::NamesList parseGetChildrenList()
{
    ParseXmlWrap::NamesList result = PropNames;
    auto& nsNames = ParseNamespaceImpl::expectedChildrenNames();
    result.insert(result.end(), nsNames.begin(), nsNames.end());
    result.push_back(common::platformsStr());
    result.push_back(common::platformStr());
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

    if ((!parseUpdateStringProperty(m_props, common::nameStr(), m_name)) ||
        (!parseUpdateStringProperty(m_props, common::descriptionStr(), m_description)) ||
        (!parseUpdateUnsignedProperty(m_props, common::idStr(), m_id)) ||
        (!parseUpdateUnsignedProperty(m_props, common::versionStr(), m_version)) ||
        (!parseUpdateUnsignedProperty(m_props, common::dslVersionStr(), m_dslVersion)) ||
        (!parseUpdateEndianProperty(m_props, common::endianStr(), m_endian)) ||
        (!parseUpdateBooleanProperty(m_props, common::nonUniqueMsgIdAllowedStr(), m_nonUniqueMsgIdAllowed)) ||
        (!parseUpdateExtraAttrs()) ||
        (!parseUpdateExtraChildren())) {
        return false;
    }

    if ((!m_name.empty()) && (!common::isValidName(m_name))) {
        parseLogError(m_protocol.parseLogger()) << ParseXmlWrap::parseLogPrefix(m_node) <<
              "Property \"" << common::nameStr() << "\" has unexpected value (" << m_name << ").";
        return false;
    }

    return true;
}

ParseSchemaImpl::NamespacesList ParseSchemaImpl::parseNamespacesList() const
{
    NamespacesList result;
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

ParseSchemaImpl::MessagesList ParseSchemaImpl::parseAllMessages() const
{
    auto total =
        std::accumulate(
            m_namespaces.begin(), m_namespaces.end(), static_cast<std::size_t>(0U),
            [](std::size_t soFar, auto& ns) -> std::size_t
            {
                return soFar + ns.second->parseMessages().size();
            });

    ParseNamespaceImpl::MessagesList messages;
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

ParseSchemaImpl::InterfacesList ParseSchemaImpl::parseAllInterfaces() const
{
    auto total =
        std::accumulate(
            m_namespaces.begin(), m_namespaces.end(), static_cast<std::size_t>(0U),
            [](std::size_t soFar, auto& ns) -> std::size_t
            {
                return soFar + ns.second->parseInterfaces().size();
            });

    ParseNamespaceImpl::InterfacesList interfaces;
    interfaces.reserve(total);
    for (auto& ns : m_namespaces) {
        auto nsMsgs = ns.second->parseInterfacesList();
        interfaces.insert(interfaces.end(), nsMsgs.begin(), nsMsgs.end());
    }

    return interfaces;
}

ParseSchemaImpl::ImplInterfacesList ParseSchemaImpl::parseAllImplInterfaces() const
{
    ImplInterfacesList interfaces;
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
    auto& globalNsPtr = m_namespaces[common::emptyString()]; // create if needed
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
    return common::schemaRefPrefix() + parseName();
}

ParseSchemaImpl::FieldRefInfosList ParseSchemaImpl::parseProcessInterfaceFieldRef(const std::string& refStr) const
{
    FieldRefInfosList result;
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

ParseSchemaImpl::ObjKind ParseSchemaImpl::parseObjKindImpl() const
{
    return ObjKind::Schema;
}

bool ParseSchemaImpl::parseUpdateStringProperty(const PropsMap& map, const std::string& name, std::string& prop)
{
    auto iter = map.find(name);
    if (iter == map.end()) {
        prop.clear();
        return true;
    }

    prop = iter->second;
    return true;
}

bool ParseSchemaImpl::parseUpdateUnsignedProperty(const PropsMap& map, const std::string& name, unsigned& prop)
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

bool ParseSchemaImpl::parseUpdateEndianProperty(const PropsMap& map, const std::string& name, ParseEndian& prop)
{
    auto& endianStr = common::getStringProp(map, name);
    prop = common::parseEndian(endianStr, ParseEndian_Little);
    if (prop == ParseEndian_NumOfValues) {
        parseLogError(m_protocol.parseLogger()) << ParseXmlWrap::parseLogPrefix(m_node) <<
            "Invalid value of \"" << name << "\" property for \"" << m_node->name << "\" element.";
        return false;
    }

    return true;
}

bool ParseSchemaImpl::parseUpdateBooleanProperty(const ParseSchemaImpl::PropsMap& map, const std::string& name, bool& prop)
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
    static const ParseXmlWrap::NamesList ChildrenNames = parseGetChildrenList();
    m_extraChildren = ParseXmlWrap::parseGetExtraChildren(m_node, ChildrenNames, m_protocol);
    return true;
}

const ParseNamespaceImpl* ParseSchemaImpl::parseGetNsFromPath(const std::string& ref, bool checkRef, std::string& remName) const
{
    if (checkRef) {
        if (!common::isValidRefName(ref)) {
            parseLogInfo(m_protocol.parseLogger()) << "Invalid ref name: " << ref;
            return nullptr;
        }
    }
    else {
        assert(common::isValidRefName(ref));
    }


    auto nameSepPos = ref.find_last_of('.');
    const ParseNamespaceImpl* ns = nullptr;
    do {
        if (nameSepPos == std::string::npos) {
            auto iter = m_namespaces.find(common::emptyString());
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
