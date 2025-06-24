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

#include "ParseStringFieldImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cassert>

#include "parse_common.h"
#include "ParseProtocolImpl.h"
#include "ParseIntFieldImpl.h"
#include "ParseRefFieldImpl.h"
#include "parse_util.h"

namespace commsdsl
{

namespace parse
{

namespace
{

} // namespace

ParseStringFieldImpl::ParseStringFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol)
{
}


ParseFieldImpl::Kind ParseStringFieldImpl::parseKindImpl() const
{
    return Kind::String;
}

ParseStringFieldImpl::ParseStringFieldImpl(const ParseStringFieldImpl& other)
  : Base(other),
    m_state(other.m_state)
{
    if (other.m_prefixField) {
        assert(other.m_state.m_extPrefixField == nullptr);
        m_prefixField = other.m_prefixField->parseClone();
    }
}

ParseFieldImpl::Ptr ParseStringFieldImpl::parseCloneImpl() const
{
    return Ptr(new ParseStringFieldImpl(*this));
}

const ParseXmlWrap::NamesList& ParseStringFieldImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::lengthStr(),
        common::parseEncodingStr(),
        common::zeroTermSuffixStr(),
        common::defaultValueStr(),
        common::defaultValidValueStr(),
        common::validValueStr(),
    };

    return List;
}

const ParseXmlWrap::NamesList& ParseStringFieldImpl::parseExtraPossiblePropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::lengthPrefixStr(),
    };

    return List;
}

const ParseXmlWrap::NamesList& ParseStringFieldImpl::parseExtraChildrenNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::lengthPrefixStr()
    };

    return List;
}

bool ParseStringFieldImpl::parseReuseImpl(const ParseFieldImpl& other)
{
    assert(other.parseKind() == parseKind());
    auto& castedOther = static_cast<const ParseStringFieldImpl&>(other);
    m_state = castedOther.m_state;
    if (castedOther.m_prefixField) {
        assert(m_state.m_extPrefixField == nullptr);
        m_prefixField = castedOther.m_prefixField->parseClone();
    }
    else {
        assert(!m_prefixField);
    }
    return true;
}

bool ParseStringFieldImpl::parseImpl()
{
    return
        parseUpdateEncoding() &&
        parseUpdateLength() &&
        parseUpdatePrefix() &&
        parseUpdateZeroTerm() &&
        parseUpdateDefaultValue() &&
        parseUpdateDefaultValidValue() &&
        parseUpdateValidValues();
}

bool ParseStringFieldImpl::parseVerifySiblingsImpl(const ParseFieldImpl::FieldsList& fields) const
{
    if (m_state.m_detachedPrefixField.empty()) {
        return true;
    }

    auto* sibling = parseFindSibling(fields, m_state.m_detachedPrefixField);
    if (sibling == nullptr) {
        return false;
    }

    auto fieldKind = parseGetNonRefFieldKind(*sibling);
    if ((fieldKind != Kind::Int) && (sibling->parseSemanticType() != SemanticType::Length)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Detached length prefix is expected to be of \"" << common::intStr() << "\" type "
            "or have semanticType=\"length\" property set.";
        return false;
    }

    return true;
}

std::size_t ParseStringFieldImpl::parseMinLengthImpl() const
{
    if (m_state.m_length != 0U) {
        return m_state.m_length;
    }

    if (m_state.m_haxZeroSuffix) {
        return 1U;
    }

    if (parseHasPrefixField()) {
        return parseGetPrefixField()->parseMinLength();
    }

    return 0U;
}

std::size_t ParseStringFieldImpl::parseMaxLengthImpl() const
{
    if (m_state.m_length != 0U) {
        return m_state.m_length;
    }

    if (parseHasPrefixField()) {
        auto* prefixField = parseGetPrefixField();
        assert(prefixField->parseKind() == ParseField::Kind::Int);
        auto& castedPrefix = static_cast<const ParseIntFieldImpl&>(*prefixField);
        auto result = castedPrefix.parseMaxLength();
        common::addToLength(static_cast<std::size_t>(castedPrefix.parseMaxValue()), result);
        return result;
    }

    return common::maxPossibleLength();
}

bool ParseStringFieldImpl::parseIsComparableToValueImpl(const std::string& val) const
{
    if (val.empty()) {
        return true;        
    }

    static const char Prefix = common::stringRefPrefix();
    if (val[0] != Prefix) {
        return true;
    }

    auto* refField = parseProtocol().parseFindField(std::string(val, 1));
    if (refField == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Referenced field (" + val + ") is not defined.";
        return false;
    }

    if (refField->parseKind() != Kind::String) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Referenced field (" + val + ") is not <string>.";        
        return false;
    }

    return true;
}

bool ParseStringFieldImpl::parseStrToStringImpl(const std::string& ref, std::string& val) const
{
    if (!parseProtocol().parseIsFieldValueReferenceSupported()) {
        return false;
    }

    if (ref.empty()) {
        val = m_state.m_defaultValue;
        return true;
    }

    return false;
}

bool ParseStringFieldImpl::parseIsValidRefTypeImpl(FieldRefType type) const
{
    return (type == FieldRefType_Size);
}

bool ParseStringFieldImpl::parseUpdateDefaultValue()
{
    auto& propName = common::defaultValueStr();
    if (!parseValidateSinglePropInstance(propName)) {
        return false;
    }

    auto iter = parseProps().find(propName);
    do {
        if (iter == parseProps().end()) {
            break;
        }

        if (!parseStrToValue(iter->second, m_state.m_defaultValue)) {
            parseReportUnexpectedPropertyValue(propName, iter->second);
            return false;
        }
    } while (false);

    if ((m_state.m_length != 0U) && (m_state.m_length < m_state.m_defaultValue.size())) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The default value (" << m_state.m_defaultValue << ") is too long "
            "for proper serialisation.";
    }

    return true;
}

bool ParseStringFieldImpl::parseUpdateDefaultValidValue()
{
    auto& propName = common::defaultValidValueStr();
    if (!parseValidateSinglePropInstance(propName)) {
        return false;
    }

    auto iter = parseProps().find(propName);
    if (iter == parseProps().end()) {
        return true;
    }

    if (!parseProtocol().parseIsValidValueInStringAndDataSupported()) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) << 
            "Property \"" << common::defaultValidValueStr() << "\" for <string> field is not supported for DSL version " << parseProtocol().parseCurrSchema().parseDslVersion() << ", ignoring...";        
        return true;
    }     

    if (!parseStrToValue(iter->second, m_state.m_defaultValue)) {
        parseReportUnexpectedPropertyValue(propName, iter->second);
        return false;
    }    

    if ((m_state.m_length != 0U) && (m_state.m_length < m_state.m_defaultValue.size())) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The default value (" << m_state.m_defaultValue << ") is too long "
            "for proper serialisation.";
    }    

    ValidValueInfo info;
    info.m_value = m_state.m_defaultValue;
    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();
    m_state.m_validValues.push_back(std::move(info));

    return true;
}

bool ParseStringFieldImpl::parseUpdateEncoding()
{
    if (!parseValidateSinglePropInstance(common::parseEncodingStr())) {
        return false;
    }

    auto iter = parseProps().find(common::parseEncodingStr());
    if (iter != parseProps().end()) {
        m_state.m_encoding = iter->second;
    }

    return true;
}

bool ParseStringFieldImpl::parseUpdateLength()
{
    if (!parseValidateSinglePropInstance(common::lengthStr())) {
        return false;
    }

    auto iter = parseProps().find(common::lengthStr());
    if (iter == parseProps().end()) {
        return true;
    }

    bool ok = false;
    auto newVal = common::strToUnsigned(iter->second, &ok);
    if ((!ok) || (newVal == 0U)) {
        parseReportUnexpectedPropertyValue(common::lengthStr(), iter->second);
        return false;
    }

    if (m_state.m_length == newVal) {
        return true;
    }

    if (parseHasPrefixField()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot force fixed length after reusing string with length prefix.";
        return false;
    }

    if (m_state.m_haxZeroSuffix) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot force fixed length after reusing string with zero suffix.";
        return false;
    }

    m_state.m_length = newVal;
    return true;
}

bool ParseStringFieldImpl::parseUpdatePrefix()
{
    if ((!parseCheckPrefixFromRef()) ||
        (!parseCheckPrefixAsChild())) {
        return false;
    }

    if (!parseHasPrefixField()) {
        return true;
    }

    if (m_state.m_length != 0U) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Length prefix field is not applicable to fixed length strings.";
        return false;
    }

    if (m_state.m_haxZeroSuffix) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Length prefix field is not applicable to zero terminated strings.";
        return false;
    }

    return true;
}

bool ParseStringFieldImpl::parseUpdateZeroTerm()
{
    if (!parseValidateSinglePropInstance(common::zeroTermSuffixStr())) {
        return false;
    }

    auto iter = parseProps().find(common::zeroTermSuffixStr());
    if (iter == parseProps().end()) {
        return true;
    }

    bool ok = false;
    auto newVal = common::parseStrToBool(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::zeroTermSuffixStr(), iter->second);
        return false;
    }

    if (newVal == m_state.m_haxZeroSuffix) {
        return true;
    }

    if (!newVal) {
        m_state.m_haxZeroSuffix = newVal;
        return true;
    }

    if (m_state.m_length != 0U) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot apply zero suffix to fixed length strings.";
        return false;
    }

    if (parseHasPrefixField()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot apply zero suffix to strings having length prefix.";
        return false;
    }

    m_state.m_haxZeroSuffix = newVal;
    return true;
}

bool ParseStringFieldImpl::parseUpdateValidValues()
{
    if (!parseCheckValidValueAsAttr(ParseXmlWrap::parseNodeProps(parseGetNode()))) {
        return false;
    }

    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), common::validValueStr());
    for (auto* c : children) {
        if (!parseCheckValidValueAsChild(c)) {
            return false;
        }
    }

    std::sort(
        m_state.m_validValues.begin(), m_state.m_validValues.end(),
        [](auto& elem1, auto& elem2) {
            return elem1.m_value < elem2.m_value;
        });

    return true;
}

bool ParseStringFieldImpl::parseCheckPrefixFromRef()
{
    if (!parseValidateSinglePropInstance(common::lengthPrefixStr())) {
        return false;
    }

    auto iter = parseProps().find(common::lengthPrefixStr());
    if (iter == parseProps().end()) {
        return true;
    }

    auto& str = iter->second;
    if (str.empty()) {
        parseReportUnexpectedPropertyValue(common::lengthPrefixStr(), str);
        return false;
    }

    if (str[0] == common::siblingRefPrefix()) {
        if (!parseCheckDetachedPrefixAllowed()) {
            return false;
        }
        
        m_state.m_detachedPrefixField = std::string(str, 1);
        common::normaliseString(m_state.m_detachedPrefixField);

        if (m_state.m_detachedPrefixField.empty()) {
            parseReportUnexpectedPropertyValue(common::lengthPrefixStr(), str);
            return false;
        }

        m_state.m_extPrefixField = nullptr;
        m_prefixField.reset();
        return true;
    }

    auto* field = parseProtocol().parseFindField(iter->second);
    if (field == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot find field referenced by \"" << common::lengthPrefixStr() <<
            "\" property (" << iter->second << ").";
        return false;
    }

    if ((field->parseKind() != Kind::Int) && (field->parseSemanticType() != SemanticType::Length)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The field referenced by \"" << common::lengthPrefixStr() <<
            "\" property (" << iter->second << ") must be of type \"" << common::intStr() << 
            "\" or have semanticType=\"length\" property set.";
        return false;
    }

    m_prefixField.reset();
    m_state.m_extPrefixField = field;
    m_state.m_detachedPrefixField.clear();
    assert(parseHasPrefixField());
    return true;
}

bool ParseStringFieldImpl::parseCheckPrefixAsChild()
{
    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), common::lengthPrefixStr());
    if (children.empty()) {
        return true;
    }

    if (1U < children.size()) {
        parseLogError() << "There must be only one occurance of \"" << common::lengthPrefixStr() << "\".";
        return false;
    }

    auto child = children.front();
    auto prefixFields = ParseXmlWrap::parseGetChildren(child);
    if (1U < prefixFields.size()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(child) <<
            "The \"" << common::lengthPrefixStr() << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto iter = parseProps().find(common::lengthPrefixStr());
    bool hasInProps = iter != parseProps().end();
    if (prefixFields.empty()) {
        if (hasInProps) {
            return true;
        }

        parseLogError() << ParseXmlWrap::parseLogPrefix(child) <<
            "The \"" << common::lengthPrefixStr() << "\" element "
            "is expected to define field as child element";
        return false;
    }

    if (hasInProps) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The \"" << common::lengthPrefixStr() << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto fieldNode = prefixFields.front();
    assert(fieldNode->name != nullptr);
    std::string fieldKind(reinterpret_cast<const char*>(fieldNode->name));

    auto field = ParseFieldImpl::parseCreate(fieldKind, fieldNode, parseProtocol());
    if (!field) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return false;
    }

    field->parseSetParent(this);
    if (!field->parse()) {
        return false;
    }

    if ((field->parseKind() != Kind::Int) && (field->parseSemanticType() != SemanticType::Length)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The \"" << common::lengthPrefixStr() << "\" element must be of type \"" << common::intStr() << 
            "\" or have semanticType=\"length\" property set.";
        return false;
    }    

    m_state.m_extPrefixField = nullptr;
    m_state.m_detachedPrefixField.clear();
    m_prefixField = std::move(field);
    return true;
}

bool ParseStringFieldImpl::parseCheckValidValueAsAttr(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::validValueStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    if (!parseProtocol().parseIsValidValueInStringAndDataSupported()) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) << 
            "Property \"" << common::validValueStr() << "\" for <string> field is not supported for DSL version " << parseProtocol().parseCurrSchema().parseDslVersion() << ", ignoring...";        
        return true;
    }    

    ValidValueInfo info;
    if (!parseStrToValue(iter->second, info.m_value)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Property value \"" << common::validValueStr() << "\" of string element \"" <<
                      parseName() << "\" cannot be properly parsed.";
        return false;
    }    

    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();

    m_state.m_validValues.push_back(std::move(info));
    return true;
}

bool ParseStringFieldImpl::parseCheckValidValueAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!ParseXmlWrap::parseNodeValue(child, parseProtocol().parseLogger(), str)) {
        return false;
    }

    if (!parseProtocol().parseIsValidValueInStringAndDataSupported()) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) << 
            "Property \"" << common::validValueStr() << "\" for <string> field is not supported for DSL version " << parseProtocol().parseCurrSchema().parseDslVersion() << ", ignoring...";        
        return true;
    }        

    ValidValueInfo info;
    if (!parseStrToValue(str, info.m_value)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Property value \"" << common::validValueStr() << "\" of string element \"" <<
                      parseName() << "\" cannot be properly parsed.";
        return false;
    }    

    info.m_sinceVersion = parseGetSinceVersion();
    info.m_deprecatedSince = parseGetDeprecated();

    if (!ParseXmlWrap::parseGetAndCheckVersions(child, parseName(), info.m_sinceVersion, info.m_deprecatedSince, parseProtocol())) {
        return false;
    }

    m_state.m_validValues.push_back(std::move(info));
    return true;
}

const ParseFieldImpl* ParseStringFieldImpl::parseGetPrefixField() const
{
    if (m_state.m_extPrefixField != nullptr) {
        assert(!m_prefixField);
        return m_state.m_extPrefixField;
    }

    assert(m_prefixField);
    return m_prefixField.get();
}

bool ParseStringFieldImpl::parseStrToValue(const std::string& str, std::string& val) const
{
    return parseProtocol().parseStrToStringValue(str, val);
}


} // namespace parse

} // namespace commsdsl
