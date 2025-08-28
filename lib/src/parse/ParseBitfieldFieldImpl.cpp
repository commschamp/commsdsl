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

ParseXmlWrap::ParseNamesList parseGetExtraNames()
{
    auto names = ParseBitfieldFieldImpl::parseSupportedTypes();
    names.push_back(common::parseMembersStr());
    names.push_back(common::parseValidCondStr());
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
        m_members.push_back(m->parseClone());
    }
}

ParseBitfieldFieldImpl::ParseMembers ParseBitfieldFieldImpl::parseMembersList() const
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

const ParseXmlWrap::ParseNamesList& ParseBitfieldFieldImpl::parseSupportedTypes()
{
    static const ParseXmlWrap::ParseNamesList Names = {
        common::parseIntStr(),
        common::parseEnumStr(),
        common::parseSetStr(),
        common::parseRefStr()
    };

    return Names;    
}

ParseFieldImpl::ParseKind ParseBitfieldFieldImpl::parseKindImpl() const
{
    return ParseKind::Bitfield;
}

ParseFieldImpl::ParsePtr ParseBitfieldFieldImpl::parseCloneImpl() const
{
    return ParsePtr(new ParseBitfieldFieldImpl(*this));
}

const ParseXmlWrap::ParseNamesList& ParseBitfieldFieldImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::ParseNamesList List = {
        common::parseEndianStr(),
        common::parseCopyValidCondFromStr(),
    };

    return List;
}

const ParseXmlWrap::ParseNamesList& ParseBitfieldFieldImpl::parseExtraPossiblePropsNamesImpl() const
{
    static const ParseXmlWrap::ParseNamesList List = {
        common::parseValidCondStr(),
    };

    return List;
}

const ParseXmlWrap::ParseNamesList& ParseBitfieldFieldImpl::parseExtraChildrenNamesImpl() const
{
    static const ParseXmlWrap::ParseNamesList Names = parseGetExtraNames();
    return Names;
}

bool ParseBitfieldFieldImpl::parseReuseImpl(const ParseFieldImpl& other)
{
    assert(other.parseKind() == parseKind());
    auto& castedOther = static_cast<const ParseBitfieldFieldImpl&>(other);
    m_endian = castedOther.m_endian;
    assert(m_members.empty());
    m_members.reserve(castedOther.m_members.size());
    std::transform(
        castedOther.m_members.begin(), castedOther.m_members.end(), std::back_inserter(m_members),
        [](auto& elem)
        {
            return elem->parseClone();
        });
    assert(m_members.size() == castedOther.m_members.size());

    if (castedOther.m_validCond) {
        m_validCond = castedOther.m_validCond->parseClone();
    }      
    return true;
}

bool ParseBitfieldFieldImpl::parseImpl()
{
    return
        parseUpdateEndian() &&
        parseUpdateMembers() &&
        parseCopyValidCond() &&
        parseUpdateSingleValidCond() &&
        parseUpdateMultiValidCond();
}

bool ParseBitfieldFieldImpl::parseReplaceMembersImpl(ParseFieldsList& members)
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

        if ((mem->parseGetSinceVersion() != parseGetSinceVersion()) ||
            (mem->parseGetDeprecated() != parseGetDeprecated())) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(mem->parseGetNode()) <<
                "Bitfield replacing members are not allowed to update \"" << common::parseSinceVersionStr() << "\" and "
                "\"" << common::parseDeprecatedStr() << "\" properties.";
            return false;
        }

        (*iter) = std::move(mem);
    }

    return true;
}

std::size_t ParseBitfieldFieldImpl::parseMinLengthImpl() const
{
    return
        std::accumulate(
            m_members.begin(), m_members.end(), static_cast<std::size_t>(0U),
            [](std::size_t soFar, auto& m) -> std::size_t
            {
                return soFar + m->parseBitLength();
    }) / 8U;
}

bool ParseBitfieldFieldImpl::parseStrToNumericImpl(const std::string& ref, std::intmax_t& val, bool& isBigUnsigned) const
{
    return parseStrToNumericOnFields(ref, m_members, val, isBigUnsigned);
}

bool ParseBitfieldFieldImpl::parseStrToFpImpl(const std::string& ref, double& val) const
{
    return parseStrToFpOnFields(ref, m_members, val);
}

bool ParseBitfieldFieldImpl::parseStrToBoolImpl(const std::string& ref, bool& val) const
{
    return parseStrToBoolOnFields(ref, m_members, val);
}

bool ParseBitfieldFieldImpl::parseVerifySemanticTypeImpl([[maybe_unused]] ::xmlNodePtr node, ParseSemanticType type) const
{
    if ((type == ParseSemanticType::Length) &&
        (parseProtocol().parseIsSemanticTypeLengthSupported()) && 
        (parseProtocol().parseIsNonIntSemanticTypeLengthSupported())) {
        return true;
    }

    return false;
}

bool ParseBitfieldFieldImpl::parseVerifyAliasedMemberImpl(const std::string& fieldName) const
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

const ParseXmlWrap::ParseNamesList& ParseBitfieldFieldImpl::parseSupportedMemberTypesImpl() const
{
    return ParseBitfieldFieldImpl::parseSupportedTypes();
}

const ParseBitfieldFieldImpl::ParseFieldsList& ParseBitfieldFieldImpl::parseMembersImpl() const
{
    return m_members;
}

bool ParseBitfieldFieldImpl::parseUpdateEndian()
{
    if (!parseValidateSinglePropInstance(common::parseEndianStr())) {
        return false;
    }

    auto& endianStr = common::parseGetStringProp(parseProps(), common::parseEndianStr());
    if ((endianStr.empty()) && (m_endian != ParseEndian_NumOfValues)) {
        return true;
    }

    m_endian = common::parseEndian(endianStr, parseProtocol().parseCurrSchema().parseEndian());
    if (m_endian == ParseEndian_NumOfValues) {
        parseReportUnexpectedPropertyValue(common::parseEndianStr(), endianStr);
        return false;
    }
    return true;
}

bool ParseBitfieldFieldImpl::parseUpdateMembers()
{
    if (!m_members.empty()) {
        for (auto& m : m_members) {
            m->parseSetSinceVersion(std::max(parseGetSinceVersion(), m->parseGetSinceVersion()));
            assert(m->parseGetSinceVersion() == parseGetSinceVersion());
            assert(m->parseGetDeprecated() == ParseProtocol::parseNotYetDeprecated());
        }
    }

    do {
        auto membersNodes = ParseXmlWrap::parseGetChildren(parseGetNode(), common::parseMembersStr());
        if (1U < membersNodes.size()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                          "Only single \"" << common::parseMembersStr() << "\" child element is "
                          "supported for \"" << common::parseBitparseFieldStr() << "\".";
            return false;
        }

        auto memberFieldsTypes = ParseXmlWrap::parseGetChildren(parseGetNode(), parseSupportedTypes());
        if ((0U < membersNodes.size()) && (0U < memberFieldsTypes.size())) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                          "The \"" << common::parseBitparseFieldStr() << "\" element does not support "
                          "list of stand alone member fields as child elements together with \"" <<
                          common::parseMembersStr() << "\" child element.";
            return false;
        }

        if ((0U == membersNodes.size()) && (0U == memberFieldsTypes.size())) {
            if (m_members.empty()) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                              "The \"" << common::parseBitparseFieldStr() << "\" must contain member fields.";
                return false;
            }

            break;
        }

        if (!m_members.empty()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                          "The \"" << common::parseBitparseFieldStr() << "\" cannot add member fields after reuse.";
            return false;
        }

        if ((0U < memberFieldsTypes.size())) {
            assert(0U == membersNodes.size());
            auto allChildren = ParseXmlWrap::parseGetChildren(parseGetNode());
            if (allChildren.size() != memberFieldsTypes.size()) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                              "The member types of \"" << common::parseBitparseFieldStr() <<
                              "\" must be defined inside \"<" << common::parseMembersStr() << ">\" child element "
                              "when there are other property describing children.";
                return false;
            }
        }

        if (0U < membersNodes.size()) {
            assert(0U == memberFieldsTypes.size());
            memberFieldsTypes = ParseXmlWrap::parseGetChildren(membersNodes.front());
            auto cleanMemberFieldsTypes = ParseXmlWrap::parseGetChildren(membersNodes.front(), parseSupportedTypes());
            if (cleanMemberFieldsTypes.size() != memberFieldsTypes.size()) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(membersNodes.front()) <<
                              "The \"" << common::parseMembersStr() << "\" child node of \"" <<
                              common::parseBitparseFieldStr() << "\" element must contain only supported types.";
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

            if ((mem->parseGetSinceVersion() != parseGetSinceVersion()) ||
                (mem->parseGetDeprecated() != parseGetDeprecated())) {
                parseLogError() << ParseXmlWrap::parseLogPrefix(mem->parseGetNode()) <<
                    "Bitfield members are not allowed to update \"" << common::parseSinceVersionStr() << "\" and "
                    "\"" << common::parseDeprecatedStr() << "\" properties.";
                return false;
            }

            m_members.push_back(std::move(mem));
        }

        if (!parseValidateMembersNames(m_members)) {
            return false;
        }

        auto totalBitLength =
            std::accumulate(
                m_members.begin(), m_members.end(), static_cast<std::size_t>(0U),
                [](std::size_t soFar, auto& elem) -> std::size_t
                {
                    return soFar + elem->parseBitLength();
                });

        if ((totalBitLength % 8U) != 0) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                          "The summary of member's bit lengths (" << totalBitLength <<
                          ") is expected to be devisable by 8.";
            return false;
        }

        static const std::size_t MaxBits = std::numeric_limits<std::uint64_t>::digits;
        if (MaxBits < totalBitLength) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                          "The summary of member's bit lengths (" << totalBitLength <<
                          ") cannot be greater than " << MaxBits << '.';
            return false;
        }
    } while (false);

    return true;
}

bool ParseBitfieldFieldImpl::parseUpdateSingleValidCond()
{
    return parseUpdateSingleCondInternal(common::parseValidCondStr(), m_validCond);
}

bool ParseBitfieldFieldImpl::parseUpdateMultiValidCond()
{
    return parseUpdateMultiCondInternal(common::parseValidCondStr(), m_validCond);
}


bool ParseBitfieldFieldImpl::parseCopyValidCond()
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

    auto* other = static_cast<const ParseBitfieldFieldImpl*>(field);
    if (!other->m_validCond) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Field referenced by the \"" << prop << "\" property (" << copySrc << ") does not specify any validity conditions";        
        return true;
    }

    m_validCond = other->m_validCond->parseClone();
    if (!m_validCond->parseVerify(m_members, parseGetNode(), parseProtocol())) {
        return false;
    }   

    return true;
}

bool ParseBitfieldFieldImpl::parseUpdateSingleCondInternal(const std::string& prop, ParseOptCondImplPtr& cond)
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
            "The property \"" << prop << "\" is not supported for <bitfield> field in dslVersion=" << 
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

bool ParseBitfieldFieldImpl::parseUpdateMultiCondInternal(const std::string& prop, ParseOptCondImplPtr& cond)
{
    auto condNodes = ParseXmlWrap::parseGetChildren(parseGetNode(), prop, true);
    if (condNodes.empty()) {
        return true;
    }

    if (!parseProtocol().parseIsValidCondSupportedInCompositeFields()) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The property \"" << prop << "\" is not supported for <bitfield> field in dslVersion=" << 
                parseProtocol().parseCurrSchema().parseDslVersion() << ".";        
        return true;
    }      

    if (condNodes.size() > 1U) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot use more that one child to the \"" << prop << "\" element.";        
        return false;
    }

    static const ParseXmlWrap::ParseNamesList ElemNames = {
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
