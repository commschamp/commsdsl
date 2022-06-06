//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "AliasImpl.h"

#include <cassert>
#include <iterator>

#include "common.h"
#include "ProtocolImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const XmlWrap::NamesList& commonProps()
{
    static const XmlWrap::NamesList CommonNames = {
        common::nameStr(),
        common::descriptionStr(),
        common::fieldStr(),
    };

    return CommonNames;
}

} // namespace

AliasImpl::Ptr AliasImpl::clone() const
{
    Ptr ptr(new AliasImpl(m_node, m_protocol));
    ptr->m_state = m_state;
    return ptr;
}

AliasImpl::Ptr AliasImpl::create(::xmlNodePtr node, ProtocolImpl& protocol)
{
    return Ptr(new AliasImpl(node, protocol));
}

bool AliasImpl::parse()
{
    auto props = XmlWrap::parseNodeProps(m_node);

    if (!XmlWrap::parseChildrenAsProps(m_node, commonProps(), m_protocol.logger(), props)) {
        return false;
    }

    bool result =
        updateName(props) &&
        updateDescription(props) &&
        updateFieldName(props);

    if (!result) {
        return false;
    }

    XmlWrap::NamesList expectedProps = commonProps();
    if (!updateExtraAttrs(expectedProps)) {
        return false;
    }

    XmlWrap::NamesList expectedChildren = commonProps();
    if (!updateExtraChildren(expectedChildren)) {
        return false;
    }

    return true;
}

bool AliasImpl::verifyAlias(
    const std::vector<Ptr>& aliases,
    const std::vector<FieldImplPtr>& fields) const
{
    auto& aliasName = name();
    assert(!aliasName.empty());
    auto checkSameNameFunc =
        [&aliasName](const std::string& n) -> bool
        {
            if (n.size() != aliasName.size()) {
                return false;
            }

            if (std::tolower(aliasName[0]) != std::tolower(n[0])) {
                return false;
            }

            return std::equal(aliasName.begin() + 1, aliasName.end(), n.begin() + 1);
        };

    auto fieldSameNameIter =
        std::find_if(
            fields.begin(), fields.end(),
            [&checkSameNameFunc](auto& f)
            {
                return checkSameNameFunc(f->name());
            });

    if (fieldSameNameIter != fields.end()) {
        logError(m_protocol.logger()) << XmlWrap::logPrefix(getNode()) <<
            "Cannot create alias with name \"" << aliasName << "\", because field "
            "with the same name has been already defined.";
        return false;
    }

    auto aliasSameNameIter =
        std::find_if(
            aliases.begin(), aliases.end(),
            [&checkSameNameFunc](auto& a)
            {
                return checkSameNameFunc(a->name());
            });

    if (aliasSameNameIter != aliases.end()) {
        logError(m_protocol.logger()) << XmlWrap::logPrefix(getNode()) <<
            "Cannot create alias with name \"" << aliasName << "\", because other alias "
            "with the same name has been already defined.";
        return false;
    }

    auto& aliasedFieldName = fieldName();
    auto dotPos = aliasedFieldName.find('.');
    std::string firstAliasedFieldName(aliasedFieldName, 0, dotPos);

    auto aliasedFieldIter =
        std::find_if(
            fields.begin(), fields.end(),
            [&firstAliasedFieldName](auto& f)
            {
                return firstAliasedFieldName == f->name();
            });

    auto reportNotFoundFieldFunc =
        [this, &aliasedFieldName]()
        {
            logError(m_protocol.logger()) << XmlWrap::logPrefix(getNode()) <<
                "Aliased field(s) with name \"" << aliasedFieldName << "\", hasn't been found.";
        };

    if (aliasedFieldIter == fields.end()) {
        reportNotFoundFieldFunc();
        return false;
    }

    if (dotPos < aliasedFieldName.size()) {
        std::string restAliasedFieldName(aliasedFieldName, dotPos + 1);
        if (!(*aliasedFieldIter)->verifyAliasedMember(restAliasedFieldName)) {
            reportNotFoundFieldFunc();
            return false;
        }
    }

    return true;
}

bool AliasImpl::updateName(const PropsMap& props)
{
    bool mustHave = m_state.m_name.empty();
    if (!validateAndUpdateStringPropValue(props, common::nameStr(), m_state.m_name, mustHave)) {
        return false;
    }

    if (!common::isValidName(m_state.m_name)) {
        reportUnexpectedPropertyValue(common::nameStr(), m_state.m_name);
        return false;
    }

    return true;
}

bool AliasImpl::updateDescription(const PropsMap& props)
{
    return validateAndUpdateStringPropValue(props, common::descriptionStr(), m_state.m_description, false, true);
}

bool AliasImpl::updateFieldName(const PropsMap& props)
{
    if (!validateAndUpdateStringPropValue(props, common::fieldStr(), m_state.m_fieldName, true)) {
        return false;
    }

    if (m_state.m_fieldName.empty() || (m_state.m_fieldName[0] != '$')) {
        reportUnexpectedPropertyValue(common::fieldStr(), m_state.m_fieldName);
        return false;
    }

    m_state.m_fieldName.erase(m_state.m_fieldName.begin()); // remove '$';

    if (!common::isValidRefName(m_state.m_fieldName)) {
        reportUnexpectedPropertyValue(common::fieldStr(), m_state.m_fieldName);
        return false;
    }

    return true;
}

bool AliasImpl::validateAndUpdateStringPropValue(
    const PropsMap& props,
    const std::string& str,
    std::string& value,
    bool mustHave,
    bool allowDeref)
{
    if (!validateSinglePropInstance(props, str, mustHave)) {
        return false;
    }

    auto iter = props.find(str);
    if (iter == props.end()) {
        assert(!mustHave);
        return true;
    }

    if (!allowDeref) {
        value = iter->second;
        return true;
    }

    if (!m_protocol.strToStringValue(iter->second, value)) {
        reportUnexpectedPropertyValue(str, iter->second);
        return false;
    }

    return true;
}

bool AliasImpl::validateSinglePropInstance(const PropsMap& props, const std::string& str, bool mustHave)
{
    return XmlWrap::validateSinglePropInstance(m_node, props, str, m_protocol.logger(), mustHave);
}

void AliasImpl::reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue)
{
    XmlWrap::reportUnexpectedPropertyValue(m_node, common::aliasStr(), propName, propValue, m_protocol.logger());
}

bool AliasImpl::updateExtraAttrs(const XmlWrap::NamesList& names)
{
    auto extraAttrs = XmlWrap::getExtraAttributes(m_node, names, m_protocol);
    if (extraAttrs.empty()) {
        return true;
    }

    if (m_state.m_extraAttrs.empty()) {
        m_state.m_extraAttrs = std::move(extraAttrs);
        return true;
    }

    std::move(extraAttrs.begin(), extraAttrs.end(), std::inserter(m_state.m_extraAttrs, m_state.m_extraAttrs.end()));
    return true;
}

bool AliasImpl::updateExtraChildren(const XmlWrap::NamesList& names)
{
    auto extraChildren = XmlWrap::getExtraChildren(m_node, names, m_protocol);
    if (extraChildren.empty()) {
        return true;
    }

    if (m_state.m_extraChildren.empty()) {
        m_state.m_extraChildren = std::move(extraChildren);
        return true;
    }

    m_state.m_extraChildren.reserve(m_state.m_extraChildren.size() + extraChildren.size());
    std::move(extraChildren.begin(), extraChildren.end(), std::back_inserter(m_state.m_extraChildren));
    return true;
}

} // namespace parse

} // namespace commsdsl

