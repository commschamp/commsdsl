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

#include <cassert>
#include <limits>
#include <numeric>
#include <utility>
#include <algorithm>
#include <iterator>

#include "ParseProtocolImpl.h"
#include "ParseOptionalFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseXmlWrap::NamesList& bundleSupportedTypes()
{
    static const ParseXmlWrap::NamesList Names = ParseFieldImpl::supportedTypes();
    return Names;
}

ParseXmlWrap::NamesList getExtraNames()
{
    auto names = bundleSupportedTypes();
    names.push_back(common::membersStr());
    names.push_back(common::aliasStr());
    names.push_back(common::validCondStr());
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
        m_members.push_back(m->clone());
    }
}

ParseBundleFieldImpl::Members ParseBundleFieldImpl::membersList() const
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

ParseBundleFieldImpl::AliasesList ParseBundleFieldImpl::aliasesList() const
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

ParseFieldImpl::Kind ParseBundleFieldImpl::kindImpl() const
{
    return Kind::Bundle;
}

ParseFieldImpl::Ptr ParseBundleFieldImpl::cloneImpl() const
{
    return Ptr(new ParseBundleFieldImpl(*this));
}

const ParseXmlWrap::NamesList& ParseBundleFieldImpl::extraPropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::reuseAliasesStr(),
        common::copyValidCondFromStr(),
    };

    return List;
}

const ParseXmlWrap::NamesList& ParseBundleFieldImpl::extraPossiblePropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::validCondStr(),
    };

    return List;
}

const ParseXmlWrap::NamesList& ParseBundleFieldImpl::extraChildrenNamesImpl() const
{
    static const ParseXmlWrap::NamesList Names = getExtraNames();
    return Names;
}

bool ParseBundleFieldImpl::reuseImpl(const ParseFieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const ParseBundleFieldImpl&>(other);
    assert(m_members.empty());
    m_members.reserve(castedOther.m_members.size());
    std::transform(
        castedOther.m_members.begin(), castedOther.m_members.end(), std::back_inserter(m_members),
        [](auto& elem)
        {
            return elem->clone();
        });
    assert(m_members.size() == castedOther.m_members.size());

    do {
        if (!protocol().isFieldAliasSupported()) {
            break;
        }

        bool reuseAliases = true;
        if (!validateAndUpdateBoolPropValue(common::reuseAliasesStr(), reuseAliases)) {
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
                return elem->clone();
            });
        assert(m_aliases.size() == castedOther.m_aliases.size());
    } while (false);

    if (castedOther.m_validCond) {
        m_validCond = castedOther.m_validCond->clone();
    }     

    return true;
}

bool ParseBundleFieldImpl::parseImpl()
{
    return
        updateMembers() &&
        updateAliases() &&
        copyValidCond() &&
        updateSingleValidCond() &&
        updateMultiValidCond();
}

bool ParseBundleFieldImpl::replaceMembersImpl(FieldsList& members)
{
    for (auto& mem : members) {
        assert(mem);
        auto iter = 
            std::find_if(
                m_members.begin(), m_members.end(),
                [&mem](auto& currMem)
                {
                    assert(currMem);
                    return mem->name() == currMem->name();
                });

        if (iter == m_members.end()) {
            logError() << ParseXmlWrap::logPrefix(mem->getNode()) <<
                "Cannot find reused member with name \"" << mem->name() << "\" to replace.";
            return false;
        }

        (*iter) = std::move(mem);
    }

    return true;
}

std::size_t ParseBundleFieldImpl::minLengthImpl() const
{
    return
        std::accumulate(
            m_members.begin(), m_members.end(), static_cast<std::size_t>(0U),
            [](std::size_t soFar, auto& m) -> std::size_t
            {
                return soFar + m->minLength();
            });
}

std::size_t ParseBundleFieldImpl::maxLengthImpl() const
{
    std::size_t sum = 0U;
    for (auto& m : m_members) {
        if (m->semanticType() == SemanticType::Length) {
            return common::maxPossibleLength();
        }

        common::addToLength(m->maxLength(), sum);
    }

    return sum;
}

bool ParseBundleFieldImpl::strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    return strToNumericOnFields(ref, m_members, val, isBigUnsigned);
}

bool ParseBundleFieldImpl::strToFpImpl(const std::string& ref, double& val) const
{
    return strToFpOnFields(ref, m_members, val);
}

bool ParseBundleFieldImpl::strToBoolImpl(const std::string& ref, bool& val) const
{
    return strToBoolOnFields(ref, m_members, val);
}

bool ParseBundleFieldImpl::strToStringImpl(const std::string& ref, std::string& val) const
{
    return strToStringOnFields(ref, m_members, val);
}

bool ParseBundleFieldImpl::strToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const
{
    return strToDataOnFields(ref, m_members, val);
}

bool ParseBundleFieldImpl::verifySemanticTypeImpl([[maybe_unused]] ::xmlNodePtr node, SemanticType type) const
{
    if ((type == SemanticType::Length) &&
        (protocol().isSemanticTypeLengthSupported()) && 
        (protocol().isNonIntSemanticTypeLengthSupported())) {
        return true;
    }

    return false;
}

bool ParseBundleFieldImpl::verifyAliasedMemberImpl(const std::string& fieldName) const
{
    auto dotPos = fieldName.find('.');
    std::string memFieldName(fieldName, 0, dotPos);
    auto iter =
        std::find_if(
            m_members.begin(), m_members.end(),
            [&memFieldName](auto& f)
            {
                return memFieldName == f->name();
            });

    if (iter == m_members.end()) {
        return false;
    }

    if (fieldName.size() <= dotPos) {
        return true;
    }

    std::string restFieldName(fieldName, dotPos + 1);
    return (*iter)->verifyAliasedMember(restFieldName);
}

const ParseXmlWrap::NamesList& ParseBundleFieldImpl::supportedMemberTypesImpl() const
{
    return bundleSupportedTypes();
}

const ParseBundleFieldImpl::FieldsList& ParseBundleFieldImpl::membersImpl() const
{
    return m_members;
}

bool ParseBundleFieldImpl::updateMembers()
{
    if (!m_members.empty()) {
        m_members.erase(
            std::remove_if(
                m_members.begin(), m_members.end(),
                [this](auto& elem)
                {
                    return
                        (elem->isDeprecatedRemoved()) &&
                        (elem->getDeprecated() <= this->getSinceVersion());
                }),
            m_members.end());

        for (auto& m : m_members) {
            m->setSinceVersion(std::max(getSinceVersion(), m->getSinceVersion()));
        }
    }

    do {
        auto membersNodes = ParseXmlWrap::getChildren(getNode(), common::membersStr());
        if (1U < membersNodes.size()) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
                          "Only single \"" << common::membersStr() << "\" child element is "
                          "supported for \"" << common::bundleStr() << "\".";
            return false;
        }

        auto memberFieldsTypes = ParseXmlWrap::getChildren(getNode(), bundleSupportedTypes());
        if ((0U < membersNodes.size()) && (0U < memberFieldsTypes.size())) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
                          "The \"" << common::bundleStr() << "\" element does not support "
                          "list of stand alone member fields as child elements together with \"" <<
                          common::membersStr() << "\" child element.";
            return false;
        }

        if ((0U == membersNodes.size()) && (0U == memberFieldsTypes.size())) {
            if (m_members.empty()) {
                logError() << ParseXmlWrap::logPrefix(getNode()) <<
                              "The \"" << common::bundleStr() << "\" must contain member fields.";
                return false;
            }

            break;
        }

        if ((0U < memberFieldsTypes.size())) {
            assert(0U == membersNodes.size());
            auto allChildren = ParseXmlWrap::getChildren(getNode());
            if (allChildren.size() != memberFieldsTypes.size()) {
                logError() << ParseXmlWrap::logPrefix(getNode()) <<
                              "The member types of \"" << common::bundleStr() <<
                              "\" must be defined inside \"<" << common::membersStr() << ">\" child element "
                              "when there are other property describing children.";
                return false;
            }
        }

        if (0U < membersNodes.size()) {
            assert(0U == memberFieldsTypes.size());
            memberFieldsTypes = ParseXmlWrap::getChildren(membersNodes.front());
            auto cleanMemberFieldsTypes = ParseXmlWrap::getChildren(membersNodes.front(), bundleSupportedTypes());
            if (cleanMemberFieldsTypes.size() != memberFieldsTypes.size()) {
                logError() << ParseXmlWrap::logPrefix(membersNodes.front()) <<
                              "The \"" << common::membersStr() << "\" child node of \"" <<
                              common::bundleStr() << "\" element must contain only supported types.";
                return false;
            }

            // memberFieldsTypes is updated with the list from <members>
        }

        m_members.reserve(m_members.size() + memberFieldsTypes.size());
        for (auto* memNode : memberFieldsTypes) {
            std::string memKind(reinterpret_cast<const char*>(memNode->name));
            auto mem = ParseFieldImpl::create(memKind, memNode, protocol());
            if (!mem) {
                [[maybe_unused]] static constexpr bool Should_not_happen = false;
                assert(Should_not_happen);
                logError() << ParseXmlWrap::logPrefix(getNode()) <<
                              "Internal error, failed to create objects for member fields.";
                return false;
            }

            mem->setParent(this);
            if (!mem->parse()) {
                return false;
            }

            if (!mem->verifySiblings(m_members)) {
                return false;
            }

            m_members.push_back(std::move(mem));
        }

        if (!validateMembersNames(m_members)) {
            return false;
        }

    } while (false);

    bool hasSameVer =
        std::any_of(
            m_members.begin(), m_members.end(),
            [this](auto& m)
            {
                return m->getSinceVersion() == this->getSinceVersion();
            });

    if (!hasSameVer) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "There must be at least one member with the same version as the parent bundle.";
        return false;
    }

    auto lengthFieldsCount =
        std::count_if(
            m_members.begin(), m_members.end(),
            [](auto& m)
            {
                return m->semanticType() == SemanticType::Length;
            });

    if (1 < lengthFieldsCount) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "No more that single field with semantiType=\"" << common::lengthStr() << "\" "
            "is allowed within \"" << common::bundleStr() << "\".";
        return false;
    }

    return true;
}

bool ParseBundleFieldImpl::updateAliases()
{
    if (!m_aliases.empty()) {
        // Remove aliases in case their aliased fields were removed
        m_aliases.erase(
            std::remove_if(
                m_aliases.begin(), m_aliases.end(),
                [this](auto& alias)
                {
                    auto& fieldName = alias->fieldName();
                    auto iter =
                        std::find_if(
                            m_members.begin(), m_members.end(),
                            [&fieldName](auto& m)
                            {
                                return m->name() == fieldName;
                            });

                    return iter == m_members.end();
                }),
            m_aliases.end());
    }

    auto aliasNodes = ParseXmlWrap::getChildren(getNode(), common::aliasStr());

    if (aliasNodes.empty()) {
        return true;
    }

    if (!protocol().isFieldAliasSupported()) {
        logError() << ParseXmlWrap::logPrefix(aliasNodes.front()) <<
              "Using \"" << common::aliasStr() << "\" nodes for too early \"" <<
              common::dslVersionStr() << "\".";
        return false;
    }

    m_aliases.reserve(m_aliases.size() + aliasNodes.size());
    for (auto* aNode : aliasNodes) {
        auto alias = ParseAliasImpl::create(aNode, protocol());
        if (!alias) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            logError() << ParseXmlWrap::logPrefix(alias->getNode()) <<
                  "Internal error, failed to create objects for member aliases.";
            return false;
        }

        if (!alias->parse()) {
            return false;
        }

        if (!alias->verifyAlias(m_aliases, m_members)) {
            return false;
        }

        m_aliases.push_back(std::move(alias));
    }

    return true;
}

bool ParseBundleFieldImpl::updateSingleValidCond()
{
    return updateSingleCondInternal(common::validCondStr(), m_validCond);
}

bool ParseBundleFieldImpl::updateMultiValidCond()
{
    return updateMultiCondInternal(common::validCondStr(), m_validCond);
}


bool ParseBundleFieldImpl::copyValidCond()
{
    auto& prop = common::copyValidCondFromStr();
    if (!validateSinglePropInstance(prop)) {
        return false;
    }

    auto& copySrc = common::getStringProp(props(), prop);
    if (copySrc.empty()) {
        return true;
    }

    if (!protocol().isPropertySupported(prop)) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
            "The property \"" << prop << "\" is not supported for dslVersion=" << 
                protocol().currSchema().dslVersion() << ".";        
        return true;
    }  

    auto* field = protocol().findField(copySrc);
    if (field == nullptr) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Field referenced by \"" << prop << "\" property (" + copySrc + ") is not found.";
        return false;        
    }     

    if (field->kind() != kind()) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Cannot reference field of other cond in property \"" << prop << "\".";
        return false;
    }      

    if (m_validCond) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Cannot use \"" << prop << "\" property when the validity condition is copied from other field by other means";        
        return false;
    }

    auto* otherBundle = static_cast<const ParseBundleFieldImpl*>(field);
    if (!otherBundle->m_validCond) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
            "Field referenced by the \"" << prop << "\" property (" << copySrc << ") does not specify any validity conditions";        
        return true;
    }

    m_validCond = otherBundle->m_validCond->clone();
    if (!m_validCond->verify(m_members, getNode(), protocol())) {
        return false;
    }   

    return true;
}

bool ParseBundleFieldImpl::updateSingleCondInternal(const std::string& prop, ParseOptCondImplPtr& cond)
{
    if (!validateSinglePropInstance(prop)) {
        return false;
    }

    auto iter = props().find(prop);
    if (iter == props().end()) {
        return true;
    }

    if (!protocol().isValidCondSupportedInCompositeFields()) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
            "The property \"" << prop << "\" is not supported for <bundle> field in dslVersion=" << 
                protocol().currSchema().dslVersion() << ".";        
        return true;
    }          

    auto newCond = std::make_unique<ParseOptCondExprImpl>();
    if (!newCond->parse(iter->second, getNode(), protocol())) {
        return false;
    }

    if (newCond->hasInterfaceReference()) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "The condition \"" << prop << "\" in fields cannot reference interface fields.";           
        return false;
    }

    if (!newCond->verify(m_members, getNode(), protocol())) {
        return false;
    }   

    cond = std::move(newCond);
    return true; 
}

bool ParseBundleFieldImpl::updateMultiCondInternal(const std::string& prop, ParseOptCondImplPtr& cond)
{
    auto condNodes = ParseXmlWrap::getChildren(getNode(), prop, true);
    if (condNodes.empty()) {
        return true;
    }

    if (!protocol().isValidCondSupportedInCompositeFields()) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
            "The property \"" << prop << "\" is not supported for <bundle> field in dslVersion=" << 
                protocol().currSchema().dslVersion() << ".";        
        return true;
    }      

    if (condNodes.size() > 1U) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Cannot use more that one child to the \"" << prop << "\" element.";        
        return false;
    }

    static const ParseXmlWrap::NamesList ElemNames = {
        common::andStr(),
        common::orStr()
    };

    auto condChildren = ParseXmlWrap::getChildren(condNodes.front(), ElemNames);
    if (condChildren.size() != condNodes.size()) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Only single \"" << common::andStr() << "\" or \"" << common::orStr() << "\" child of the \"" << prop << "\" element is supported.";           
        return false;
    }    

    auto iter = props().find(prop);
    if (iter != props().end()) {
        logError() << ParseXmlWrap::logPrefix(condNodes.front()) <<
            "Only single \"" << prop << "\" property is supported";

        return false;
    }    

    auto newCond = std::make_unique<ParseOptCondListImpl>();
    newCond->overrideCondStr(prop);
    if (!newCond->parse(condChildren.front(), protocol())) {
        return false;
    }

    if (newCond->hasInterfaceReference()) {
        logError() << ParseXmlWrap::logPrefix(condNodes.front()) <<
            "The condition \"" << prop << "\" in fields cannot reference interface fields.";           
        return false;
    }    

    if (!newCond->verify(m_members, condChildren.front(), protocol())) {
        return false;
    }    

    cond = std::move(newCond);
    return true;
}

} // namespace parse

} // namespace commsdsl
