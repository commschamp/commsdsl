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

#include "ParseNamespaceImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <numeric>

#include "parse_common.h"
#include "ParseProtocolImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

static const ParseXmlWrap::NamesList PropNames = {
    common::nameStr(),
    common::descriptionStr()
};

static const ParseXmlWrap::NamesList ChildrenNames = {
    common::fieldsStr(),
    common::messagesStr(),
    common::messageStr(),
    common::framesStr(),
    common::frameStr(),
    common::nsStr(),
    common::interfacesStr(),
    common::interfaceStr()
};

ParseXmlWrap::NamesList allNames()
{
    ParseXmlWrap::NamesList names = PropNames;
    names.insert(names.end(), ChildrenNames.begin(), ChildrenNames.end());
    return names;
}

bool updateStringProperty(const ParseXmlWrap::PropsMap& map, const std::string& name, std::string& prop)
{
    auto iter = map.find(name);
    if (iter == map.end()) {
        prop.clear();
        return true;
    }

    prop = iter->second;
    return true;
}

} // namespace

ParseNamespaceImpl::ParseNamespaceImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : m_node(node),
    m_protocol(protocol)
{
}

const ParseXmlWrap::NamesList& ParseNamespaceImpl::expectedChildrenNames()
{
    return ChildrenNames;
}

bool ParseNamespaceImpl::parseProps()
{
    assert (m_node != nullptr);

    m_props = ParseXmlWrap::parseNodeProps(m_node);
    if (!ParseXmlWrap::parseChildrenAsProps(m_node, PropNames, m_protocol.logger(), m_props)) {
        return false;
    }

    if ((!updateStringProperty(m_props, common::nameStr(), m_name)) ||
        (!updateStringProperty(m_props, common::descriptionStr(), m_description)) ||
        (!updateExtraAttrs()) ||
        (!updateExtraChildren())) {
        return false;
    }

    if (!common::isValidName(m_name)) {
        logError() << ParseXmlWrap::logPrefix(m_node) <<
              "Property \"" << common::nameStr() << "\" has unexpected value (" << m_name << ").";
        return false;
    }

    return true;
}

bool ParseNamespaceImpl::parseChildren(ParseNamespaceImpl* realNs)
{
    auto children = ParseXmlWrap::getChildren(m_node, ChildrenNames);
    for (auto* c : children) {
        if (!processChild(c, realNs)) {
            return false;
        }
    }
    return true;
}

bool ParseNamespaceImpl::parse()
{
    if (m_node == nullptr) {
        // default namespace
        return true;
    }

    if (!parseProps()) {
        return false;
    }

    return parseChildren();
}

bool ParseNamespaceImpl::processChild(::xmlNodePtr node, ParseNamespaceImpl* realNs)
{
    using ProcessFunc = bool (ParseNamespaceImpl::*)(::xmlNodePtr node);
    static const std::map<std::string, ProcessFunc> ParseFuncMap = {
        std::make_pair(common::nsStr(), &ParseNamespaceImpl::processNamespace),
        std::make_pair(common::fieldsStr(), &ParseNamespaceImpl::processMultipleFields),
        std::make_pair(common::messageStr(), &ParseNamespaceImpl::processMessage),
        std::make_pair(common::messagesStr(), &ParseNamespaceImpl::processMultipleMessages),
        std::make_pair(common::frameStr(), &ParseNamespaceImpl::processFrame),
        std::make_pair(common::framesStr(), &ParseNamespaceImpl::processMultipleFrames),
        std::make_pair(common::interfaceStr(), &ParseNamespaceImpl::processInterface),
        std::make_pair(common::interfacesStr(), &ParseNamespaceImpl::processMultipleInterfaces),
    };

    std::string cName(reinterpret_cast<const char*>(node->name));
    auto iter = ParseFuncMap.find(cName);
    if (iter == ParseFuncMap.end()) {
        return false;
    }

    if (realNs == nullptr) {
        realNs = this;
    }
    auto func = iter->second;
    return (realNs->*func)(node);
}

const ParseXmlWrap::NamesList& ParseNamespaceImpl::supportedChildren()
{
    return ChildrenNames;
}

ParseNamespaceImpl::NamespacesList ParseNamespaceImpl::namespacesList() const
{
    NamespacesList result;
    result.reserve(m_namespaces.size());
    for (auto& n : m_namespaces) {
        assert(n.second);
        result.emplace_back(n.second.get());
    }

    std::sort(
        result.begin(), result.end(),
        [](auto& e1, auto& e2) {
            return e1.name() < e2.name();
        });

    return result;
}

ParseNamespaceImpl::FieldsList ParseNamespaceImpl::fieldsList() const
{
    FieldsList result;
    result.reserve(m_fields.size());
    for (auto& f : m_fields) {
        assert(f.second);
        result.emplace_back(f.second.get());
    }

    std::sort(
        result.begin(), result.end(),
        [](auto& e1, auto& e2) {
            return e1.name() < e2.name();
        });

    return result;
}

ParseNamespaceImpl::MessagesList ParseNamespaceImpl::messagesList() const
{
    MessagesList result;
    result.reserve(m_messages.size());
    for (auto& m : m_messages) {
        assert(m.second);
        result.emplace_back(m.second.get());
    }

    std::sort(
        result.begin(), result.end(),
        [](auto& e1, auto& e2) {
            return e1.name() < e2.name();
        });    
    return result;
}

ParseNamespaceImpl::InterfacesList ParseNamespaceImpl::interfacesList() const
{
    InterfacesList result;
    result.reserve(m_interfaces.size());
    for (auto& m : m_interfaces) {
        assert(m.second);
        result.emplace_back(m.second.get());
    }

    std::sort(
        result.begin(), result.end(),
        [](auto& e1, auto& e2) {
            return e1.name() < e2.name();
        });    
    return result;
}

ParseNamespaceImpl::FramesList ParseNamespaceImpl::framesList() const
{
    FramesList result;
    result.reserve(m_frames.size());
    for (auto& m : m_frames) {
        assert(m.second);
        result.emplace_back(m.second.get());
    }

    std::sort(
        result.begin(), result.end(),
        [](auto& e1, auto& e2) {
            return e1.name() < e2.name();
        });    
    return result;
}

const ParseFieldImpl* ParseNamespaceImpl::findField(const std::string& fieldName) const
{
    auto iter = m_fields.find(fieldName);
    if (iter == m_fields.end()) {
        return nullptr;
    }

    assert(iter->second);
    return iter->second.get();
}

const ParseMessageImpl* ParseNamespaceImpl::findMessage(const std::string& msgName) const
{
    auto iter = m_messages.find(msgName);
    if (iter == m_messages.end()) {
        return nullptr;
    }

    assert(iter->second);
    return iter->second.get();
}

const ParseInterfaceImpl* ParseNamespaceImpl::findInterface(const std::string& intName) const
{
    auto iter = m_interfaces.find(intName);
    if (iter == m_interfaces.end()) {
        return nullptr;
    }

    assert(iter->second);
    return iter->second.get();
}

const ParseFrameImpl* ParseNamespaceImpl::findFrame(const std::string& intName) const
{
    auto iter = m_frames.find(intName);
    if (iter == m_frames.end()) {
        return nullptr;
    }

    assert(iter->second);
    return iter->second.get();
}

std::string ParseNamespaceImpl::externalRef(bool schemaRef) const
{
    assert(getParent() != nullptr);
    assert((getParent()->objKind() == ObjKind::Schema) || (getParent()->objKind() == ObjKind::Namespace));
    if (getParent()->objKind() != ObjKind::Namespace) {
        if (!schemaRef) {
            return name();
        }

        auto& parentSchema  = static_cast<const ParseSchemaImpl&>(*getParent());
        auto result = parentSchema.externalRef();
        auto& nsName = name();
        if (!nsName.empty()) {
            result += '.';
            result += nsName;
        }
        return result;
    }

    auto& parentNs = static_cast<const ParseNamespaceImpl&>(*getParent());
    auto parentRef = parentNs.externalRef(schemaRef);
    assert(!parentRef.empty());
    return parentRef + '.' + name();
}

unsigned ParseNamespaceImpl::countMessageIds() const
{
    unsigned result =
        std::accumulate(
            m_namespaces.begin(), m_namespaces.end(), unsigned(0U),
            [](unsigned soFar, auto& n)
            {
                return soFar + n.second->countMessageIds();
            });

    return std::accumulate(
            m_fields.begin(), m_fields.end(), result,
            [](unsigned soFar, auto& f)
            {
                if (f.second->semanticType() != ParseField::SemanticType::MessageId) {
                    return soFar;
                }

                return soFar + 1U;
            });
}

bool ParseNamespaceImpl::strToNumeric(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    return
        strToValue(
            ref,
            [&val, &isBigUnsigned](const ParseNamespaceImpl& ns, const std::string& str)
            {
                return ns.strToNumeric(str, val, isBigUnsigned);
            },
            [&val, &isBigUnsigned](const ParseFieldImpl& f, const std::string& str)
            {
                return f.strToNumeric(str, val, isBigUnsigned);
            });
}

bool ParseNamespaceImpl::strToFp(const std::string& ref, double& val) const
{
    return
        strToValue(
            ref,
            [&val](const ParseNamespaceImpl& ns, const std::string& str)
            {
                return ns.strToFp(str, val);
            },
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.strToFp(str, val);
            });
}

bool ParseNamespaceImpl::strToBool(const std::string& ref, bool& val) const
{
    return
        strToValue(
            ref,
            [&val](const ParseNamespaceImpl& ns, const std::string& str)
            {
                return ns.strToBool(str, val);
            },
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.strToBool(str, val);
            });
}

bool ParseNamespaceImpl::strToString(const std::string& ref, std::string& val) const
{
    return
        strToValue(
            ref,
            [&val](const ParseNamespaceImpl& ns, const std::string& str)
            {
                return ns.strToString(str, val);
            },
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.strToString(str, val);
            });
}

bool ParseNamespaceImpl::strToData(const std::string& ref, std::vector<std::uint8_t>& val) const
{
    return
        strToValue(
            ref,
            [&val](const ParseNamespaceImpl& ns, const std::string& str)
            {
                return ns.strToData(str, val);
            },
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.strToData(str, val);
            });
}

ParseNamespaceImpl::ImplInterfacesList ParseNamespaceImpl::allImplInterfaces() const
{
    ImplInterfacesList result;
    for (auto& n : m_namespaces) {
        auto ifaces = n.second->allImplInterfaces();
        result.insert(result.end(), ifaces.begin(), ifaces.end());
    }

    result.reserve(result.size() + m_interfaces.size());
    for (auto& i : m_interfaces) {
        result.push_back(i.second.get());
    }

    return result;
}

ParseNamespaceImpl::FieldRefInfosList ParseNamespaceImpl::processInterfaceFieldRef(const std::string& refStr) const
{
    FieldRefInfosList result;
    auto allInterfaces = allImplInterfaces();
    result.reserve(allInterfaces.size());
    for (auto* iface : allInterfaces) {
        auto info = iface->processInnerFieldRef(refStr);
        if (info.m_field != nullptr) {
            result.push_back(std::move(info));
        }
    }
    return result;
}

bool ParseNamespaceImpl::validateAllMessages(bool allowNonUniquIds)
{
    MessagesList allMsgs = messagesList();
    for (auto& ns : m_namespaces) {
        auto nsMsgs = ns.second->messagesList();
        allMsgs.insert(allMsgs.end(), nsMsgs.begin(), nsMsgs.end());
    }

    std::sort(
        allMsgs.begin(), allMsgs.end(),
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
            logError() << "Messages \"" << iter->externalRef() << "\" and \"" <<
                          nextIter->externalRef() << "\" have the same id: " << iter->id();
            return false;
        }

        if (iter->order() == nextIter->order()) {
            logError() << "Messages \"" << iter->externalRef() << "\" and \"" <<
                          nextIter->externalRef() << "\" have the same \"" <<
                          common::idStr() << "\" and \"" << common::orderStr() << "\" values.";
            return false;
        }

        assert(iter->order() < nextIter->order());
    }

    return true;
}

ParseObject::ObjKind ParseNamespaceImpl::objKindImpl() const
{
    return ObjKind::Namespace;
}

bool ParseNamespaceImpl::processNamespace(::xmlNodePtr node)
{
    Ptr ns(new ParseNamespaceImpl(node, m_protocol));
    ns->setParent(this);

    if (!ns->parseProps()) {
        return false;
    }

    auto& nsName = ns->name();
    auto iter = m_namespaces.find(nsName);
    ParseNamespaceImpl* nsToProcess = nullptr;
    ParseNamespaceImpl* realNs = nullptr;
    do {
        if (iter == m_namespaces.end()) {
            m_namespaces.emplace(nsName, std::move(ns));
            iter = m_namespaces.find(nsName);
            assert(iter != m_namespaces.end());
            nsToProcess = iter->second.get();
            break;
        }

        nsToProcess = ns.get();
        realNs = iter->second.get();

        if ((!nsToProcess->description().empty()) &&
            (nsToProcess->description() != realNs->description())) {
            if (realNs->description().empty()) {
                realNs->updateDescription(nsToProcess->description());
            }
            else {
                logWarning() << ParseXmlWrap::logPrefix(nsToProcess->getNode()) <<
                    "Description of namespace \"" << nsToProcess->name() << "\" differs to "
                    "one encountered before.";
            }
        }

        if (!nsToProcess->parseExtraAttributes().empty()) {
            for (auto& a : nsToProcess->parseExtraAttributes()) {
                auto attIter = realNs->parseExtraAttributes().find(a.first);
                if (attIter == realNs->parseExtraAttributes().end()) {
                    realNs->parseExtraAttributes().insert(a);
                }
                else if (a.second != attIter->second) {
                    logWarning() << ParseXmlWrap::logPrefix(nsToProcess->getNode()) <<
                        "Value of attribute \"" << a.first << "\" differs to one defined before.";
                }
            }
        }

        realNs->extraChildren().insert(realNs->extraChildren().end(), nsToProcess->extraChildren().begin(), nsToProcess->extraChildren().end());

    } while (false);

    return nsToProcess->parseChildren(realNs);
}

bool ParseNamespaceImpl::processMultipleFields(::xmlNodePtr node)
{
    auto childrenNodes = ParseXmlWrap::getChildren(node);
    for (auto* c : childrenNodes) {
        std::string cName(reinterpret_cast<const char*>(c->name));
        auto field = ParseFieldImpl::create(cName, c, m_protocol);
        if (!field) {
            logError() << ParseXmlWrap::logPrefix(c) << "Invalid field type \"" << cName << "\"";
            return false;
        }

        field->setParent(this);

        if (!field->parse()) {
            return false;
        }

        auto& name = field->name();
        if (name.empty()) {
            logError() << ParseXmlWrap::logPrefix(c) << "Field \"" << cName << "\" doesn't have any name.";
            return false;
        }

        auto iter = m_fields.find(name);
        if (iter != m_fields.end()) {
            logError() << ParseXmlWrap::logPrefix(c) << "Field with name \"" << name << "\" has been already defined at " <<
                          iter->second->getNode()->doc->URL << ":" << iter->second->getNode()->line << '.';
            return false;
        }

        m_fields.insert(std::make_pair(name, std::move(field)));
    }

    return true;
}

bool ParseNamespaceImpl::processMessage(::xmlNodePtr node)
{
    auto msg = std::make_unique<ParseMessageImpl>(node, m_protocol);
    msg->setParent(this);
    if (!msg->parse()) {
        return false;
    }

    auto msgPtr = findMessage(msg->name());
    auto& msgName = msg->name();
    if (msgPtr != nullptr) {
        logError() << ParseXmlWrap::logPrefix(node) << "Message with name \"" << msgName << "\" has been already defined at " <<
                      msgPtr->getNode()->doc->URL << ":" << msgPtr->getNode()->line << '.';

        return false;
    }

    m_messages.insert(std::make_pair(msgName, std::move(msg)));
    return true;
}

bool ParseNamespaceImpl::processMultipleMessages(::xmlNodePtr node)
{
    auto childrenNodes = ParseXmlWrap::getChildren(node);
    for (auto c : childrenNodes) {
        assert(c != nullptr);
        std::string cName(reinterpret_cast<const char*>(c->name));
        if (cName != common::messageStr()) {
            logError() << ParseXmlWrap::logPrefix(c) <<
                "The \"" << common::messagesStr() << "\" element cannot contain \"" <<
                cName << "\".";
            return false;
        }

        if (!processMessage(c)) {
            return false;
        }
    }
    return true;
}

bool ParseNamespaceImpl::processInterface(::xmlNodePtr node)
{
    auto interface = std::make_unique<ParseInterfaceImpl>(node, m_protocol);
    interface->setParent(this);
    if (!interface->parse()) {
        return false;
    }

    auto intPtr = findInterface(interface->name());
    auto& intName = interface->name();
    if (intPtr != nullptr) {
        logError() << ParseXmlWrap::logPrefix(node) << "Interface with name \"" << intName << "\" has been already defined at " <<
                      intPtr->getNode()->doc->URL << ":" << intPtr->getNode()->line << '.';

        return false;
    }

    m_interfaces.insert(std::make_pair(intName, std::move(interface)));
    return true;
}

bool ParseNamespaceImpl::processMultipleInterfaces(::xmlNodePtr node)
{
    auto childrenNodes = ParseXmlWrap::getChildren(node);
    for (auto c : childrenNodes) {
        assert(c != nullptr);
        std::string cName(reinterpret_cast<const char*>(c->name));
        if (cName != common::interfaceStr()) {
            logError() << ParseXmlWrap::logPrefix(c) <<
                "The \"" << common::interfacesStr() << "\" element cannot contain \"" <<
                cName << "\".";
            return false;
        }

        if (!processInterface(c)) {
            return false;
        }
    }
    return true;
}

bool ParseNamespaceImpl::processFrame(::xmlNodePtr node)
{
    auto frame = std::make_unique<ParseFrameImpl>(node, m_protocol);
    frame->setParent(this);
    if (!frame->parse()) {
        return false;
    }

    auto framePtr = findFrame(frame->name());
    auto& frameName = frame->name();
    if (framePtr != nullptr) {
        logError() << ParseXmlWrap::logPrefix(node) << "Frame with name \"" << frameName << "\" has been already defined at " <<
                      framePtr->getNode()->doc->URL << ":" << framePtr->getNode()->line << '.';

        return false;
    }

    m_frames.insert(std::make_pair(frameName, std::move(frame)));
    return true;
}

bool ParseNamespaceImpl::processMultipleFrames(::xmlNodePtr node)
{
    auto childrenNodes = ParseXmlWrap::getChildren(node);
    for (auto c : childrenNodes) {
        assert(c != nullptr);
        std::string cName(reinterpret_cast<const char*>(c->name));
        if (cName != common::frameStr()) {
            logError() << ParseXmlWrap::logPrefix(c) <<
                "The \"" << common::framesStr() << "\" element cannot contain \"" <<
                cName << "\".";
            return false;
        }

        if (!processFrame(c)) {
            return false;
        }
    }
    return true;
}

bool ParseNamespaceImpl::updateExtraAttrs()
{
    m_extraAttrs = ParseXmlWrap::getExtraAttributes(m_node, PropNames, m_protocol);
    return true;
}

bool ParseNamespaceImpl::updateExtraChildren()
{
    static const ParseXmlWrap::NamesList Names = allNames();
    m_extraChildren = ParseXmlWrap::getExtraChildren(m_node, Names, m_protocol);
    return true;
}

bool ParseNamespaceImpl::strToValue(const std::string& ref, StrToValueNsConvertFunc&& nsFunc, StrToValueFieldConvertFunc&& fFunc) const
{
    auto firstDotPos = ref.find_first_of('.');
    if (firstDotPos == std::string::npos) {
        auto fieldIter = m_fields.find(ref);
        if (fieldIter == m_fields.end()) {
            return false;
        }

        assert(fieldIter->second);
        return fFunc(*fieldIter->second, common::emptyString());
    }

    std::string firstName(ref, 0, firstDotPos);
    std::string restName(ref, firstDotPos + 1);
    assert(!firstName.empty());
    auto nsIter = m_namespaces.find(firstName);
    if (nsIter != m_namespaces.end()) {
        assert(nsIter->second);
        return nsFunc(*nsIter->second, restName);
    }

    auto fieldIter = m_fields.find(firstName);
    if (fieldIter == m_fields.end()) {
        return false;
    }

    assert(fieldIter->second);
    return fFunc(*fieldIter->second, restName);
}

LogWrapper ParseNamespaceImpl::logError() const
{
    return commsdsl::parse::logError(m_protocol.logger());
}

LogWrapper ParseNamespaceImpl::logWarning() const
{
    return commsdsl::parse::logWarning(m_protocol.logger());
}

LogWrapper ParseNamespaceImpl::logInfo() const
{
    return commsdsl::parse::logInfo(m_protocol.logger());
}


} // namespace parse

} // namespace commsdsl
