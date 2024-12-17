//
// Copyright 2018 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "SchemaImpl.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <numeric>
#include <type_traits>

#include "common.h"
#include "ProtocolImpl.h"
#include "NamespaceImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const XmlWrap::NamesList PropNames = {
    common::nameStr(),
    common::idStr(),
    common::versionStr(),
    common::dslVersionStr(),
    common::endianStr(),
    common::descriptionStr(),
    common::nonUniqueMsgIdAllowedStr()
};

XmlWrap::NamesList getChildrenList()
{
    XmlWrap::NamesList result = PropNames;
    auto& nsNames = NamespaceImpl::expectedChildrenNames();
    result.insert(result.end(), nsNames.begin(), nsNames.end());
    result.push_back(common::platformsStr());
    result.push_back(common::platformStr());
    return result;
}

} // namespace

SchemaImpl::SchemaImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : m_node(node),
    m_protocol(protocol)
{
}

bool SchemaImpl::processNode()
{

    m_props = XmlWrap::parseNodeProps(m_node);
    if (!XmlWrap::parseChildrenAsProps(m_node, PropNames, m_protocol.logger(), m_props)) {
        return false;
    }

    if ((!updateStringProperty(m_props, common::nameStr(), m_name)) ||
        (!updateStringProperty(m_props, common::descriptionStr(), m_description)) ||
        (!updateUnsignedProperty(m_props, common::idStr(), m_id)) ||
        (!updateUnsignedProperty(m_props, common::versionStr(), m_version)) ||
        (!updateUnsignedProperty(m_props, common::dslVersionStr(), m_dslVersion)) ||
        (!updateEndianProperty(m_props, common::endianStr(), m_endian)) ||
        (!updateBooleanProperty(m_props, common::nonUniqueMsgIdAllowedStr(), m_nonUniqueMsgIdAllowed)) ||
        (!updateExtraAttrs()) ||
        (!updateExtraChildren())) {
        return false;
    }

    if ((!m_name.empty()) && (!common::isValidName(m_name))) {
        logError(m_protocol.logger()) << XmlWrap::logPrefix(m_node) <<
              "Property \"" << common::nameStr() << "\" has unexpected value (" << m_name << ").";
        return false;
    }

    return true;
}

SchemaImpl::NamespacesList SchemaImpl::namespacesList() const
{
    NamespacesList result;
    result.reserve(m_namespaces.size());
    for (auto& n : m_namespaces) {
        result.emplace_back(n.second.get());
    }
    return result;
}

const FieldImpl* SchemaImpl::findField(const std::string& ref, bool checkRef) const
{
    std::string fieldName;
    auto ns = getNsFromPath(ref, checkRef, fieldName);
    if (ns == nullptr) {
        return nullptr;
    }
    return ns->findField(fieldName);
}

const MessageImpl* SchemaImpl::findMessage(const std::string& ref, bool checkRef) const
{
    std::string msgName;
    auto ns = getNsFromPath(ref, checkRef, msgName);
    if (ns == nullptr) {
        return nullptr;
    }
    return ns->findMessage(msgName);
}

const InterfaceImpl* SchemaImpl::findInterface(const std::string& ref, bool checkRef) const
{
    std::string name;
    auto ns = getNsFromPath(ref, checkRef, name);
    if (ns == nullptr) {
        return nullptr;
    }

    return ns->findInterface(name);
}

SchemaImpl::MessagesList SchemaImpl::allMessages() const
{
    auto total =
        std::accumulate(
            m_namespaces.begin(), m_namespaces.end(), static_cast<std::size_t>(0U),
            [](std::size_t soFar, auto& ns) -> std::size_t
            {
                return soFar + ns.second->messages().size();
            });

    NamespaceImpl::MessagesList messages;
    messages.reserve(total);
    for (auto& ns : m_namespaces) {
        auto nsMsgs = ns.second->messagesList();
        messages.insert(messages.end(), nsMsgs.begin(), nsMsgs.end());
    }

    std::sort(
        messages.begin(), messages.end(),
        [](const auto& msg1, const auto& msg2)
        {
            assert(msg1.valid());
            assert(msg2.valid());
            auto id1 = msg1.id();
            auto id2 = msg2.id();
            if (id1 != id2) {
                return id1 < id2;
            }

            return msg1.order() < msg2.order();
        });

    return messages;
}

SchemaImpl::InterfacesList SchemaImpl::allInterfaces() const
{
    auto total =
        std::accumulate(
            m_namespaces.begin(), m_namespaces.end(), static_cast<std::size_t>(0U),
            [](std::size_t soFar, auto& ns) -> std::size_t
            {
                return soFar + ns.second->interfaces().size();
            });

    NamespaceImpl::InterfacesList interfaces;
    interfaces.reserve(total);
    for (auto& ns : m_namespaces) {
        auto nsMsgs = ns.second->interfacesList();
        interfaces.insert(interfaces.end(), nsMsgs.begin(), nsMsgs.end());
    }

    return interfaces;
}

SchemaImpl::ImplInterfacesList SchemaImpl::allImplInterfaces() const
{
    ImplInterfacesList interfaces;
    for (auto& ns : m_namespaces) {
        auto nsMsgs = ns.second->allImplInterfaces();
        interfaces.insert(interfaces.end(), nsMsgs.begin(), nsMsgs.end());
    }

    return interfaces;
}

bool SchemaImpl::addPlatform(const std::string& name)
{
    auto platIter = std::lower_bound(m_platforms.begin(), m_platforms.end(), name);
    if ((platIter != m_platforms.end()) && (*platIter == name)) {
        return false;
    }

    m_platforms.insert(platIter, name);
    return true;
}

void SchemaImpl::addNamespace(NamespaceImplPtr ns)
{
    assert(ns->getParent() == this);
    auto& nsName = ns->name();
    m_namespaces.insert(std::make_pair(nsName, std::move(ns)));
}

NamespaceImpl& SchemaImpl::defaultNamespace()
{
    auto& globalNsPtr = m_namespaces[common::emptyString()]; // create if needed
    if (!globalNsPtr) {
        globalNsPtr.reset(new NamespaceImpl(nullptr, m_protocol));
        globalNsPtr->setParent(this);
    }
        
    return *globalNsPtr;
}

bool SchemaImpl::validateAllMessages()
{
    bool allowNonUniquIds = nonUniqueMsgIdAllowed();
    auto allMsgs = allMessages();
    if (allMsgs.empty()) {
        return true;
    }

    for (auto iter = allMsgs.begin(); iter != (allMsgs.end() - 1); ++iter) {
        auto nextIter = iter + 1;
        assert(nextIter != allMsgs.end());

        assert(iter->valid());
        assert(nextIter->valid());
        if (iter->id() != nextIter->id()) {
            continue;
        }

        if (!allowNonUniquIds) {
            logError(m_protocol.logger()) << "Messages \"" << iter->externalRef() << "\" and \"" <<
                          nextIter->externalRef() << "\" have the same id: " << iter->id();
            return false;
        }

        if (iter->order() == nextIter->order()) {
            logError(m_protocol.logger()) << "Messages \"" << iter->externalRef() << "\" and \"" <<
                          nextIter->externalRef() << "\" have the same \"" <<
                          common::idStr() << "\" and \"" << common::orderStr() << "\" values.";
            return false;
        }

        assert(iter->order() < nextIter->order());
    }

    return true;
}

unsigned SchemaImpl::countMessageIds() const
{
    return
        std::accumulate(
            m_namespaces.begin(), m_namespaces.end(), unsigned(0U),
            [](unsigned soFar, auto& n)
            {
                return soFar + n.second->countMessageIds();
            });
}

std::string SchemaImpl::externalRef() const
{
    return common::schemaRefPrefix() + name();
}

SchemaImpl::FieldRefInfosList SchemaImpl::processInterfaceFieldRef(const std::string& refStr) const
{
    FieldRefInfosList result;
    for (auto& nsInfo : m_namespaces) {
        auto nsResult = nsInfo.second->processInterfaceFieldRef(refStr);
        if (nsResult.empty()) {
            continue;
        }

        result.reserve(result.size() + nsResult.size());
        std::move(nsResult.begin(), nsResult.end(), std::back_inserter(result));
    }

    return result;
}

SchemaImpl::ObjKind SchemaImpl::objKindImpl() const
{
    return ObjKind::Schema;
}

bool SchemaImpl::updateStringProperty(const PropsMap& map, const std::string& name, std::string& prop)
{
    auto iter = map.find(name);
    if (iter == map.end()) {
        prop.clear();
        return true;
    }

    prop = iter->second;
    return true;
}

bool SchemaImpl::updateUnsignedProperty(const PropsMap& map, const std::string& name, unsigned& prop)
{
    auto iter = map.find(name);
    if (iter == map.end()) {
        prop = 0U;
        return true;
    }

    bool ok = false;
    unsigned val = common::strToUnsigned(iter->second, &ok);
    if (!ok) {
        logError(m_protocol.logger()) << m_node->doc->URL << ':' << m_node->line <<
            ": Invalid value of \"" << name << "\" property for \"" << m_node->name << "\" element.";
        return false;
    }

    prop = val;
    return true;
}

bool SchemaImpl::updateEndianProperty(const PropsMap& map, const std::string& name, Endian& prop)
{
    auto& endianStr = common::getStringProp(map, name);
    prop = common::parseEndian(endianStr, Endian_Little);
    if (prop == Endian_NumOfValues) {
        logError(m_protocol.logger()) << XmlWrap::logPrefix(m_node) <<
            "Invalid value of \"" << name << "\" property for \"" << m_node->name << "\" element.";
        return false;
    }

    return true;
}

bool SchemaImpl::updateBooleanProperty(const SchemaImpl::PropsMap& map, const std::string& name, bool& prop)
{
    auto iter = map.find(name);
    if (iter == map.end()) {
        prop = false;
        return true;
    }

    bool ok = false;
    bool val = common::strToBool(iter->second, &ok);
    if (!ok) {
        logError(m_protocol.logger()) << m_node->doc->URL << ':' << m_node->line <<
            ": Invalid value of \"" << name << "\" property for \"" << m_node->name << "\" element.";
        return false;
    }

    prop = val;
    return true;
}

bool SchemaImpl::updateExtraAttrs()
{
    m_extraAttrs = XmlWrap::getExtraAttributes(m_node, PropNames, m_protocol);
    return true;
}

bool SchemaImpl::updateExtraChildren()
{
    static const XmlWrap::NamesList ChildrenNames = getChildrenList();
    m_extraChildren = XmlWrap::getExtraChildren(m_node, ChildrenNames, m_protocol);
    return true;
}

const NamespaceImpl* SchemaImpl::getNsFromPath(const std::string& ref, bool checkRef, std::string& remName) const
{
    if (checkRef) {
        if (!common::isValidRefName(ref)) {
            logInfo(m_protocol.logger()) << "Invalid ref name: " << ref;
            return nullptr;
        }
    }
    else {
        assert(common::isValidRefName(ref));
    }


    auto nameSepPos = ref.find_last_of('.');
    const NamespaceImpl* ns = nullptr;
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
                nsMap = &(ns->namespacesMap());
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
