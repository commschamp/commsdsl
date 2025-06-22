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

#include "ParseBitfieldFieldImpl.h"

#include "ParseProtocolImpl.h"

#include <cassert>
#include <limits>
#include <numeric>
#include <utility>
#include <algorithm>
#include <iterator>

namespace commsdsl
{

namespace parse
{

namespace
{

ParseXmlWrap::NamesList getExtraNames()
{
    auto names = ParseBitfieldFieldImpl::supportedTypes();
    names.push_back(common::membersStr());
    names.push_back(common::validCondStr());
    return names;
}

} // namespace

ParseBitfieldFieldImpl::ParseBitfieldFieldImpl(xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol)
{
}

ParseBitfieldFieldImpl::ParseBitfieldFieldImpl(const ParseBitfieldFieldImpl& other)
  : Base(other),
    m_endian(other.m_endian)
{
    m_members.reserve(other.m_members.size());
    for (auto& m : other.m_members) {
        m_members.push_back(m->clone());
    }
}

ParseBitfieldFieldImpl::Members ParseBitfieldFieldImpl::membersList() const
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

const ParseXmlWrap::NamesList& ParseBitfieldFieldImpl::supportedTypes()
{
    static const ParseXmlWrap::NamesList Names = {
        common::intStr(),
        common::enumStr(),
        common::setStr(),
        common::refStr()
    };

    return Names;    
}

ParseFieldImpl::Kind ParseBitfieldFieldImpl::kindImpl() const
{
    return Kind::Bitfield;
}

ParseFieldImpl::Ptr ParseBitfieldFieldImpl::cloneImpl() const
{
    return Ptr(new ParseBitfieldFieldImpl(*this));
}

const ParseXmlWrap::NamesList& ParseBitfieldFieldImpl::extraPropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::endianStr(),
        common::copyValidCondFromStr(),
    };

    return List;
}

const ParseXmlWrap::NamesList& ParseBitfieldFieldImpl::extraPossiblePropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::validCondStr(),
    };

    return List;
}

const ParseXmlWrap::NamesList& ParseBitfieldFieldImpl::extraChildrenNamesImpl() const
{
    static const ParseXmlWrap::NamesList Names = getExtraNames();
    return Names;
}

bool ParseBitfieldFieldImpl::reuseImpl(const ParseFieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const ParseBitfieldFieldImpl&>(other);
    m_endian = castedOther.m_endian;
    assert(m_members.empty());
    m_members.reserve(castedOther.m_members.size());
    std::transform(
        castedOther.m_members.begin(), castedOther.m_members.end(), std::back_inserter(m_members),
        [](auto& elem)
        {
            return elem->clone();
        });
    assert(m_members.size() == castedOther.m_members.size());

    if (castedOther.m_validCond) {
        m_validCond = castedOther.m_validCond->clone();
    }      
    return true;
}

bool ParseBitfieldFieldImpl::parseImpl()
{
    return
        updateEndian() &&
        updateMembers() &&
        copyValidCond() &&
        updateSingleValidCond() &&
        updateMultiValidCond();
}

bool ParseBitfieldFieldImpl::replaceMembersImpl(FieldsList& members)
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

        if ((mem->getSinceVersion() != getSinceVersion()) ||
            (mem->getDeprecated() != getDeprecated())) {
            logError() << ParseXmlWrap::logPrefix(mem->getNode()) <<
                "Bitfield replacing members are not allowed to update \"" << common::sinceVersionStr() << "\" and "
                "\"" << common::deprecatedStr() << "\" properties.";
            return false;
        }

        (*iter) = std::move(mem);
    }

    return true;
}

std::size_t ParseBitfieldFieldImpl::minLengthImpl() const
{
    return
        std::accumulate(
            m_members.begin(), m_members.end(), static_cast<std::size_t>(0U),
            [](std::size_t soFar, auto& m) -> std::size_t
            {
                return soFar + m->bitLength();
    }) / 8U;
}

bool ParseBitfieldFieldImpl::strToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    return strToNumericOnFields(ref, m_members, val, isBigUnsigned);
}

bool ParseBitfieldFieldImpl::strToFpImpl(const std::string& ref, double& val) const
{
    return strToFpOnFields(ref, m_members, val);
}

bool ParseBitfieldFieldImpl::strToBoolImpl(const std::string& ref, bool& val) const
{
    return strToBoolOnFields(ref, m_members, val);
}

bool ParseBitfieldFieldImpl::verifySemanticTypeImpl([[maybe_unused]] ::xmlNodePtr node, SemanticType type) const
{
    if ((type == SemanticType::Length) &&
        (protocol().isSemanticTypeLengthSupported()) && 
        (protocol().isNonIntSemanticTypeLengthSupported())) {
        return true;
    }

    return false;
}

bool ParseBitfieldFieldImpl::verifyAliasedMemberImpl(const std::string& fieldName) const
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

const ParseXmlWrap::NamesList& ParseBitfieldFieldImpl::supportedMemberTypesImpl() const
{
    return ParseBitfieldFieldImpl::supportedTypes();
}

const ParseBitfieldFieldImpl::FieldsList& ParseBitfieldFieldImpl::membersImpl() const
{
    return m_members;
}

bool ParseBitfieldFieldImpl::updateEndian()
{
    if (!validateSinglePropInstance(common::endianStr())) {
        return false;
    }

    auto& endianStr = common::getStringProp(props(), common::endianStr());
    if ((endianStr.empty()) && (m_endian != ParseEndian_NumOfValues)) {
        return true;
    }

    m_endian = common::parseEndian(endianStr, protocol().currSchema().endian());
    if (m_endian == ParseEndian_NumOfValues) {
        reportUnexpectedPropertyValue(common::endianStr(), endianStr);
        return false;
    }
    return true;
}

bool ParseBitfieldFieldImpl::updateMembers()
{
    if (!m_members.empty()) {
        for (auto& m : m_members) {
            m->setSinceVersion(std::max(getSinceVersion(), m->getSinceVersion()));
            assert(m->getSinceVersion() == getSinceVersion());
            assert(m->getDeprecated() == ParseProtocol::notYetDeprecated());
        }
    }

    do {
        auto membersNodes = ParseXmlWrap::getChildren(getNode(), common::membersStr());
        if (1U < membersNodes.size()) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
                          "Only single \"" << common::membersStr() << "\" child element is "
                          "supported for \"" << common::bitfieldStr() << "\".";
            return false;
        }

        auto memberFieldsTypes = ParseXmlWrap::getChildren(getNode(), supportedTypes());
        if ((0U < membersNodes.size()) && (0U < memberFieldsTypes.size())) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
                          "The \"" << common::bitfieldStr() << "\" element does not support "
                          "list of stand alone member fields as child elements together with \"" <<
                          common::membersStr() << "\" child element.";
            return false;
        }

        if ((0U == membersNodes.size()) && (0U == memberFieldsTypes.size())) {
            if (m_members.empty()) {
                logError() << ParseXmlWrap::logPrefix(getNode()) <<
                              "The \"" << common::bitfieldStr() << "\" must contain member fields.";
                return false;
            }

            break;
        }

        if (!m_members.empty()) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
                          "The \"" << common::bitfieldStr() << "\" cannot add member fields after reuse.";
            return false;
        }

        if ((0U < memberFieldsTypes.size())) {
            assert(0U == membersNodes.size());
            auto allChildren = ParseXmlWrap::getChildren(getNode());
            if (allChildren.size() != memberFieldsTypes.size()) {
                logError() << ParseXmlWrap::logPrefix(getNode()) <<
                              "The member types of \"" << common::bitfieldStr() <<
                              "\" must be defined inside \"<" << common::membersStr() << ">\" child element "
                              "when there are other property describing children.";
                return false;
            }
        }

        if (0U < membersNodes.size()) {
            assert(0U == memberFieldsTypes.size());
            memberFieldsTypes = ParseXmlWrap::getChildren(membersNodes.front());
            auto cleanMemberFieldsTypes = ParseXmlWrap::getChildren(membersNodes.front(), supportedTypes());
            if (cleanMemberFieldsTypes.size() != memberFieldsTypes.size()) {
                logError() << ParseXmlWrap::logPrefix(membersNodes.front()) <<
                              "The \"" << common::membersStr() << "\" child node of \"" <<
                              common::bitfieldStr() << "\" element must contain only supported types.";
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

            if ((mem->getSinceVersion() != getSinceVersion()) ||
                (mem->getDeprecated() != getDeprecated())) {
                logError() << ParseXmlWrap::logPrefix(mem->getNode()) <<
                    "Bitfield members are not allowed to update \"" << common::sinceVersionStr() << "\" and "
                    "\"" << common::deprecatedStr() << "\" properties.";
                return false;
            }

            m_members.push_back(std::move(mem));
        }

        if (!validateMembersNames(m_members)) {
            return false;
        }

        auto totalBitLength =
            std::accumulate(
                m_members.begin(), m_members.end(), static_cast<std::size_t>(0U),
                [](std::size_t soFar, auto& elem) -> std::size_t
                {
                    return soFar + elem->bitLength();
                });

        if ((totalBitLength % 8U) != 0) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
                          "The summary of member's bit lengths (" << totalBitLength <<
                          ") is expected to be devisable by 8.";
            return false;
        }

        static const std::size_t MaxBits = std::numeric_limits<std::uint64_t>::digits;
        if (MaxBits < totalBitLength) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
                          "The summary of member's bit lengths (" << totalBitLength <<
                          ") cannot be greater than " << MaxBits << '.';
            return false;
        }
    } while (false);

    return true;
}

bool ParseBitfieldFieldImpl::updateSingleValidCond()
{
    return updateSingleCondInternal(common::validCondStr(), m_validCond);
}

bool ParseBitfieldFieldImpl::updateMultiValidCond()
{
    return updateMultiCondInternal(common::validCondStr(), m_validCond);
}


bool ParseBitfieldFieldImpl::copyValidCond()
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

    auto* other = static_cast<const ParseBitfieldFieldImpl*>(field);
    if (!other->m_validCond) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
            "Field referenced by the \"" << prop << "\" property (" << copySrc << ") does not specify any validity conditions";        
        return true;
    }

    m_validCond = other->m_validCond->clone();
    if (!m_validCond->verify(m_members, getNode(), protocol())) {
        return false;
    }   

    return true;
}

bool ParseBitfieldFieldImpl::updateSingleCondInternal(const std::string& prop, ParseOptCondImplPtr& cond)
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
            "The property \"" << prop << "\" is not supported for <bitfield> field in dslVersion=" << 
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

bool ParseBitfieldFieldImpl::updateMultiCondInternal(const std::string& prop, ParseOptCondImplPtr& cond)
{
    auto condNodes = ParseXmlWrap::getChildren(getNode(), prop, true);
    if (condNodes.empty()) {
        return true;
    }

    if (!protocol().isValidCondSupportedInCompositeFields()) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
            "The property \"" << prop << "\" is not supported for <bitfield> field in dslVersion=" << 
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
