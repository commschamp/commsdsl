#include "NamespaceImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <cassert>

#include "common.h"
#include "ProtocolImpl.h"

namespace bbmp
{

namespace
{

static const XmlWrap::NamesList ChildrenNames = {
    common::fieldsStr(),
    common::messagesStr(),
    common::messageStr(),
    common::framesStr(),
    common::frameStr(),
    common::nsStr()
};

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

bool NamespaceImpl::parseProps()
{
    assert (m_node != nullptr);

    static const XmlWrap::NamesList Names = {
        common::nameStr(),
        common::descriptionStr()
    };

    m_props = XmlWrap::parseNodeProps(m_node);
    if (!XmlWrap::parseChildrenAsProps(m_node, Names, m_protocol.logger(), m_props)) {
        return false;
    }

    if ((!updateStringProperty(m_props, common::nameStr(), m_name)) ||
        (!updateStringProperty(m_props, common::descriptionStr(), m_description))) {
        return false;
    }

    if (!common::isValidName(m_name)) {
        logError() << XmlWrap::logPrefix(m_node) <<
              "Property \"" << common::nameStr() << "\" has unexpected value (" << m_name << ").";
        return false;
    }

    m_unknownAttrs = XmlWrap::getUnknownProps(m_node, Names);

    m_unknownChildren = XmlWrap::getUnknownChildrenContents(m_node, ChildrenNames);
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

bool NamespaceImpl::merge(NamespaceImpl& other)
{
    assert(m_name == other.m_name);
    if (m_description.empty()) {
        m_description = std::move(other.m_description);
    }

    for (auto& n : other.m_namespaces) {
        auto iter = m_namespaces.find(n.first);
        if (iter == m_namespaces.end()) {
            m_namespaces.emplace(n.first, std::move(n.second));
            continue;
        }

        assert(iter->second);
        if (!iter->second->merge(*n.second)) {
            return false;
        }
    }

    other.m_namespaces.clear();
    other.m_namespacesList.clear();

    for (auto& f : other.m_fields) {
        auto iter = m_fields.find(f.first);
        if (iter != m_fields.end()) {
            assert(f.second);
            assert(iter->second);
            logError() << XmlWrap::logPrefix(f.second->getNode()) <<
                "Field with name \"" << f.first << "\" has been defined earlier at " <<
                XmlWrap::logPrefix(iter->second->getNode());
            return false;
        }

        m_fields.insert(std::move(f));
    }
    other.m_fields.clear();
    other.m_fieldsList.clear();


    // TODO: merge messages and frames
    return true;
}

bool NamespaceImpl::finalise()
{
    m_namespacesList.clear();
    m_namespacesList.reserve(m_namespaces.size());
    for (auto& n : m_namespaces) {
        assert(n.second);
        m_namespacesList.emplace_back(n.second.get());
    }

    m_fieldsList.clear();
    m_fieldsList.reserve(m_fields.size());
    for (auto& f : m_fields) {
        assert(f.second);
        m_fieldsList.emplace_back(f.second.get());
    }

    // TODO: finalise messages and frames
    return true;
}

const XmlWrap::NamesList& NamespaceImpl::supportedChildren()
{
    return ChildrenNames;
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

    auto iter = m_namespaces.find(ns->name());
    if (iter != m_namespaces.end()) {
        logError() << XmlWrap::logPrefix(ns->getNode()) <<
                      "Namespace with the same local name (" << ns->name() << ") "
                      "was already defined.";
        return false;
    }

    auto& nameStr = ns->name();
    m_namespaces.emplace(nameStr, std::move(ns));

    iter = m_namespaces.find(nameStr);
    assert(iter != m_namespaces.end());
    assert(iter->second);
    return iter->second->parseChildren();
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
            logError() << XmlWrap::logPrefix(c) << "Parsing of \"" << cName << "\" has failed.";
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
    static_cast<void>(node);
    // TODO:
    logError() << __FUNCTION__ << ": NYI!";
    return false;
}

bool NamespaceImpl::processMultipleMessages(::xmlNodePtr node)
{
    static_cast<void>(node);
    // TODO:
    logError() << __FUNCTION__ << ": NYI!";
    return false;
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

LogWrapper NamespaceImpl::logError() const
{
    return bbmp::logError(m_protocol.logger());
}

LogWrapper NamespaceImpl::logWarning() const
{
    return bbmp::logWarning(m_protocol.logger());
}

LogWrapper NamespaceImpl::logInfo() const
{
    return bbmp::logInfo(m_protocol.logger());
}


} // namespace bbmp
