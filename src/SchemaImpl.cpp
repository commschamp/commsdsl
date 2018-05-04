#include "SchemaImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>

#include "common.h"
#include "ProtocolImpl.h"

namespace bbmp
{

namespace
{

const XmlWrap::NamesList PropNames = {
    common::nameStr(),
    common::idStr(),
    common::versionStr(),
    common::endianStr(),
    common::descriptionStr(),
    common::nonUniqueMsgIdAllowedStr()
};

XmlWrap::NamesList getChildrenList()
{
    XmlWrap::NamesList result = PropNames;
    result.push_back(common::fieldsStr());
    result.push_back(common::messagesStr());
    result.push_back(common::messageStr());
    result.push_back(common::frameStr());
    result.push_back(common::framesStr());
    result.push_back(common::nsStr());
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
        (!updateEndianProperty(m_props, common::endianStr(), m_endian)) ||
        (!updateBooleanProperty(m_props, common::nonUniqueMsgIdAllowedStr(), m_nonUniqueMsgIdAllowed)) ||
        (!updateExtraAttrs()) ||
        (!updateExtraChildren())) {
        return false;
    }

    if (!common::isValidName(m_name)) {
        logError(m_protocol.logger()) << XmlWrap::logPrefix(m_node) <<
              "Property \"" << common::nameStr() << "\" has unexpected value (" << m_name << ").";
        return false;
    }

    return true;
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

} // namespace bbmp
