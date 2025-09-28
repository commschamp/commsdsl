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

#include "ParseVariantFieldImpl.h"

#include "ParseProtocolImpl.h"
#include "ParseOptionalFieldImpl.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <numeric>
#include <utility>

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseXmlWrap::ParseNamesList& parseVariantSupportedTypes()
{
    static const ParseXmlWrap::ParseNamesList Names = ParseFieldImpl::parseSupportedTypes();
    return Names;
}

ParseXmlWrap::ParseNamesList parseGetExtraNames()
{
    auto names = parseVariantSupportedTypes();
    names.push_back(common::parseMembersStr());
    return names;
}

} // namespace

ParseVariantFieldImpl::ParseVariantFieldImpl(xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol)
{
}

ParseVariantFieldImpl::ParseVariantFieldImpl(const ParseVariantFieldImpl& other)
  : Base(other),
    m_state(other.m_state)
{
    m_members.reserve(other.m_members.size());
    for (auto& m : other.m_members) {
        m_members.push_back(m->parseClone());
    }
}

ParseVariantFieldImpl::ParseMembers ParseVariantFieldImpl::parseMembersList() const
{
    ParseMembers result;
    result.reserve(m_members.size());
    std::transform(
        m_members.begin(), m_members.end(), std::back_inserter(result),
        [](auto& elem)
        {
            return ParseField(elem.get());
        });
    return result;
}

ParseFieldImpl::ParseKind ParseVariantFieldImpl::parseKindImpl() const
{
    return ParseKind::Variant;
}

ParseFieldImpl::ParsePtr ParseVariantFieldImpl::parseCloneImpl() const
{
    return ParsePtr(new ParseVariantFieldImpl(*this));
}

const ParseXmlWrap::ParseNamesList& ParseVariantFieldImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::ParseNamesList List = {
        common::parseDefaultMemberStr(),
        common::parseDisplayIdxReadOnlyHiddenStr()
    };

    return List;
}

const ParseXmlWrap::ParseNamesList& ParseVariantFieldImpl::parseExtraChildrenNamesImpl() const
{
    static const ParseXmlWrap::ParseNamesList Names = parseGetExtraNames();
    return Names;
}

bool ParseVariantFieldImpl::parseReuseImpl(const ParseFieldImpl& other)
{
    assert(other.parseKind() == parseKind());
    auto& castedOther = static_cast<const ParseVariantFieldImpl&>(other);
    m_state = castedOther.m_state;
    assert(m_members.empty());
    m_members.reserve(castedOther.m_members.size());
    std::transform(
        castedOther.m_members.begin(), castedOther.m_members.end(), std::back_inserter(m_members),
        [](auto& elem)
        {
            return elem->parseClone();
        });
    assert(m_members.size() == castedOther.m_members.size());
    return true;
}

bool ParseVariantFieldImpl::parseReplaceMembersImpl(ParseFieldsList& members)
{
    for (auto& mem : members) {
        assert(mem);
        auto iter =
            std::find_if(
                m_members.begin(), m_members.end(),
                [&mem](auto& currMem)
                {
                    assert(currMem);
                    return mem->parseName() == currMem->parseName();
                });

        if (iter == m_members.end()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(mem->parseGetNode()) <<
                "Cannot find reused member with name \"" << mem->parseName() << "\" to replace.";
            return false;
        }

        (*iter) = std::move(mem);
    }

    return true;
}

bool ParseVariantFieldImpl::parseImpl()
{
    return
        parseUpdateMembers() &&
        parseUpdateDefaultMember() &&
        parseUpdateIdxHidden();
}

std::size_t ParseVariantFieldImpl::parseMinLengthImpl() const
{
    return 0U;
}

std::size_t ParseVariantFieldImpl::parseMaxLengthImpl() const
{
    auto maxLen = common::parseMaxPossibleLength();
    std::size_t sum = 0U;
    for (auto& m : m_members) {
        auto val = m->parseMaxLength();
        if (val == maxLen) {
            return val;
        }

        sum = std::max(val, sum);
    }

    return sum;
}

bool ParseVariantFieldImpl::parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    return parseStrToNumericOnFields(ref, m_members, val, isBigUnsigned);
}

bool ParseVariantFieldImpl::parseStrToFpImpl(const std::string& ref, double& val) const
{
    return parseStrToFpOnFields(ref, m_members, val);
}

bool ParseVariantFieldImpl::parseStrToBoolImpl(const std::string& ref, bool& val) const
{
    return parseStrToBoolOnFields(ref, m_members, val);
}

bool ParseVariantFieldImpl::parseStrToStringImpl(const std::string& ref, std::string& val) const
{
    return parseStrToStringOnFields(ref, m_members, val);
}

bool ParseVariantFieldImpl::parseStrToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const
{
    return parseStrToDataOnFields(ref, m_members, val);
}

const ParseVariantFieldImpl::ParseFieldsList& ParseVariantFieldImpl::parseMembersImpl() const
{
    return m_members;
}

bool ParseVariantFieldImpl::parseUpdateMembers()
{
    if (!m_members.empty()) {
        m_members.erase(
            std::remove_if(
                m_members.begin(), m_members.end(),
                [this](auto& elem)
                {
                    return
                        (elem->parseIsDeprecatedRemoved()) &&
                        (elem->parseGetDeprecated() <= this->parseGetSinceVersion());
                }),
            m_members.end());

        for (auto& m : m_members) {
            m->parseSetSinceVersion(std::max(parseGetSinceVersion(), m->parseGetSinceVersion()));
        }
    }

    do {
        auto membersNodes = ParseXmlWrap::parseGetChildren(parseGetNode(), common::parseMembersStr());
        if (1U < membersNodes.size()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                          "Only single \"" << common::parseMembersStr() << "\" child element is "
                          "supported for \"" << common::parseVariantStr() << "\".";
            return false;
        }

        auto memberFieldsTypes = ParseXmlWrap::parseGetChildren(parseGetNode(), parseVariantSupportedTypes());
        if ((0U < membersNodes.size()) && (0U < memberFieldsTypes.size())) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                          "The \"" << common::parseVariantStr() << "\" element does not support "
                          "list of stand alone member fields as child elements together with \"" <<
                          common::parseMembersStr() << "\" child element.";
            return false;
        }

        if ((0U == membersNodes.size()) && (0U == memberFieldsTypes.size())) {
            if (m_members.empty()) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                              "The \"" << common::parseVariantStr() << "\" must contain member fields.";
                return false;
            }

            break;
        }

        if ((0U < memberFieldsTypes.size())) {
            assert(0U == membersNodes.size());
            auto allChildren = ParseXmlWrap::parseGetChildren(parseGetNode());
            if (allChildren.size() != memberFieldsTypes.size()) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                              "The member types of \"" << common::parseVariantStr() <<
                              "\" must be defined inside \"<" << common::parseMembersStr() << ">\" child element "
                              "when there are other property describing children.";
                return false;
            }
        }

        if (0U < membersNodes.size()) {
            assert(0U == memberFieldsTypes.size());
            memberFieldsTypes = ParseXmlWrap::parseGetChildren(membersNodes.front());
            auto cleanMemberFieldsTypes = ParseXmlWrap::parseGetChildren(membersNodes.front(), parseVariantSupportedTypes());
            if (cleanMemberFieldsTypes.size() != memberFieldsTypes.size()) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(membersNodes.front()) <<
                              "The \"" << common::parseMembersStr() << "\" child node of \"" <<
                              common::parseVariantStr() << "\" element must contain only supported types.";
                return false;
            }

            // memberFieldsTypes is updated with the list from <members>
        }

        m_members.reserve(m_members.size() + memberFieldsTypes.size());
        for (auto* memNode : memberFieldsTypes) {
            std::string memKind(reinterpret_cast<const char*>(memNode->name));
            auto mem = ParseFieldImpl::parseCreate(memKind, memNode, parseProtocol());
            if (!mem) {
                [[maybe_unused]] static constexpr bool Should_not_happen = false;
                assert(Should_not_happen);
                parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                              "Internal error, failed to create objects for member fields.";
                return false;
            }

            mem->parseSetParent(this);
            if (!mem->parse()) {
                return false;
            }

            // if (!mem->parseVerifySiblings(m_members)) {
            //     return false;
            // }

            m_members.push_back(std::move(mem));
        }

        if (!parseValidateMembersNames(m_members)) {
            return false;
        }

    } while (false);

    bool hasSameVer =
        std::any_of(
            m_members.begin(), m_members.end(),
            [this](auto& m)
            {
                return m->parseGetSinceVersion() == this->parseGetSinceVersion();
            });

    if (!hasSameVer) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "There must be at least one member with the same version as the parent variant.";
        return false;
    }

    return true;
}

bool ParseVariantFieldImpl::parseUpdateDefaultMember()
{
    auto& propName = common::parseDefaultMemberStr();
    if (!parseValidateSinglePropInstance(propName)) {
        return false;
    }

    auto& valueStr = common::parseGetStringProp(parseProps(), propName);
    if (valueStr.empty()) {
        return true;
    }

    if (common::parseIsValidName(valueStr)) {
        auto iter =
            std::find_if(
                m_members.begin(), m_members.end(),
                [&valueStr](auto& m)
                {
                    return valueStr == m->parseName();
                });

        if (iter == m_members.end()) {
            parseReportUnexpectedPropertyValue(propName, valueStr);
            return false;
        }

        m_state.m_defaultIdx =
            static_cast<decltype(m_state.m_defaultIdx)>(
                std::distance(m_members.begin(), iter));

        return true;
    }

    std::intmax_t val = 0U;
    do {
        if (common::parseIsValidRefName(valueStr)) {
            if (parseProtocol().parseStrToEnumValue(valueStr, val, false)) {
                break;
            }

            parseReportUnexpectedPropertyValue(propName, valueStr);
            return false;
        }

        bool ok = false;
        val = common::parseStrToIntMax(valueStr, &ok);
        if (!ok) {
            parseReportUnexpectedPropertyValue(propName, valueStr);
            return false;
        }
    } while (false);

    if (val < 0) {
        m_state.m_defaultIdx = std::numeric_limits<std::size_t>::max();
        return true;
    }

    if (m_members.size() <= static_cast<std::size_t>(val)) {
        parseReportUnexpectedPropertyValue(propName, valueStr);
        return false;
    }

    m_state.m_defaultIdx = static_cast<std::size_t>(val);
    return true;
}

bool ParseVariantFieldImpl::parseUpdateIdxHidden()
{
    parseCheckAndReportDeprecatedPropertyValue(common::parseDisplayIdxReadOnlyHiddenStr());
    return true;
}

} // namespace parse

} // namespace commsdsl
