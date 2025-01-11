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

#include "BundleFieldImpl.h"

#include <cassert>
#include <limits>
#include <numeric>
#include <utility>
#include <algorithm>
#include <iterator>

#include "ProtocolImpl.h"
#include "OptionalFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const XmlWrap::NamesList& bundleSupportedTypes()
{
    static const XmlWrap::NamesList Names = FieldImpl::supportedTypes();
    return Names;
}

XmlWrap::NamesList getExtraNames()
{
    auto names = bundleSupportedTypes();
    names.push_back(common::membersStr());
    names.push_back(common::aliasStr());
    names.push_back(common::validCondStr());
    return names;
}

} // namespace

BundleFieldImpl::BundleFieldImpl(xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

BundleFieldImpl::BundleFieldImpl(const BundleFieldImpl& other)
  : Base(other)
{
    m_members.reserve(other.m_members.size());
    for (auto& m : other.m_members) {
        m_members.push_back(m->clone());
    }
}

BundleFieldImpl::Members BundleFieldImpl::membersList() const
{
    Members result;
    result.reserve(m_members.size());
    std::transform(
        m_members.begin(), m_members.end(), std::back_inserter(result),
        [](auto& elem)
        {
            return Field(elem.get());
        });
    return result;
}

BundleFieldImpl::AliasesList BundleFieldImpl::aliasesList() const
{
    AliasesList result;
    result.reserve(m_aliases.size());
    std::transform(
        m_aliases.begin(), m_aliases.end(), std::back_inserter(result),
        [](auto& elem)
        {
            return Alias(elem.get());
        });
    return result;
}

FieldImpl::Kind BundleFieldImpl::kindImpl() const
{
    return Kind::Bundle;
}

FieldImpl::Ptr BundleFieldImpl::cloneImpl() const
{
    return Ptr(new BundleFieldImpl(*this));
}

const XmlWrap::NamesList& BundleFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::reuseAliasesStr(),
        common::copyValidCondFromStr(),
    };

    return List;
}

const XmlWrap::NamesList& BundleFieldImpl::extraPossiblePropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::validCondStr(),
    };

    return List;
}

const XmlWrap::NamesList& BundleFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList Names = getExtraNames();
    return Names;
}

bool BundleFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const BundleFieldImpl&>(other);
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

bool BundleFieldImpl::parseImpl()
{
    return
        updateMembers() &&
        updateAliases() &&
        copyValidCond() &&
        updateSingleValidCond() &&
        updateMultiValidCond();
}

bool BundleFieldImpl::replaceMembersImpl(FieldsList& members)
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
            logError() << XmlWrap::logPrefix(mem->getNode()) <<
                "Cannot find reused member with name \"" << mem->name() << "\" to replace.";
            return false;
        }

        (*iter) = std::move(mem);
    }

    return true;
}

std::size_t BundleFieldImpl::minLengthImpl() const
{
    return
        std::accumulate(
            m_members.begin(), m_members.end(), static_cast<std::size_t>(0U),
            [](std::size_t soFar, auto& m) -> std::size_t
            {
                return soFar + m->minLength();
            });
}

std::size_t BundleFieldImpl::maxLengthImpl() const
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

bool BundleFieldImpl::strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    return strToNumericOnFields(ref, m_members, val, isBigUnsigned);
}

bool BundleFieldImpl::strToFpImpl(const std::string& ref, double& val) const
{
    return strToFpOnFields(ref, m_members, val);
}

bool BundleFieldImpl::strToBoolImpl(const std::string& ref, bool& val) const
{
    return strToBoolOnFields(ref, m_members, val);
}

bool BundleFieldImpl::strToStringImpl(const std::string& ref, std::string& val) const
{
    return strToStringOnFields(ref, m_members, val);
}

bool BundleFieldImpl::strToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const
{
    return strToDataOnFields(ref, m_members, val);
}

bool BundleFieldImpl::verifySemanticTypeImpl([[maybe_unused]] ::xmlNodePtr node, SemanticType type) const
{
    if ((type == SemanticType::Length) &&
        (protocol().isSemanticTypeLengthSupported()) && 
        (protocol().isNonIntSemanticTypeLengthSupported())) {
        return true;
    }

    return false;
}

bool BundleFieldImpl::verifyAliasedMemberImpl(const std::string& fieldName) const
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

const XmlWrap::NamesList& BundleFieldImpl::supportedMemberTypesImpl() const
{
    return bundleSupportedTypes();
}

const BundleFieldImpl::FieldsList& BundleFieldImpl::membersImpl() const
{
    return m_members;
}

bool BundleFieldImpl::updateMembers()
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
        auto membersNodes = XmlWrap::getChildren(getNode(), common::membersStr());
        if (1U < membersNodes.size()) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                          "Only single \"" << common::membersStr() << "\" child element is "
                          "supported for \"" << common::bundleStr() << "\".";
            return false;
        }

        auto memberFieldsTypes = XmlWrap::getChildren(getNode(), bundleSupportedTypes());
        if ((0U < membersNodes.size()) && (0U < memberFieldsTypes.size())) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                          "The \"" << common::bundleStr() << "\" element does not support "
                          "list of stand alone member fields as child elements together with \"" <<
                          common::membersStr() << "\" child element.";
            return false;
        }

        if ((0U == membersNodes.size()) && (0U == memberFieldsTypes.size())) {
            if (m_members.empty()) {
                logError() << XmlWrap::logPrefix(getNode()) <<
                              "The \"" << common::bundleStr() << "\" must contain member fields.";
                return false;
            }

            break;
        }

        if ((0U < memberFieldsTypes.size())) {
            assert(0U == membersNodes.size());
            auto allChildren = XmlWrap::getChildren(getNode());
            if (allChildren.size() != memberFieldsTypes.size()) {
                logError() << XmlWrap::logPrefix(getNode()) <<
                              "The member types of \"" << common::bundleStr() <<
                              "\" must be defined inside \"<" << common::membersStr() << ">\" child element "
                              "when there are other property describing children.";
                return false;
            }
        }

        if (0U < membersNodes.size()) {
            assert(0U == memberFieldsTypes.size());
            memberFieldsTypes = XmlWrap::getChildren(membersNodes.front());
            auto cleanMemberFieldsTypes = XmlWrap::getChildren(membersNodes.front(), bundleSupportedTypes());
            if (cleanMemberFieldsTypes.size() != memberFieldsTypes.size()) {
                logError() << XmlWrap::logPrefix(membersNodes.front()) <<
                              "The \"" << common::membersStr() << "\" child node of \"" <<
                              common::bundleStr() << "\" element must contain only supported types.";
                return false;
            }

            // memberFieldsTypes is updated with the list from <members>
        }

        m_members.reserve(m_members.size() + memberFieldsTypes.size());
        for (auto* memNode : memberFieldsTypes) {
            std::string memKind(reinterpret_cast<const char*>(memNode->name));
            auto mem = FieldImpl::create(memKind, memNode, protocol());
            if (!mem) {
                [[maybe_unused]] static constexpr bool Should_not_happen = false;
                assert(Should_not_happen);
                logError() << XmlWrap::logPrefix(getNode()) <<
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
        logError() << XmlWrap::logPrefix(getNode()) <<
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
        logError() << XmlWrap::logPrefix(getNode()) <<
            "No more that single field with semantiType=\"" << common::lengthStr() << "\" "
            "is allowed within \"" << common::bundleStr() << "\".";
        return false;
    }

    return true;
}

bool BundleFieldImpl::updateAliases()
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

    auto aliasNodes = XmlWrap::getChildren(getNode(), common::aliasStr());

    if (aliasNodes.empty()) {
        return true;
    }

    if (!protocol().isFieldAliasSupported()) {
        logError() << XmlWrap::logPrefix(aliasNodes.front()) <<
              "Using \"" << common::aliasStr() << "\" nodes for too early \"" <<
              common::dslVersionStr() << "\".";
        return false;
    }

    m_aliases.reserve(m_aliases.size() + aliasNodes.size());
    for (auto* aNode : aliasNodes) {
        auto alias = AliasImpl::create(aNode, protocol());
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

        if (!alias->verifyAlias(m_aliases, m_members)) {
            return false;
        }

        m_aliases.push_back(std::move(alias));
    }

    return true;
}

bool BundleFieldImpl::updateSingleValidCond()
{
    return updateSingleCondInternal(common::validCondStr(), m_validCond);
}

bool BundleFieldImpl::updateMultiValidCond()
{
    return updateMultiCondInternal(common::validCondStr(), m_validCond);
}


bool BundleFieldImpl::copyValidCond()
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
        logWarning() << XmlWrap::logPrefix(getNode()) <<
            "The property \"" << prop << "\" is not supported for dslVersion=" << 
                protocol().currSchema().dslVersion() << ".";        
        return true;
    }  

    auto* field = protocol().findField(copySrc);
    if (field == nullptr) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Field referenced by \"" << prop << "\" property (" + copySrc + ") is not found.";
        return false;        
    }     

    if (field->kind() != kind()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot reference field of other cond in property \"" << prop << "\".";
        return false;
    }      

    if (m_validCond) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot use \"" << prop << "\" property when the validity condition is copied from other field by other means";        
        return false;
    }

    auto* otherBundle = static_cast<const BundleFieldImpl*>(field);
    if (!otherBundle->m_validCond) {
        logWarning() << XmlWrap::logPrefix(getNode()) <<
            "Field referenced by the \"" << prop << "\" property (" << copySrc << ") does not specify any validity conditions";        
        return true;
    }

    m_validCond = otherBundle->m_validCond->clone();
    if (!m_validCond->verify(m_members, getNode(), protocol())) {
        return false;
    }   

    return true;
}

bool BundleFieldImpl::updateSingleCondInternal(const std::string& prop, OptCondImplPtr& cond)
{
    if (!validateSinglePropInstance(prop)) {
        return false;
    }

    auto iter = props().find(prop);
    if (iter == props().end()) {
        return true;
    }

    if (!protocol().isValidCondSupportedInCompositeFields()) {
        logWarning() << XmlWrap::logPrefix(getNode()) <<
            "The property \"" << prop << "\" is not supported for <bundle> field in dslVersion=" << 
                protocol().currSchema().dslVersion() << ".";        
        return true;
    }          

    auto newCond = std::make_unique<OptCondExprImpl>();
    if (!newCond->parse(iter->second, getNode(), protocol())) {
        return false;
    }

    if (newCond->hasInterfaceReference()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "The condition \"" << prop << "\" in fields cannot reference interface fields.";           
        return false;
    }

    if (!newCond->verify(m_members, getNode(), protocol())) {
        return false;
    }   

    cond = std::move(newCond);
    return true; 
}

bool BundleFieldImpl::updateMultiCondInternal(const std::string& prop, OptCondImplPtr& cond)
{
    auto condNodes = XmlWrap::getChildren(getNode(), prop, true);
    if (condNodes.empty()) {
        return true;
    }

    if (!protocol().isValidCondSupportedInCompositeFields()) {
        logWarning() << XmlWrap::logPrefix(getNode()) <<
            "The property \"" << prop << "\" is not supported for <bundle> field in dslVersion=" << 
                protocol().currSchema().dslVersion() << ".";        
        return true;
    }      

    if (condNodes.size() > 1U) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot use more that one child to the \"" << prop << "\" element.";        
        return false;
    }

    static const XmlWrap::NamesList ElemNames = {
        common::andStr(),
        common::orStr()
    };

    auto condChildren = XmlWrap::getChildren(condNodes.front(), ElemNames);
    if (condChildren.size() != condNodes.size()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Only single \"" << common::andStr() << "\" or \"" << common::orStr() << "\" child of the \"" << prop << "\" element is supported.";           
        return false;
    }    

    auto iter = props().find(prop);
    if (iter != props().end()) {
        logError() << XmlWrap::logPrefix(condNodes.front()) <<
            "Only single \"" << prop << "\" property is supported";

        return false;
    }    

    auto newCond = std::make_unique<OptCondListImpl>();
    newCond->overrideCondStr(prop);
    if (!newCond->parse(condChildren.front(), protocol())) {
        return false;
    }

    if (newCond->hasInterfaceReference()) {
        logError() << XmlWrap::logPrefix(condNodes.front()) <<
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
