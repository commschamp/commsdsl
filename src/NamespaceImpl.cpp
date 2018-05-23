#include "NamespaceImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <cctype>

#include "common.h"
#include "ProtocolImpl.h"

namespace commsdsl
{

namespace
{

static const XmlWrap::NamesList PropNames = {
    common::nameStr(),
    common::descriptionStr()
};

static const XmlWrap::NamesList ChildrenNames = {
    common::fieldsStr(),
    common::messagesStr(),
    common::messageStr(),
    common::framesStr(),
    common::frameStr(),
    common::nsStr(),
    common::interfacesStr(),
    common::interfaceStr()
};

XmlWrap::NamesList allNames()
{
    XmlWrap::NamesList names = PropNames;
    names.insert(names.end(), ChildrenNames.begin(), ChildrenNames.end());
    return names;
}

bool updateStringProperty(const XmlWrap::PropsMap& map, const std::string& name, std::string& prop)
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

NamespaceImpl::NamespaceImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : m_node(node),
    m_protocol(protocol)
{
}

const XmlWrap::NamesList& NamespaceImpl::expectedChildrenNames()
{
    return ChildrenNames;
}

bool NamespaceImpl::parseProps()
{
    assert (m_node != nullptr);

    m_props = XmlWrap::parseNodeProps(m_node);
    if (!XmlWrap::parseChildrenAsProps(m_node, PropNames, m_protocol.logger(), m_props)) {
        return false;
    }

    if ((!updateStringProperty(m_props, common::nameStr(), m_name)) ||
        (!updateStringProperty(m_props, common::descriptionStr(), m_description)) ||
        (!updateExtraAttrs()) ||
        (!updateExtraChildren())) {
        return false;
    }

    if (!common::isValidName(m_name)) {
        logError() << XmlWrap::logPrefix(m_node) <<
              "Property \"" << common::nameStr() << "\" has unexpected value (" << m_name << ").";
        return false;
    }

    return true;
}

bool NamespaceImpl::parseChildren(NamespaceImpl* realNs)
{
    auto children = XmlWrap::getChildren(m_node, ChildrenNames);
    for (auto* c : children) {
        if (!processChild(c, realNs)) {
            return false;
        }
    }
    return true;
}

bool NamespaceImpl::parse()
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

bool NamespaceImpl::processChild(::xmlNodePtr node, NamespaceImpl* realNs)
{
    using ProcessFunc = bool (NamespaceImpl::*)(::xmlNodePtr node);
    static const std::map<std::string, ProcessFunc> ParseFuncMap = {
        std::make_pair(common::nsStr(), &NamespaceImpl::processNamespace),
        std::make_pair(common::fieldsStr(), &NamespaceImpl::processMultipleFields),
        std::make_pair(common::messageStr(), &NamespaceImpl::processMessage),
        std::make_pair(common::messagesStr(), &NamespaceImpl::processMultipleMessages),
        std::make_pair(common::frameStr(), &NamespaceImpl::processFrame),
        std::make_pair(common::framesStr(), &NamespaceImpl::processMultipleFrames),
        std::make_pair(common::interfaceStr(), &NamespaceImpl::processInterface),
        std::make_pair(common::interfacesStr(), &NamespaceImpl::processMultipleInterfaces),
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

const XmlWrap::NamesList& NamespaceImpl::supportedChildren()
{
    return ChildrenNames;
}

NamespaceImpl::NamespacesList NamespaceImpl::namespacesList() const
{
    NamespacesList result;
    result.reserve(m_namespaces.size());
    for (auto& n : m_namespaces) {
        assert(n.second);
        result.emplace_back(n.second.get());
    }
    return result;
}

NamespaceImpl::FieldsList NamespaceImpl::fieldsList() const
{
    FieldsList result;
    result.reserve(m_fields.size());
    for (auto& f : m_fields) {
        assert(f.second);
        result.emplace_back(f.second.get());
    }
    return result;
}

NamespaceImpl::MessagesList NamespaceImpl::messagesList() const
{
    MessagesList result;
    result.reserve(m_messages.size());
    for (auto& m : m_messages) {
        assert(m.second);
        result.emplace_back(m.second.get());
    }
    return result;
}

NamespaceImpl::InterfacesList NamespaceImpl::interfacesList() const
{
    InterfacesList result;
    result.reserve(m_interfaces.size());
    for (auto& m : m_interfaces) {
        assert(m.second);
        result.emplace_back(m.second.get());
    }
    return result;
}

const FieldImpl* NamespaceImpl::findField(const std::string& fieldName) const
{
    auto iter = m_fields.find(fieldName);
    if (iter == m_fields.end()) {
        return nullptr;
    }

    assert(iter->second);
    return iter->second.get();
}

const MessageImpl* NamespaceImpl::findMessage(const std::string& msgName) const
{
    auto iter = m_messages.find(msgName);
    if (iter == m_messages.end()) {
        return nullptr;
    }

    assert(iter->second);
    return iter->second.get();
}

const InterfaceImpl* NamespaceImpl::findInterface(const std::string& intName) const
{
    auto iter = m_interfaces.find(intName);
    if (iter == m_interfaces.end()) {
        return nullptr;
    }

    assert(iter->second);
    return iter->second.get();
}

std::string NamespaceImpl::externalRef() const
{
    assert((getParent() == nullptr) || (getParent()->objKind() == ObjKind::Namespace));
    if ((getParent() == nullptr) || (getParent()->objKind() != ObjKind::Namespace)) {
        return name();
    }

    auto& parentNs = static_cast<const NamespaceImpl&>(*getParent());
    auto parentRef = parentNs.externalRef();
    assert(!parentRef.empty());
    return parentRef + '.' + name();
}

Object::ObjKind NamespaceImpl::objKindImpl() const
{
    return ObjKind::Namespace;
}

bool NamespaceImpl::processNamespace(::xmlNodePtr node)
{
    Ptr ns(new NamespaceImpl(node, m_protocol));
    ns->setParent(this);

    if (!ns->parseProps()) {
        return false;
    }

    auto& nsName = ns->name();
    auto iter = m_namespaces.find(nsName);
    NamespaceImpl* nsToProcess = nullptr;
    NamespaceImpl* realNs = nullptr;
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
                logWarning() << XmlWrap::logPrefix(nsToProcess->getNode()) <<
                    "Description of namespace \"" << nsToProcess->name() << "\" differs to "
                    "one encountered before.";
            }
        }

        if (!nsToProcess->extraAttributes().empty()) {
            for (auto& a : nsToProcess->extraAttributes()) {
                auto attIter = realNs->extraAttributes().find(a.first);
                if (attIter == realNs->extraAttributes().end()) {
                    realNs->extraAttributes().insert(a);
                }
                else if (a.second != attIter->second) {
                    logWarning() << XmlWrap::logPrefix(nsToProcess->getNode()) <<
                        "Value of attribute \"" << a.first << "\" differs to one defined before.";
                }
            }
        }

        realNs->extraChildren().insert(realNs->extraChildren().end(), nsToProcess->extraChildren().begin(), nsToProcess->extraChildren().end());

    } while (false);

    return nsToProcess->parseChildren(realNs);
}

bool NamespaceImpl::processMultipleFields(::xmlNodePtr node)
{
    auto childrenNodes = XmlWrap::getChildren(node);
    for (auto* c : childrenNodes) {
        std::string cName(reinterpret_cast<const char*>(c->name));
        auto field = FieldImpl::create(cName, c, m_protocol);
        if (!field) {
            logError() << XmlWrap::logPrefix(c) << "Invalid field type \"" << cName << "\"";
            return false;
        }

        field->setParent(this);

        if (!field->parse()) {
            return false;
        }

        auto& name = field->name();
        if (name.empty()) {
            logError() << XmlWrap::logPrefix(c) << "Field \"" << cName << "\" doesn't have any name.";
            return false;
        }

        auto iter = m_fields.find(name);
        if (iter != m_fields.end()) {
            logError() << XmlWrap::logPrefix(c) << "Field with name \"" << name << "\" has been already defined at " <<
                          iter->second->getNode()->doc->URL << ":" << iter->second->getNode()->line << '.';
            return false;
        }

        m_fields.insert(std::make_pair(name, std::move(field)));
    }

    return true;
}

bool NamespaceImpl::processMessage(::xmlNodePtr node)
{
    auto msg = std::make_unique<MessageImpl>(node, m_protocol);
    msg->setParent(this);
    if (!msg->parse()) {
        return false;
    }

    auto msgPtr = findMessage(msg->name());
    auto& msgName = msg->name();
    if (msgPtr != nullptr) {
        logError() << XmlWrap::logPrefix(node) << "Message with name \"" << msgName << "\" has been already defined at " <<
                      msgPtr->getNode()->doc->URL << ":" << msgPtr->getNode()->line << '.';

        return false;
    }

    m_messages.insert(std::make_pair(msgName, std::move(msg)));
    return true;
}

bool NamespaceImpl::processMultipleMessages(::xmlNodePtr node)
{
    auto childrenNodes = XmlWrap::getChildren(node);
    for (auto c : childrenNodes) {
        assert(c != nullptr);
        std::string cName(reinterpret_cast<const char*>(c->name));
        if (cName != common::messageStr()) {
            logError() << XmlWrap::logPrefix(c) <<
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

bool NamespaceImpl::processInterface(::xmlNodePtr node)
{
    auto interface = std::make_unique<InterfaceImpl>(node, m_protocol);
    interface->setParent(this);
    if (!interface->parse()) {
        return false;
    }

    auto intPtr = findInterface(interface->name());
    auto& intName = interface->name();
    if (intPtr != nullptr) {
        logError() << XmlWrap::logPrefix(node) << "Interface with name \"" << intName << "\" has been already defined at " <<
                      intPtr->getNode()->doc->URL << ":" << intPtr->getNode()->line << '.';

        return false;
    }

    m_interfaces.insert(std::make_pair(intName, std::move(interface)));
    return true;
}

bool NamespaceImpl::processMultipleInterfaces(::xmlNodePtr node)
{
    auto childrenNodes = XmlWrap::getChildren(node);
    for (auto c : childrenNodes) {
        assert(c != nullptr);
        std::string cName(reinterpret_cast<const char*>(c->name));
        if (cName != common::interfaceStr()) {
            logError() << XmlWrap::logPrefix(c) <<
                "The \"" << common::interfaceStr() << "\" element cannot contain \"" <<
                cName << "\".";
            return false;
        }

        if (!processInterface(c)) {
            return false;
        }
    }
    return true;
}

bool NamespaceImpl::processFrame(::xmlNodePtr node)
{
    static_cast<void>(node);
    // TODO:
    logError() << __FUNCTION__ << ": NYI!";
    return false;
}

bool NamespaceImpl::processMultipleFrames(::xmlNodePtr node)
{
    static_cast<void>(node);
    // TODO:
    logError() << __FUNCTION__ << ": NYI!";
    return false;
}

bool NamespaceImpl::updateExtraAttrs()
{
    m_extraAttrs = XmlWrap::getExtraAttributes(m_node, PropNames, m_protocol);
    return true;
}

bool NamespaceImpl::updateExtraChildren()
{
    static const XmlWrap::NamesList ChildrenNames = allNames();
    m_extraChildren = XmlWrap::getExtraChildren(m_node, ChildrenNames, m_protocol);
    return true;
}

LogWrapper NamespaceImpl::logError() const
{
    return commsdsl::logError(m_protocol.logger());
}

LogWrapper NamespaceImpl::logWarning() const
{
    return commsdsl::logWarning(m_protocol.logger());
}

LogWrapper NamespaceImpl::logInfo() const
{
    return commsdsl::logInfo(m_protocol.logger());
}


} // namespace commsdsl
