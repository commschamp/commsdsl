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

#include "InterfaceImpl.h"

#include <cassert>
#include <limits>
#include <algorithm>
#include <set>
#include <numeric>
#include <iterator>

#include "ProtocolImpl.h"
#include "NamespaceImpl.h"
#include "common.h"
#include "OptionalFieldImpl.h"

namespace commsdsl
{

namespace
{

const XmlWrap::NamesList& interfaceSupportedTypes()
{
    static const XmlWrap::NamesList Names = FieldImpl::supportedTypes();
    return Names;
}


} // namespace

InterfaceImpl::InterfaceImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : m_node(node),
    m_protocol(protocol),
    m_name(&common::emptyString()),
    m_description(&common::emptyString())
{
}

bool InterfaceImpl::parse()
{
    m_props = XmlWrap::parseNodeProps(m_node);

    if (!XmlWrap::parseChildrenAsProps(m_node, commonProps(), m_protocol.logger(), m_props)) {
        return false;
    }

    return
        updateName() &&
        updateDescription() &&
        copyFields() &&
        updateFields() &&
        updateExtraAttrs() &&
        updateExtraChildren();
}

const std::string& InterfaceImpl::name() const
{
    assert(m_name != nullptr);
    return *m_name;
}

const std::string& InterfaceImpl::description() const
{
    assert(m_description != nullptr);
    return *m_description;
}

InterfaceImpl::FieldsList InterfaceImpl::fieldsList() const
{
    FieldsList result;
    result.reserve(m_fields.size());
    std::transform(
        m_fields.begin(), m_fields.end(), std::back_inserter(result),
        [](auto& f)
        {
            return Field(f.get());
        });
    return result;
}

std::string InterfaceImpl::externalRef() const
{
    assert(getParent() != nullptr);
    assert(getParent()->objKind() == ObjKind::Namespace);

    auto& ns = static_cast<const commsdsl::NamespaceImpl&>(*getParent());
    auto nsRef = ns.externalRef();
    if (nsRef.empty()) {
        return name();
    }

    return nsRef + '.' + name();
}

std::size_t InterfaceImpl::findFieldIdx(const std::string& name) const
{
    auto iter =
        std::find_if(
            m_fields.begin(), m_fields.end(),
            [&name](auto& fPtr)
            {
                return fPtr->name() == name;
            });
    if (iter == m_fields.end()) {
        return std::numeric_limits<std::size_t>::max();
    }

    return static_cast<std::size_t>(std::distance(m_fields.begin(), iter));
}

Object::ObjKind InterfaceImpl::objKindImpl() const
{
    return ObjKind::Interface;
}

LogWrapper InterfaceImpl::logError() const
{
    return commsdsl::logError(m_protocol.logger());
}

LogWrapper InterfaceImpl::logWarning() const
{
    return commsdsl::logWarning(m_protocol.logger());
}

LogWrapper InterfaceImpl::logInfo() const
{
    return commsdsl::logInfo(m_protocol.logger());
}

bool InterfaceImpl::validateSinglePropInstance(const std::string& str, bool mustHave)
{
    return XmlWrap::validateSinglePropInstance(m_node, m_props, str, m_protocol.logger(), mustHave);
}

bool InterfaceImpl::validateAndUpdateStringPropValue(
    const std::string& str,
    const std::string*& valuePtr,
    bool mustHave)
{
    if (!validateSinglePropInstance(str, mustHave)) {
        return false;
    }

    auto iter = m_props.find(str);
    if (iter != m_props.end()) {
        valuePtr = &iter->second;
    }

    assert(iter != m_props.end() || (!mustHave));
    return true;
}

void InterfaceImpl::reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue)
{
    XmlWrap::reportUnexpectedPropertyValue(m_node, name(), propName, propValue, m_protocol.logger());
}

const XmlWrap::NamesList& InterfaceImpl::commonProps()
{
    static const XmlWrap::NamesList CommonNames = {
        common::nameStr(),
        common::descriptionStr(),
        common::copyFieldsFromStr(),
    };

    return CommonNames;
}

XmlWrap::NamesList InterfaceImpl::allNames()
{
    auto names = commonProps();
    auto& fieldTypes = interfaceSupportedTypes();
    names.insert(names.end(), fieldTypes.begin(), fieldTypes.end());
    names.push_back(common::fieldsStr());
    return names;
}

bool InterfaceImpl::updateName()
{
    assert(m_name != nullptr);
    bool mustHave = m_name->empty();
    if (!validateAndUpdateStringPropValue(common::nameStr(), m_name, mustHave)) {
        return false;
    }

    if (!common::isValidName(*m_name)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Invalid value for name property \"" << *m_name << "\".";
        return false;
    }

    return true;
}

bool InterfaceImpl::updateDescription()
{
    return validateAndUpdateStringPropValue(common::descriptionStr(), m_description);
}

bool InterfaceImpl::copyFields()
{
    if (!validateSinglePropInstance(common::copyFieldsFromStr())) {
        return false;
    }

    auto iter = props().find(common::copyFieldsFromStr());
    if (iter == props().end()) {
        return true;
    }

    auto* other = m_protocol.findInterface(iter->second);
    if (other == nullptr) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Invalid reference to other interface \"" << iter->second << "\".";
        return false;
    }

    cloneFieldsFrom(*other);
    return true;
}

bool InterfaceImpl::updateFields()
{
    do {
        auto fieldsNodes = XmlWrap::getChildren(getNode(), common::fieldsStr());
        if (1U < fieldsNodes.size()) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                          "Only single \"" << common::fieldsStr() << "\" child element is "
                          "supported for \"" << common::interfaceStr() << "\".";
            return false;
        }

        auto fieldsTypes = XmlWrap::getChildren(getNode(), interfaceSupportedTypes());
        if ((!fieldsNodes.empty()) && (!fieldsTypes.empty())) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                          "The \"" << common::interfaceStr() << "\" element does not support "
                          "list of stand alone fields as child elements together with \"" <<
                          common::fieldsStr() << "\" child element.";
            return false;
        }

        if ((fieldsNodes.empty()) && (fieldsTypes.empty())) {
            break;
        }

        if ((0U < fieldsTypes.size())) {
            assert(0U == fieldsNodes.size());
            auto allChildren = XmlWrap::getChildren(getNode());
            if (allChildren.size() != fieldsTypes.size()) {
                logError() << XmlWrap::logPrefix(getNode()) <<
                              "The field types of \"" << common::interfaceStr() <<
                              "\" must be defined inside \"<" << common::fieldsStr() << ">\" child element "
                              "when there are other property describing children.";
                return false;
            }
        }

        if (0U < fieldsNodes.size()) {
            assert(0U == fieldsTypes.size());
            fieldsTypes = XmlWrap::getChildren(fieldsNodes.front());
            auto cleanMemberFieldsTypes = XmlWrap::getChildren(fieldsNodes.front(), interfaceSupportedTypes());
            if (cleanMemberFieldsTypes.size() != fieldsTypes.size()) {
                logError() << XmlWrap::logPrefix(fieldsNodes.front()) <<
                    "The \"" << common::fieldsStr() << "\" child node of \"" <<
                    common::interfaceStr() << "\" element must contain only supported field types.";
                return false;
            }

            // fieldsTypes is updated with the list from <fields>
        }

        m_fields.reserve(m_fields.size() + fieldsTypes.size());
        for (auto* fNode : fieldsTypes) {
            std::string fKind(reinterpret_cast<const char*>(fNode->name));
            auto field = FieldImpl::create(fKind, fNode, m_protocol);
            if (!field) {
                assert(!"Internal error");
                logError() << XmlWrap::logPrefix(getNode()) <<
                      "Internal error, failed to create objects for member fields.";
                return false;
            }

            field->setParent(this);
            if (!field->parse()) {
                return false;
            }

            if (!field->verifySiblings(m_fields)) {
                return false;
            }

            m_fields.push_back(std::move(field));
        }

        if (!FieldImpl::validateMembersNames(m_fields, m_protocol.logger())) {
            return false;
        }

    } while (false);

    return true;
}

void InterfaceImpl::cloneFieldsFrom(const InterfaceImpl& other)
{
    m_fields.reserve(other.m_fields.size());
    for (auto& f : other.m_fields) {
        m_fields.push_back(f->clone());
    }
}

bool InterfaceImpl::updateExtraAttrs()
{
    m_extraAttrs = XmlWrap::getExtraAttributes(m_node, commonProps(), m_protocol);
    return true;
}

bool InterfaceImpl::updateExtraChildren()
{
    static const XmlWrap::NamesList ChildrenNames = allNames();
    m_extraChildren = XmlWrap::getExtraChildren(m_node, ChildrenNames, m_protocol);
    return true;
}


} // namespace commsdsl
