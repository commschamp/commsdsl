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

#include "ParseBundleFieldImpl.h"

#include "ParseOptionalFieldImpl.h"
#include "ParseProtocolImpl.h"

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

const ParseXmlWrap::NamesList& parseBundleSupportedTypes()
{
    static const ParseXmlWrap::NamesList Names = ParseFieldImpl::parseSupportedTypes();
    return Names;
}

ParseXmlWrap::NamesList parseGetExtraNames()
{
    auto names = parseBundleSupportedTypes();
    names.push_back(common::parseMembersStr());
    names.push_back(common::parseAliasStr());
    names.push_back(common::parseValidCondStr());
    return names;
}

} // namespace

ParseBundleFieldImpl::ParseBundleFieldImpl(xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol)
{
}

ParseBundleFieldImpl::ParseBundleFieldImpl(const ParseBundleFieldImpl& other)
  : Base(other)
{
    m_members.reserve(other.m_members.size());
    for (auto& m : other.m_members) {
        m_members.push_back(m->parseClone());
    }
}

ParseBundleFieldImpl::Members ParseBundleFieldImpl::parseMembersList() const
{
    Members result;
    result.reserve(m_members.size());
    std::transform(
        m_members.begin(), m_members.end(), std::back_inserter(result),
        [](auto& elem)
        {
            return ParseField(elem.get());
        });
    return result;
}

ParseBundleFieldImpl::AliasesList ParseBundleFieldImpl::parseAliasesList() const
{
    AliasesList result;
    result.reserve(m_aliases.size());
    std::transform(
        m_aliases.begin(), m_aliases.end(), std::back_inserter(result),
        [](auto& elem)
        {
            return ParseAlias(elem.get());
        });
    return result;
}

ParseFieldImpl::Kind ParseBundleFieldImpl::parseKindImpl() const
{
    return Kind::Bundle;
}

ParseFieldImpl::Ptr ParseBundleFieldImpl::parseCloneImpl() const
{
    return Ptr(new ParseBundleFieldImpl(*this));
}

const ParseXmlWrap::NamesList& ParseBundleFieldImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::parseReuseAliasesStr(),
        common::parseCopyValidCondFromStr(),
    };

    return List;
}

const ParseXmlWrap::NamesList& ParseBundleFieldImpl::parseExtraPossiblePropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::parseValidCondStr(),
    };

    return List;
}

const ParseXmlWrap::NamesList& ParseBundleFieldImpl::parseExtraChildrenNamesImpl() const
{
    static const ParseXmlWrap::NamesList Names = parseGetExtraNames();
    return Names;
}

bool ParseBundleFieldImpl::parseReuseImpl(const ParseFieldImpl& other)
{
    assert(other.parseKind() == parseKind());
    auto& castedOther = static_cast<const ParseBundleFieldImpl&>(other);
    assert(m_members.empty());
    m_members.reserve(castedOther.m_members.size());
    std::transform(
        castedOther.m_members.begin(), castedOther.m_members.end(), std::back_inserter(m_members),
        [](auto& elem)
        {
            return elem->parseClone();
        });
    assert(m_members.size() == castedOther.m_members.size());

    do {
        if (!parseProtocol().parseIsFieldAliasSupported()) {
            break;
        }

        bool reuseAliases = true;
        if (!parseValidateAndUpdateBoolPropValue(common::parseReuseAliasesStr(), reuseAliases)) {
            return false;
        }

        if (!reuseAliases) {
            break;
        }

        m_aliases.reserve(castedOther.m_aliases.size());
        std::transform(
            castedOther.m_aliases.begin(), castedOther.m_aliases.end(), std::back_inserter(m_aliases),
            [](auto& elem)
            {
                return elem->parseClone();
            });
        assert(m_aliases.size() == castedOther.m_aliases.size());
    } while (false);

    if (castedOther.m_validCond) {
        m_validCond = castedOther.m_validCond->parseClone();
    }     

    return true;
}

bool ParseBundleFieldImpl::parseImpl()
{
    return
        parseUpdateMembers() &&
        parseUpdateAliases() &&
        parseCopyValidCond() &&
        parseUpdateSingleValidCond() &&
        parseUpdateMultiValidCond();
}

bool ParseBundleFieldImpl::parseReplaceMembersImpl(FieldsList& members)
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

std::size_t ParseBundleFieldImpl::parseMinLengthImpl() const
{
    return
        std::accumulate(
            m_members.begin(), m_members.end(), static_cast<std::size_t>(0U),
            [](std::size_t soFar, auto& m) -> std::size_t
            {
                return soFar + m->parseMinLength();
            });
}

std::size_t ParseBundleFieldImpl::parseMaxLengthImpl() const
{
    std::size_t sum = 0U;
    for (auto& m : m_members) {
        if (m->parseSemanticType() == SemanticType::Length) {
            return common::parseMaxPossibleLength();
        }

        common::parseAddToLength(m->parseMaxLength(), sum);
    }

    return sum;
}

bool ParseBundleFieldImpl::parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    return parseStrToNumericOnFields(ref, m_members, val, isBigUnsigned);
}

bool ParseBundleFieldImpl::parseStrToFpImpl(const std::string& ref, double& val) const
{
    return parseStrToFpOnFields(ref, m_members, val);
}

bool ParseBundleFieldImpl::parseStrToBoolImpl(const std::string& ref, bool& val) const
{
    return parseStrToBoolOnFields(ref, m_members, val);
}

bool ParseBundleFieldImpl::parseStrToStringImpl(const std::string& ref, std::string& val) const
{
    return parseStrToStringOnFields(ref, m_members, val);
}

bool ParseBundleFieldImpl::parseStrToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const
{
    return parseStrToDataOnFields(ref, m_members, val);
}

bool ParseBundleFieldImpl::parseVerifySemanticTypeImpl([[maybe_unused]] ::xmlNodePtr node, SemanticType type) const
{
    if ((type == SemanticType::Length) &&
        (parseProtocol().parseIsSemanticTypeLengthSupported()) && 
        (parseProtocol().parseIsNonIntSemanticTypeLengthSupported())) {
        return true;
    }

    return false;
}

bool ParseBundleFieldImpl::parseVerifyAliasedMemberImpl(const std::string& fieldName) const
{
    auto dotPos = fieldName.find('.');
    std::string memFieldName(fieldName, 0, dotPos);
    auto iter =
        std::find_if(
            m_members.begin(), m_members.end(),
            [&memFieldName](auto& f)
            {
                return memFieldName == f->parseName();
            });

    if (iter == m_members.end()) {
        return false;
    }

    if (fieldName.size() <= dotPos) {
        return true;
    }

    std::string restFieldName(fieldName, dotPos + 1);
    return (*iter)->parseVerifyAliasedMember(restFieldName);
}

const ParseXmlWrap::NamesList& ParseBundleFieldImpl::parseSupportedMemberTypesImpl() const
{
    return parseBundleSupportedTypes();
}

const ParseBundleFieldImpl::FieldsList& ParseBundleFieldImpl::parseMembersImpl() const
{
    return m_members;
}

bool ParseBundleFieldImpl::parseUpdateMembers()
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
                          "supported for \"" << common::parseBundleStr() << "\".";
            return false;
        }

        auto memberFieldsTypes = ParseXmlWrap::parseGetChildren(parseGetNode(), parseBundleSupportedTypes());
        if ((0U < membersNodes.size()) && (0U < memberFieldsTypes.size())) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                          "The \"" << common::parseBundleStr() << "\" element does not support "
                          "list of stand alone member fields as child elements together with \"" <<
                          common::parseMembersStr() << "\" child element.";
            return false;
        }

        if ((0U == membersNodes.size()) && (0U == memberFieldsTypes.size())) {
            if (m_members.empty()) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                              "The \"" << common::parseBundleStr() << "\" must contain member fields.";
                return false;
            }

            break;
        }

        if ((0U < memberFieldsTypes.size())) {
            assert(0U == membersNodes.size());
            auto allChildren = ParseXmlWrap::parseGetChildren(parseGetNode());
            if (allChildren.size() != memberFieldsTypes.size()) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                              "The member types of \"" << common::parseBundleStr() <<
                              "\" must be defined inside \"<" << common::parseMembersStr() << ">\" child element "
                              "when there are other property describing children.";
                return false;
            }
        }

        if (0U < membersNodes.size()) {
            assert(0U == memberFieldsTypes.size());
            memberFieldsTypes = ParseXmlWrap::parseGetChildren(membersNodes.front());
            auto cleanMemberFieldsTypes = ParseXmlWrap::parseGetChildren(membersNodes.front(), parseBundleSupportedTypes());
            if (cleanMemberFieldsTypes.size() != memberFieldsTypes.size()) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(membersNodes.front()) <<
                              "The \"" << common::parseMembersStr() << "\" child node of \"" <<
                              common::parseBundleStr() << "\" element must contain only supported types.";
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

            if (!mem->parseVerifySiblings(m_members)) {
                return false;
            }

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
            "There must be at least one member with the same version as the parent bundle.";
        return false;
    }

    auto lengthFieldsCount =
        std::count_if(
            m_members.begin(), m_members.end(),
            [](auto& m)
            {
                return m->parseSemanticType() == SemanticType::Length;
            });

    if (1 < lengthFieldsCount) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "No more that single field with semantiType=\"" << common::parseLengthStr() << "\" "
            "is allowed within \"" << common::parseBundleStr() << "\".";
        return false;
    }

    return true;
}

bool ParseBundleFieldImpl::parseUpdateAliases()
{
    if (!m_aliases.empty()) {
        // Remove aliases in case their aliased fields were removed
        m_aliases.erase(
            std::remove_if(
                m_aliases.begin(), m_aliases.end(),
                [this](auto& alias)
                {
                    auto& fieldName = alias->parseFieldName();
                    auto iter =
                        std::find_if(
                            m_members.begin(), m_members.end(),
                            [&fieldName](auto& m)
                            {
                                return m->parseName() == fieldName;
                            });

                    return iter == m_members.end();
                }),
            m_aliases.end());
    }

    auto aliasNodes = ParseXmlWrap::parseGetChildren(parseGetNode(), common::parseAliasStr());

    if (aliasNodes.empty()) {
        return true;
    }

    if (!parseProtocol().parseIsFieldAliasSupported()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(aliasNodes.front()) <<
              "Using \"" << common::parseAliasStr() << "\" nodes for too early \"" <<
              common::parseDslVersionStr() << "\".";
        return false;
    }

    m_aliases.reserve(m_aliases.size() + aliasNodes.size());
    for (auto* aNode : aliasNodes) {
        auto alias = ParseAliasImpl::parseCreate(aNode, parseProtocol());
        if (!alias) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            parseLogError() << ParseXmlWrap::parseLogPrefix(alias->parseGetNode()) <<
                  "Internal error, failed to create objects for member aliases.";
            return false;
        }

        if (!alias->parse()) {
            return false;
        }

        if (!alias->parseVerifyAlias(m_aliases, m_members)) {
            return false;
        }

        m_aliases.push_back(std::move(alias));
    }

    return true;
}

bool ParseBundleFieldImpl::parseUpdateSingleValidCond()
{
    return parseUpdateSingleCondInternal(common::parseValidCondStr(), m_validCond);
}

bool ParseBundleFieldImpl::parseUpdateMultiValidCond()
{
    return parseUpdateMultiCondInternal(common::parseValidCondStr(), m_validCond);
}


bool ParseBundleFieldImpl::parseCopyValidCond()
{
    auto& prop = common::parseCopyValidCondFromStr();
    if (!parseValidateSinglePropInstance(prop)) {
        return false;
    }

    auto& copySrc = common::parseGetStringProp(parseProps(), prop);
    if (copySrc.empty()) {
        return true;
    }

    if (!parseProtocol().parseIsPropertySupported(prop)) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The property \"" << prop << "\" is not supported for dslVersion=" << 
                parseProtocol().parseCurrSchema().parseDslVersion() << ".";        
        return true;
    }  

    auto* field = parseProtocol().parseFindField(copySrc);
    if (field == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Field referenced by \"" << prop << "\" property (" + copySrc + ") is not found.";
        return false;        
    }     

    if (field->parseKind() != parseKind()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot reference field of other cond in property \"" << prop << "\".";
        return false;
    }      

    if (m_validCond) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot use \"" << prop << "\" property when the validity condition is copied from other field by other means";        
        return false;
    }

    auto* otherBundle = static_cast<const ParseBundleFieldImpl*>(field);
    if (!otherBundle->m_validCond) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Field referenced by the \"" << prop << "\" property (" << copySrc << ") does not specify any validity conditions";        
        return true;
    }

    m_validCond = otherBundle->m_validCond->parseClone();
    if (!m_validCond->parseVerify(m_members, parseGetNode(), parseProtocol())) {
        return false;
    }   

    return true;
}

bool ParseBundleFieldImpl::parseUpdateSingleCondInternal(const std::string& prop, ParseOptCondImplPtr& cond)
{
    if (!parseValidateSinglePropInstance(prop)) {
        return false;
    }

    auto iter = parseProps().find(prop);
    if (iter == parseProps().end()) {
        return true;
    }

    if (!parseProtocol().parseIsValidCondSupportedInCompositeFields()) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The property \"" << prop << "\" is not supported for <bundle> field in dslVersion=" << 
                parseProtocol().parseCurrSchema().parseDslVersion() << ".";        
        return true;
    }          

    auto newCond = std::make_unique<ParseOptCondExprImpl>();
    if (!newCond->parse(iter->second, parseGetNode(), parseProtocol())) {
        return false;
    }

    if (newCond->parseHasInterfaceReference()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The condition \"" << prop << "\" in fields cannot reference interface fields.";           
        return false;
    }

    if (!newCond->parseVerify(m_members, parseGetNode(), parseProtocol())) {
        return false;
    }   

    cond = std::move(newCond);
    return true; 
}

bool ParseBundleFieldImpl::parseUpdateMultiCondInternal(const std::string& prop, ParseOptCondImplPtr& cond)
{
    auto condNodes = ParseXmlWrap::parseGetChildren(parseGetNode(), prop, true);
    if (condNodes.empty()) {
        return true;
    }

    if (!parseProtocol().parseIsValidCondSupportedInCompositeFields()) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The property \"" << prop << "\" is not supported for <bundle> field in dslVersion=" << 
                parseProtocol().parseCurrSchema().parseDslVersion() << ".";        
        return true;
    }      

    if (condNodes.size() > 1U) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot use more that one child to the \"" << prop << "\" element.";        
        return false;
    }

    static const ParseXmlWrap::NamesList ElemNames = {
        common::parseAndStr(),
        common::parseOrStr()
    };

    auto condChildren = ParseXmlWrap::parseGetChildren(condNodes.front(), ElemNames);
    if (condChildren.size() != condNodes.size()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Only single \"" << common::parseAndStr() << "\" or \"" << common::parseOrStr() << "\" child of the \"" << prop << "\" element is supported.";           
        return false;
    }    

    auto iter = parseProps().find(prop);
    if (iter != parseProps().end()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(condNodes.front()) <<
            "Only single \"" << prop << "\" property is supported";

        return false;
    }    

    auto newCond = std::make_unique<ParseOptCondListImpl>();
    newCond->parseOverrideCondStr(prop);
    if (!newCond->parse(condChildren.front(), parseProtocol())) {
        return false;
    }

    if (newCond->parseHasInterfaceReference()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(condNodes.front()) <<
            "The condition \"" << prop << "\" in fields cannot reference interface fields.";           
        return false;
    }    

    if (!newCond->parseVerify(m_members, condChildren.front(), parseProtocol())) {
        return false;
    }    

    cond = std::move(newCond);
    return true;
}

} // namespace parse

} // namespace commsdsl
