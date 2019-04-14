//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
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

#include <type_traits>
#include <algorithm>
#include <iterator>

#include "common.h"
#include "ProtocolImpl.h"
#include "NamespaceImpl.h"

namespace commsdsl
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

} // namespace commsdsl
