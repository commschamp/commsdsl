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

static const ParseXmlWrap::ParseNamesList PropNames = {
    common::parseNameStr(),
    common::parseDisplayNameStr(),
    common::parseDescriptionStr()
};

static const ParseXmlWrap::ParseNamesList ChildrenNames = {
    common::parseFieldsStr(),
    common::parseMessagesStr(),
    common::parseMessageStr(),
    common::parseFramesStr(),
    common::parseFrameStr(),
    common::parseNsStr(),
    common::parseInterfacesStr(),
    common::parseInterfaceStr(),
};

ParseXmlWrap::ParseNamesList parseAllNames()
{
    ParseXmlWrap::ParseNamesList names = PropNames;
    names.insert(names.end(), ChildrenNames.begin(), ChildrenNames.end());
    return names;
}

bool parseUpdateStringProperty(const ParseXmlWrap::ParsePropsMap& map, const std::string& name, std::string& prop)
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

const ParseXmlWrap::ParseNamesList& ParseNamespaceImpl::expectedChildrenNames()
{
    return ChildrenNames;
}

bool ParseNamespaceImpl::parseProps()
{
    assert (m_node != nullptr);

    m_props = ParseXmlWrap::parseNodeProps(m_node);
    if (!ParseXmlWrap::parseChildrenAsProps(m_node, PropNames, m_protocol.parseLogger(), m_props)) {
        return false;
    }

    if ((!parseUpdateStringProperty(m_props, common::parseNameStr(), m_name)) ||
        (!parseUpdateStringProperty(m_props, common::parseDescriptionStr(), m_description)) ||
        (!parseUpdateDisplayName()) ||
        (!parseUpdateExtraAttrs()) ||
        (!parseUpdateExtraChildren())) {
        return false;
    }

    if (!common::parseIsValidName(m_name)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(m_node) <<
              "Property \"" << common::parseNameStr() << "\" has unexpected value (" << m_name << ").";
        return false;
    }

    return true;
}

bool ParseNamespaceImpl::parseChildren(ParseNamespaceImpl* realNs)
{
    auto children = ParseXmlWrap::parseGetChildren(m_node, ChildrenNames);
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
        std::make_pair(common::parseNsStr(), &ParseNamespaceImpl::processNamespace),
        std::make_pair(common::parseFieldsStr(), &ParseNamespaceImpl::processMultipleFields),
        std::make_pair(common::parseMessageStr(), &ParseNamespaceImpl::processMessage),
        std::make_pair(common::parseMessagesStr(), &ParseNamespaceImpl::processMultipleMessages),
        std::make_pair(common::parseFrameStr(), &ParseNamespaceImpl::processFrame),
        std::make_pair(common::parseFramesStr(), &ParseNamespaceImpl::processMultipleFrames),
        std::make_pair(common::parseInterfaceStr(), &ParseNamespaceImpl::processInterface),
        std::make_pair(common::parseInterfacesStr(), &ParseNamespaceImpl::processMultipleInterfaces),
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

const ParseXmlWrap::ParseNamesList& ParseNamespaceImpl::parseSupportedChildren()
{
    return ChildrenNames;
}

ParseNamespaceImpl::ParseNamespacesList ParseNamespaceImpl::parseNamespacesList() const
{
    ParseNamespacesList result;
    result.reserve(m_namespaces.size());
    for (auto& n : m_namespaces) {
        assert(n.second);
        result.emplace_back(n.second.get());
    }

    std::sort(
        result.begin(), result.end(),
        [](auto& e1, auto& e2) {
            return e1.parseName() < e2.parseName();
        });

    return result;
}

ParseNamespaceImpl::ParseFieldsList ParseNamespaceImpl::parseFieldsList() const
{
    ParseFieldsList result;
    result.reserve(m_fields.size());
    for (auto& f : m_fields) {
        assert(f.second);
        result.emplace_back(f.second.get());
    }

    std::sort(
        result.begin(), result.end(),
        [](auto& e1, auto& e2) {
            return e1.parseName() < e2.parseName();
        });

    return result;
}

ParseNamespaceImpl::ParseMessagesList ParseNamespaceImpl::parseMessagesList() const
{
    ParseMessagesList result;
    result.reserve(m_messages.size());
    for (auto& m : m_messages) {
        assert(m.second);
        result.emplace_back(m.second.get());
    }

    std::sort(
        result.begin(), result.end(),
        [](auto& e1, auto& e2) {
            return e1.parseName() < e2.parseName();
        });    
    return result;
}

ParseNamespaceImpl::ParseInterfacesList ParseNamespaceImpl::parseInterfacesList() const
{
    ParseInterfacesList result;
    result.reserve(m_interfaces.size());
    for (auto& m : m_interfaces) {
        assert(m.second);
        result.emplace_back(m.second.get());
    }

    std::sort(
        result.begin(), result.end(),
        [](auto& e1, auto& e2) {
            return e1.parseName() < e2.parseName();
        });    
    return result;
}

ParseNamespaceImpl::ParseFramesList ParseNamespaceImpl::parseFramesList() const
{
    ParseFramesList result;
    result.reserve(m_frames.size());
    for (auto& m : m_frames) {
        assert(m.second);
        result.emplace_back(m.second.get());
    }

    std::sort(
        result.begin(), result.end(),
        [](auto& e1, auto& e2) {
            return e1.parseName() < e2.parseName();
        });    
    return result;
}

const ParseFieldImpl* ParseNamespaceImpl::parseFindField(const std::string& fieldName) const
{
    auto iter = m_fields.find(fieldName);
    if (iter == m_fields.end()) {
        return nullptr;
    }

    assert(iter->second);
    return iter->second.get();
}

const ParseMessageImpl* ParseNamespaceImpl::parseFindMessage(const std::string& msgName) const
{
    auto iter = m_messages.find(msgName);
    if (iter == m_messages.end()) {
        return nullptr;
    }

    assert(iter->second);
    return iter->second.get();
}

const ParseInterfaceImpl* ParseNamespaceImpl::parseFindInterface(const std::string& intName) const
{
    auto iter = m_interfaces.find(intName);
    if (iter == m_interfaces.end()) {
        return nullptr;
    }

    assert(iter->second);
    return iter->second.get();
}

const ParseFrameImpl* ParseNamespaceImpl::parseFindFrame(const std::string& intName) const
{
    auto iter = m_frames.find(intName);
    if (iter == m_frames.end()) {
        return nullptr;
    }

    assert(iter->second);
    return iter->second.get();
}

std::string ParseNamespaceImpl::parseExternalRef(bool schemaRef) const
{
    assert(parseGetParent() != nullptr);
    assert((parseGetParent()->parseObjKind() == ParseObjKind::Schema) || (parseGetParent()->parseObjKind() == ParseObjKind::Namespace));
    if (parseGetParent()->parseObjKind() != ParseObjKind::Namespace) {
        if (!schemaRef) {
            return parseName();
        }

        auto& parentSchema  = static_cast<const ParseSchemaImpl&>(*parseGetParent());
        auto result = parentSchema.parseExternalRef();
        auto& nsName = parseName();
        if (!nsName.empty()) {
            result += '.';
            result += nsName;
        }
        return result;
    }

    auto& parentNs = static_cast<const ParseNamespaceImpl&>(*parseGetParent());
    auto parentRef = parentNs.parseExternalRef(schemaRef);
    assert(!parentRef.empty());
    return parentRef + '.' + parseName();
}

unsigned ParseNamespaceImpl::parseCountMessageIds() const
{
    unsigned result =
        std::accumulate(
            m_namespaces.begin(), m_namespaces.end(), unsigned(0U),
            [](unsigned soFar, auto& n)
            {
                return soFar + n.second->parseCountMessageIds();
            });

    return std::accumulate(
            m_fields.begin(), m_fields.end(), result,
            [](unsigned soFar, auto& f)
            {
                if (f.second->parseSemanticType() != ParseField::ParseSemanticType::MessageId) {
                    return soFar;
                }

                return soFar + 1U;
            });
}

bool ParseNamespaceImpl::parseStrToNumeric(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    return
        parseStrToValue(
            ref,
            [&val, &isBigUnsigned](const ParseNamespaceImpl& ns, const std::string& str)
            {
                return ns.parseStrToNumeric(str, val, isBigUnsigned);
            },
            [&val, &isBigUnsigned](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToNumeric(str, val, isBigUnsigned);
            });
}

bool ParseNamespaceImpl::parseStrToFp(const std::string& ref, double& val) const
{
    return
        parseStrToValue(
            ref,
            [&val](const ParseNamespaceImpl& ns, const std::string& str)
            {
                return ns.parseStrToFp(str, val);
            },
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToFp(str, val);
            });
}

bool ParseNamespaceImpl::parseStrToBool(const std::string& ref, bool& val) const
{
    return
        parseStrToValue(
            ref,
            [&val](const ParseNamespaceImpl& ns, const std::string& str)
            {
                return ns.parseStrToBool(str, val);
            },
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToBool(str, val);
            });
}

bool ParseNamespaceImpl::parseStrToString(const std::string& ref, std::string& val) const
{
    return
        parseStrToValue(
            ref,
            [&val](const ParseNamespaceImpl& ns, const std::string& str)
            {
                return ns.parseStrToString(str, val);
            },
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToString(str, val);
            });
}

bool ParseNamespaceImpl::parseStrToData(const std::string& ref, std::vector<std::uint8_t>& val) const
{
    return
        parseStrToValue(
            ref,
            [&val](const ParseNamespaceImpl& ns, const std::string& str)
            {
                return ns.parseStrToData(str, val);
            },
            [&val](const ParseFieldImpl& f, const std::string& str)
            {
                return f.parseStrToData(str, val);
            });
}

ParseNamespaceImpl::ParseImplInterfacesList ParseNamespaceImpl::parseAllImplInterfaces() const
{
    ParseImplInterfacesList result;
    for (auto& n : m_namespaces) {
        auto ifaces = n.second->parseAllImplInterfaces();
        result.insert(result.end(), ifaces.begin(), ifaces.end());
    }

    result.reserve(result.size() + m_interfaces.size());
    for (auto& i : m_interfaces) {
        result.push_back(i.second.get());
    }

    return result;
}

ParseNamespaceImpl::ParseFieldRefInfosList ParseNamespaceImpl::parseProcessInterfaceFieldRef(const std::string& refStr) const
{
    ParseFieldRefInfosList result;
    auto parseAllInterfaces = parseAllImplInterfaces();
    result.reserve(parseAllInterfaces.size());
    for (auto* iface : parseAllInterfaces) {
        auto info = iface->processInnerFieldRef(refStr);
        if (info.m_field != nullptr) {
            result.push_back(std::move(info));
        }
    }
    return result;
}

bool ParseNamespaceImpl::parseValidateAllMessages(bool allowNonUniquIds)
{
    ParseMessagesList allMsgs = parseMessagesList();
    for (auto& ns : m_namespaces) {
        auto nsMsgs = ns.second->parseMessagesList();
        allMsgs.insert(allMsgs.end(), nsMsgs.begin(), nsMsgs.end());
    }

    std::sort(
        allMsgs.begin(), allMsgs.end(),
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

    if (allMsgs.empty()) {
        return true;
    }

    for (auto iter = allMsgs.begin(); iter != (allMsgs.end() - 1); ++iter) {
        auto nextIter = iter + 1;
        assert(nextIter != allMsgs.end());

        assert(iter->parseValid());
        assert(nextIter->parseValid());
        if (iter->parseId() != nextIter->parseId()) {
            continue;
        }

        if (!allowNonUniquIds) {
            parseLogError() << "Messages \"" << iter->parseExternalRef() << "\" and \"" <<
                          nextIter->parseExternalRef() << "\" have the same id: " << iter->parseId();
            return false;
        }

        if (iter->parseOrder() == nextIter->parseOrder()) {
            parseLogError() << "Messages \"" << iter->parseExternalRef() << "\" and \"" <<
                          nextIter->parseExternalRef() << "\" have the same \"" <<
                          common::parseIdStr() << "\" and \"" << common::parseOrderStr() << "\" values.";
            return false;
        }

        assert(iter->parseOrder() < nextIter->parseOrder());
    }

    return true;
}

ParseObject::ParseObjKind ParseNamespaceImpl::parseObjKindImpl() const
{
    return ParseObjKind::Namespace;
}

bool ParseNamespaceImpl::processNamespace(::xmlNodePtr node)
{
    ParsePtr ns(new ParseNamespaceImpl(node, m_protocol));
    ns->parseSetParent(this);

    if (!ns->parseProps()) {
        return false;
    }

    auto& nsName = ns->parseName();
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

        if ((!nsToProcess->parseDescription().empty()) &&
            (nsToProcess->parseDescription() != realNs->parseDescription())) {
            if (realNs->parseDescription().empty()) {
                realNs->parseUpdateDescription(nsToProcess->parseDescription());
            }
            else {
                parseLogWarning() << ParseXmlWrap::parseLogPrefix(nsToProcess->parseGetNode()) <<
                    "Description of namespace \"" << nsToProcess->parseName() << "\" differs to "
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
                    parseLogWarning() << ParseXmlWrap::parseLogPrefix(nsToProcess->parseGetNode()) <<
                        "Value of attribute \"" << a.first << "\" differs to one defined before.";
                }
            }
        }

        realNs->parseExtraChildren().insert(realNs->parseExtraChildren().end(), nsToProcess->parseExtraChildren().begin(), nsToProcess->parseExtraChildren().end());

    } while (false);

    return nsToProcess->parseChildren(realNs);
}

bool ParseNamespaceImpl::processMultipleFields(::xmlNodePtr node)
{
    auto childrenNodes = ParseXmlWrap::parseGetChildren(node);
    for (auto* c : childrenNodes) {
        std::string cName(reinterpret_cast<const char*>(c->name));
        auto field = ParseFieldImpl::parseCreate(cName, c, m_protocol);
        if (!field) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(c) << "Invalid field type \"" << cName << "\"";
            return false;
        }

        field->parseSetParent(this);

        if (!field->parse()) {
            return false;
        }

        auto& name = field->parseName();
        if (name.empty()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(c) << "Field \"" << cName << "\" doesn't have any name.";
            return false;
        }

        auto iter = m_fields.find(name);
        if (iter != m_fields.end()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(c) << "Field with name \"" << name << "\" has been already defined at " <<
                          iter->second->parseGetNode()->doc->URL << ":" << iter->second->parseGetNode()->line << '.';
            return false;
        }

        m_fields.insert(std::make_pair(name, std::move(field)));
    }

    return true;
}

bool ParseNamespaceImpl::processMessage(::xmlNodePtr node)
{
    auto msg = std::make_unique<ParseMessageImpl>(node, m_protocol);
    msg->parseSetParent(this);
    if (!msg->parse()) {
        return false;
    }

    auto msgPtr = parseFindMessage(msg->parseName());
    auto& msgName = msg->parseName();
    if (msgPtr != nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(node) << "Message with name \"" << msgName << "\" has been already defined at " <<
                      msgPtr->parseGetNode()->doc->URL << ":" << msgPtr->parseGetNode()->line << '.';

        return false;
    }

    m_messages.insert(std::make_pair(msgName, std::move(msg)));
    return true;
}

bool ParseNamespaceImpl::processMultipleMessages(::xmlNodePtr node)
{
    auto childrenNodes = ParseXmlWrap::parseGetChildren(node);
    for (auto c : childrenNodes) {
        assert(c != nullptr);
        std::string cName(reinterpret_cast<const char*>(c->name));
        if (cName != common::parseMessageStr()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(c) <<
                "The \"" << common::parseMessagesStr() << "\" element cannot contain \"" <<
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
    interface->parseSetParent(this);
    if (!interface->parse()) {
        return false;
    }

    auto intPtr = parseFindInterface(interface->parseName());
    auto& intName = interface->parseName();
    if (intPtr != nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(node) << "Interface with name \"" << intName << "\" has been already defined at " <<
                      intPtr->parseGetNode()->doc->URL << ":" << intPtr->parseGetNode()->line << '.';

        return false;
    }

    m_interfaces.insert(std::make_pair(intName, std::move(interface)));
    return true;
}

bool ParseNamespaceImpl::processMultipleInterfaces(::xmlNodePtr node)
{
    auto childrenNodes = ParseXmlWrap::parseGetChildren(node);
    for (auto c : childrenNodes) {
        assert(c != nullptr);
        std::string cName(reinterpret_cast<const char*>(c->name));
        if (cName != common::parseInterfaceStr()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(c) <<
                "The \"" << common::parseInterfacesStr() << "\" element cannot contain \"" <<
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
    frame->parseSetParent(this);
    if (!frame->parse()) {
        return false;
    }

    auto framePtr = parseFindFrame(frame->parseName());
    auto& frameName = frame->parseName();
    if (framePtr != nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(node) << "Frame with name \"" << frameName << "\" has been already defined at " <<
                      framePtr->parseGetNode()->doc->URL << ":" << framePtr->parseGetNode()->line << '.';

        return false;
    }

    m_frames.insert(std::make_pair(frameName, std::move(frame)));
    return true;
}

bool ParseNamespaceImpl::processMultipleFrames(::xmlNodePtr node)
{
    auto childrenNodes = ParseXmlWrap::parseGetChildren(node);
    for (auto c : childrenNodes) {
        assert(c != nullptr);
        std::string cName(reinterpret_cast<const char*>(c->name));
        if (cName != common::parseFrameStr()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(c) <<
                "The \"" << common::parseFramesStr() << "\" element cannot contain \"" <<
                cName << "\".";
            return false;
        }

        if (!processFrame(c)) {
            return false;
        }
    }
    return true;
}

bool ParseNamespaceImpl::parseUpdateExtraAttrs()
{
    m_extraAttrs = ParseXmlWrap::parseGetExtraAttributes(m_node, PropNames, m_protocol);
    return true;
}

bool ParseNamespaceImpl::parseUpdateExtraChildren()
{
    static const ParseXmlWrap::ParseNamesList Names = parseAllNames();
    m_extraChildren = ParseXmlWrap::parseGetExtraChildren(m_node, Names, m_protocol);
    return true;
}

bool ParseNamespaceImpl::parseUpdateDisplayName()
{
    auto& propName = common::parseDisplayNameStr();
    if (!parseUpdateStringProperty(m_props, propName, m_displayName)) {
        return false;
    }

    if ((!m_displayName.empty()) && (!m_protocol.parseIsNamespaceDisplayNameSupported())) {
        // The check must be explicit here and not via protocol feature check. 
        // The current schema for the protocol object is not set yet.
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The property \"" << propName << "\" of namespace is not supported for dslVersion=" << 
                m_protocol.parseCurrSchema().parseDslVersion() << ".";        
        m_displayName.clear();
    }
    
    return true;
}

bool ParseNamespaceImpl::parseStrToValue(const std::string& ref, ParseStrToValueNsConvertFunc&& nsFunc, ParseStrToValueFieldConvertFunc&& fFunc) const
{
    auto firstDotPos = ref.find_first_of('.');
    if (firstDotPos == std::string::npos) {
        auto fieldIter = m_fields.find(ref);
        if (fieldIter == m_fields.end()) {
            return false;
        }

        assert(fieldIter->second);
        return fFunc(*fieldIter->second, common::parseEmptyString());
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

ParseLogWrapper ParseNamespaceImpl::parseLogError() const
{
    return commsdsl::parse::parseLogError(m_protocol.parseLogger());
}

ParseLogWrapper ParseNamespaceImpl::parseLogWarning() const
{
    return commsdsl::parse::parseLogWarning(m_protocol.parseLogger());
}

ParseLogWrapper ParseNamespaceImpl::parseLogInfo() const
{
    return commsdsl::parse::parseLogInfo(m_protocol.parseLogger());
}


} // namespace parse

} // namespace commsdsl
