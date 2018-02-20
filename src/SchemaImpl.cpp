#include "SchemaImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>

#include "common.h"

namespace bbmp
{

SchemaImpl::SchemaImpl(::xmlNodePtr node, Logger& logger)
  : m_node(node),
    m_logger(logger)
{
}

bool SchemaImpl::processNode()
{
    static const XmlWrap::NamesList Names = {
        common::nameStr(),
        common::idStr(),
        common::versionStr(),
        common::endianStr(),
        common::descriptionStr()
    };

    m_props = XmlWrap::parseNodeProps(m_node);
    if (!XmlWrap::parseChildrenAsProps(m_node, Names, m_logger, m_props)) {
        return false;
    }

    if ((!updateStringProperty(m_props, common::nameStr(), m_name)) ||
        (!updateStringProperty(m_props, common::descriptionStr(), m_description)) ||
        (!updateUnsignedProperty(m_props, common::idStr(), m_id)) ||
        (!updateUnsignedProperty(m_props, common::versionStr(), m_version)) ||
        (!updateEndianProperty(m_props, common::endianStr(), m_endian))) {
        return false;
    }

    m_unknownAttrs = XmlWrap::getUnknownProps(m_node, Names);

    static const XmlWrap::NamesList ChildrenNames = {
        common::nameStr(),
        common::idStr(),
        common::versionStr(),
        common::endianStr(),
        common::descriptionStr(),
        common::fieldsStr(),
        common::messagesStr(),
        common::messageStr(),
        common::frameStr(),
        common::framesStr()
    };

    m_unknownChildren = XmlWrap::getUnknownChildren(m_node, ChildrenNames);
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
        logError(m_logger) << m_node->doc->URL << ':' << m_node->line <<
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
        logError(m_logger) << XmlWrap::logPrefix(m_node) <<
            "Invalid value of \"" << name << "\" property for \"" << m_node->name << "\" element.";
        return false;
    }

    return true;
}

} // namespace bbmp
