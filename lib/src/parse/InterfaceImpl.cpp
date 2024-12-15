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

namespace parse
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
    m_protocol(protocol)
{
}

bool InterfaceImpl::parse()
{
    m_props = XmlWrap::parseNodeProps(m_node);

    if (!XmlWrap::parseChildrenAsProps(m_node, commonProps(), m_protocol.logger(), m_props)) {
        return false;
    }

    return
        checkReuse() &&
        updateName() &&
        updateDescription() &&
        copyFields() &&
        updateFields() &&
        copyAliases() &&
        updateAliases() &&
        updateExtraAttrs() &&
        updateExtraChildren();
}

const std::string& InterfaceImpl::name() const
{
    return m_state.m_name;
}

const std::string& InterfaceImpl::description() const
{
    return m_state.m_description;
}

const std::string& InterfaceImpl::copyCodeFrom() const
{
    return m_state.m_copyCodeFrom;
}

InterfaceImpl::FieldsList InterfaceImpl::fieldsList() const
{
    FieldsList result;
    result.reserve(m_state.m_fields.size());
    std::transform(
        m_state.m_fields.begin(), m_state.m_fields.end(), std::back_inserter(result),
        [](auto& f)
        {
            return Field(f.get());
        });
    return result;
}

InterfaceImpl::AliasesList InterfaceImpl::aliasesList() const
{
    AliasesList result;
    result.reserve(m_state.m_aliases.size());
    std::transform(
        m_state.m_aliases.begin(), m_state.m_aliases.end(), std::back_inserter(result),
        [](auto& a)
        {
            return Alias(a.get());
        });
    return result;
}

std::string InterfaceImpl::externalRef(bool schemaRef) const
{
    assert(getParent() != nullptr);
    assert(getParent()->objKind() == ObjKind::Namespace);

    auto& ns = static_cast<const NamespaceImpl&>(*getParent());
    auto nsRef = ns.externalRef(schemaRef);
    
    if (nsRef.empty()) {
        return name();
    }

    return nsRef + '.' + name();
}

std::size_t InterfaceImpl::findFieldIdx(const std::string& name) const
{
    auto iter =
        std::find_if(
            m_state.m_fields.begin(), m_state.m_fields.end(),
            [&name](auto& fPtr)
            {
                return fPtr->name() == name;
            });
    if (iter == m_state.m_fields.end()) {
        return std::numeric_limits<std::size_t>::max();
    }

    return static_cast<std::size_t>(std::distance(m_state.m_fields.begin(), iter));
}

InterfaceImpl::ImplFieldsList InterfaceImpl::allImplFields() const
{
    ImplFieldsList result;
    result.reserve(m_state.m_fields.size());
    for (auto& fPtr : m_state.m_fields) {
        result.push_back(fPtr.get());
    }

    return result;
}

InterfaceImpl::FieldRefInfo InterfaceImpl::processInnerFieldRef(const std::string refStr) const
{
    if ((!refStr.empty()) && ((refStr[0] == '#') || (refStr[0] == '?'))) {
        auto info = processInnerFieldRef(refStr.substr(1));
        do {
            if ((info.m_field == nullptr) || 
                (info.m_refType != FieldRefType::FieldRefType_Field)) {
                info = FieldRefInfo();
                break;
            }

            if (refStr[0] == '#') {
                info.m_refType = FieldRefType::FieldRefType_Size;
                break;
            }

            assert(refStr[0] == '?');
            info.m_refType = FieldRefType::FieldRefType_Exists;
            break;
        } while (false);

        if ((info.m_field != nullptr) && (!info.m_field->isValidRefType(info.m_refType))) {
            info = FieldRefInfo();
        }

        return info;
    }

    auto dotPos = refStr.find_first_of(".");
    const auto fieldName = refStr.substr(0, dotPos);
    auto iter = 
        std::find_if(
            m_state.m_fields.begin(), m_state.m_fields.end(),
            [&fieldName](auto& fieldPtr)
            {
                return fieldName == fieldPtr->name();
            });

    if (iter == m_state.m_fields.end()) {
        return FieldRefInfo();
    }

    std::string nextRef;
    if (dotPos < refStr.size()) {
        nextRef = refStr.substr(dotPos + 1U);
    }

    return (*iter)->processInnerRef(nextRef);

}

Object::ObjKind InterfaceImpl::objKindImpl() const
{
    return ObjKind::Interface;
}

LogWrapper InterfaceImpl::logError() const
{
    return commsdsl::parse::logError(m_protocol.logger());
}

LogWrapper InterfaceImpl::logWarning() const
{
    return commsdsl::parse::logWarning(m_protocol.logger());
}

LogWrapper InterfaceImpl::logInfo() const
{
    return commsdsl::parse::logInfo(m_protocol.logger());
}

bool InterfaceImpl::validateSinglePropInstance(const std::string& str, bool mustHave)
{
    return XmlWrap::validateSinglePropInstance(m_node, m_props, str, m_protocol.logger(), mustHave);
}

bool InterfaceImpl::validateAndUpdateBoolPropValue(const std::string& propName, bool& value, bool mustHave)
{
    if (!validateSinglePropInstance(propName, mustHave)) {
        return false;
    }

    auto iter = m_props.find(propName);
    if (iter == m_props.end()) {
        return true;
    }

    if (!m_protocol.isPropertySupported(propName)) {
        logWarning() << XmlWrap::logPrefix(m_node) <<
            "Property \"" << common::availableLengthLimitStr() << "\" is not available for dslVersion= " << m_protocol.currSchema().dslVersion();                
        return true;
    }

    bool ok = false;
    value = common::strToBool(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(propName, iter->second);
        return false;
    }

    return true;
}

bool InterfaceImpl::validateAndUpdateStringPropValue(
    const std::string& str,
    std::string& value,
    bool mustHave)
{
    if (!validateSinglePropInstance(str, mustHave)) {
        return false;
    }

    auto iter = m_props.find(str);
    if (iter != m_props.end()) {
        value = iter->second;
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
        common::copyFieldsAliasesStr(),
        common::reuseStr(),
        common::reuseCodeStr(),
    };

    return CommonNames;
}

XmlWrap::NamesList InterfaceImpl::allNames()
{
    auto names = commonProps();
    auto& fieldTypes = interfaceSupportedTypes();
    names.insert(names.end(), fieldTypes.begin(), fieldTypes.end());
    names.push_back(common::fieldsStr());
    names.push_back(common::aliasStr());
    return names;
}

bool InterfaceImpl::checkReuse()
{
    auto& propStr = common::reuseStr();
    if (!validateSinglePropInstance(propStr)) {
        return false;
    }

    auto iter = m_props.find(propStr);
    if (iter == m_props.end()) {
        return true;
    }

    if (!m_protocol.isInterfaceReuseSupported()) {
        logWarning() << XmlWrap::logPrefix(getNode()) <<
            "Property \"" << propStr << "\" is not supported for <interface> in DSL version " << m_protocol.currSchema().dslVersion() << ", ignoring...";
        return true;
    }

    auto& valueStr = iter->second;
    auto* iFace = m_protocol.findInterface(valueStr);
    if (iFace == nullptr) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "The message \"" << valueStr << "\" hasn't been recorded yet.";
        return false;
    }

    assert(iFace != this);
    Base::reuseState(*iFace);
    m_state = iFace->m_state;

    do {
        auto& codeProp = common::reuseCodeStr();
        if (!validateSinglePropInstance(codeProp, false)) {
            return false;
        }

        m_state.m_copyCodeFrom.clear();
        auto codeIter = m_props.find(codeProp);
        if (codeIter == m_props.end()) {
            break;
        }  

        bool copyCode = false;
        if (!validateAndUpdateBoolPropValue(codeProp, copyCode)) {
            return false;
        }

        if (!copyCode) {
            break;
        }

        m_state.m_copyCodeFrom = valueStr; 
    } while (false);    
    return true;
}

bool InterfaceImpl::updateName()
{
    bool mustHave = m_state.m_name.empty();
    if (!validateAndUpdateStringPropValue(common::nameStr(), m_state.m_name, mustHave)) {
        return false;
    }

    if (!common::isValidName(m_state.m_name)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                      "Invalid value for name property \"" << m_state.m_name << "\".";
        return false;
    }

    return true;
}

bool InterfaceImpl::updateDescription()
{
    return validateAndUpdateStringPropValue(common::descriptionStr(), m_state.m_description);
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

    if (!m_state.m_fields.empty()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Copying fields from multiple sources using various properties is not supported";
        return false;
    }    

    do {
        m_copyFieldsFromInterface = m_protocol.findInterface(iter->second);
        if (m_copyFieldsFromInterface != nullptr) {
            cloneFieldsFrom(*m_copyFieldsFromInterface);
            break;
        }

        if (!m_protocol.isCopyFieldsFromBundleSupported()) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                "Invalid reference to other interface \"" << iter->second << "\".";
            return false;            
        }

        auto* copyFromField = m_protocol.findField(iter->second);
        if ((copyFromField == nullptr) || (copyFromField->kind() != Field::Kind::Bundle)) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                "Invalid reference to other interface or bundle \"" << iter->second << "\".";
            return false;
        }        

        m_copyFieldsFromBundle = static_cast<const BundleFieldImpl*>(copyFromField);
        cloneFieldsFrom(*m_copyFieldsFromBundle);
    } while (false);

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

        m_state.m_fields.reserve(m_state.m_fields.size() + fieldsTypes.size());
        for (auto* fNode : fieldsTypes) {
            std::string fKind(reinterpret_cast<const char*>(fNode->name));
            auto field = FieldImpl::create(fKind, fNode, m_protocol);
            if (!field) {
                [[maybe_unused]] static constexpr bool Should_not_happen = false;
                assert(Should_not_happen);
                logError() << XmlWrap::logPrefix(getNode()) <<
                      "Internal error, failed to create objects for member fields.";
                return false;
            }

            field->setParent(this);
            if (!field->parse()) {
                return false;
            }

            if (!field->verifySiblings(m_state.m_fields)) {
                return false;
            }

            m_state.m_fields.push_back(std::move(field));
        }

        if (!FieldImpl::validateMembersNames(m_state.m_fields, m_protocol.logger())) {
            return false;
        }

    } while (false);

    return true;
}

bool InterfaceImpl::copyAliases()
{
    auto& propStr = common::copyFieldsAliasesStr();
    if (!validateSinglePropInstance(propStr)) {
        return false;
    }

    auto iter = props().find(propStr);
    if (iter != props().end() && (!m_protocol.isFieldAliasSupported())) {
        logError() << XmlWrap::logPrefix(m_node) <<
            "Unexpected property \"" << propStr << "\".";
        return false;
    }

    if (!m_protocol.isFieldAliasSupported()) {
        return true;
    }

    bool copyAliases = true;
    if (iter != props().end()) {
        bool ok = false;
        copyAliases = common::strToBool(iter->second, &ok);
        if (!ok) {
            reportUnexpectedPropertyValue(propStr, iter->second);
            return false;
        }
    }

    if (!copyAliases) {
        return true;
    }

    if ((iter != props().end()) && (m_copyFieldsFromInterface == nullptr) && (m_copyFieldsFromBundle == nullptr)) {
        logWarning() << XmlWrap::logPrefix(m_node) <<
            "Property \"" << propStr << "\" is inapplicable without \"" << common::copyFieldsFromStr() << "\".";
        return true;
    }

    if (m_copyFieldsFromInterface != nullptr) {
        cloneAliasesFrom(*m_copyFieldsFromInterface);
    }
    else if (m_copyFieldsFromBundle != nullptr) {
        cloneAliasesFrom(*m_copyFieldsFromBundle);
    }    
    else {
        return true;
    }

    if (!m_state.m_aliases.empty()) {
        m_state.m_aliases.erase(
            std::remove_if(
                m_state.m_aliases.begin(), m_state.m_aliases.end(),
                [this](auto& alias)
                {
                    auto& fieldName = alias->fieldName();
                    assert(!fieldName.empty());
                    auto fieldIter =
                        std::find_if(
                            m_state.m_fields.begin(), m_state.m_fields.end(),
                            [&fieldName](auto& f)
                            {
                                return fieldName == f->name();
                            });

                    return fieldIter == m_state.m_fields.end();
                }),
            m_state.m_aliases.end());
    }
    return true;
}

void InterfaceImpl::cloneFieldsFrom(const InterfaceImpl& other)
{
    m_state.m_fields.reserve(other.m_state.m_fields.size());
    for (auto& f : other.m_state.m_fields) {
        m_state.m_fields.push_back(f->clone());
    }
}

void InterfaceImpl::cloneFieldsFrom(const BundleFieldImpl& other)
{
    m_state.m_fields.reserve(other.members().size());
    for (auto& f : other.members()) {
        m_state.m_fields.push_back(f->clone());
    }
}

void InterfaceImpl::cloneAliasesFrom(const InterfaceImpl& other)
{
    m_state.m_aliases.reserve(other.m_state.m_aliases.size());
    for (auto& a : other.m_state.m_aliases) {
        m_state.m_aliases.push_back(a->clone());
    }
}

void InterfaceImpl::cloneAliasesFrom(const BundleFieldImpl& other)
{
    m_state.m_aliases.reserve(other.aliases().size());
    for (auto& a : other.aliases()) {
        m_state.m_aliases.push_back(a->clone());
    }
}

bool InterfaceImpl::updateAliases()
{
    auto aliasNodes = XmlWrap::getChildren(getNode(), common::aliasStr());

    if (aliasNodes.empty()) {
        return true;
    }

    if (!m_protocol.isFieldAliasSupported()) {
        logError() << XmlWrap::logPrefix(aliasNodes.front()) <<
              "Using \"" << common::aliasStr() << "\" nodes for too early \"" <<
              common::dslVersionStr() << "\".";
        return false;
    }

    m_state.m_aliases.reserve(m_state.m_aliases.size() + aliasNodes.size());
    for (auto* aNode : aliasNodes) {
        auto alias = AliasImpl::create(aNode, m_protocol);
        if (!alias) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            logError() << XmlWrap::logPrefix(alias->getNode()) <<
                  "Internal error, failed to create objects for member aliases.";
            return false;
        }

        if (!alias->parse()) {
            return false;
        }

        if (!alias->verifyAlias(m_state.m_aliases, m_state.m_fields)) {
            return false;
        }

        m_state.m_aliases.push_back(std::move(alias));
    }

    return true;
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


} // namespace parse

} // namespace commsdsl
