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

#include "ParseDataFieldImpl.h"

#include <type_traits>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <iterator>
#include <limits>

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

ParseDataFieldImpl::ParseDataFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol)
{
}


ParseFieldImpl::Kind ParseDataFieldImpl::kindImpl() const
{
    return Kind::Data;
}

ParseDataFieldImpl::ParseDataFieldImpl(const ParseDataFieldImpl& other)
  : Base(other),
    m_state(other.m_state)
{
    if (other.m_prefixField) {
        assert(other.m_state.m_extPrefixField == nullptr);
        m_prefixField = other.m_prefixField->clone();
    }
}

ParseFieldImpl::Ptr ParseDataFieldImpl::cloneImpl() const
{
    return Ptr(new ParseDataFieldImpl(*this));
}

const ParseXmlWrap::NamesList& ParseDataFieldImpl::extraPropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::lengthStr(),
        common::defaultValueStr(),
        common::defaultValidValueStr(),
        common::validValueStr(),
    };

    return List;
}

const ParseXmlWrap::NamesList&ParseDataFieldImpl::extraPossiblePropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::lengthPrefixStr(),
    };

    return List;
}

const ParseXmlWrap::NamesList& ParseDataFieldImpl::extraChildrenNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::lengthPrefixStr()
    };

    return List;
}

bool ParseDataFieldImpl::reuseImpl(const ParseFieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const ParseDataFieldImpl&>(other);
    m_state = castedOther.m_state;
    if (castedOther.m_prefixField) {
        assert(m_state.m_extPrefixField == nullptr);
        m_prefixField = castedOther.m_prefixField->clone();
    }
    else {
        assert(!m_prefixField);
    }
    return true;
}

bool ParseDataFieldImpl::parseImpl()
{
    return
        updateLength() &&
        updatePrefix() &&
        updateDefaultValue() &&
        updateDefaultValidValue() &&
        updateValidValues();
}

bool ParseDataFieldImpl::verifySiblingsImpl(const ParseFieldImpl::FieldsList& fields) const
{
    if (m_state.m_detachedPrefixField.empty()) {
        return true;
    }

    auto* sibling = findSibling(fields, m_state.m_detachedPrefixField);
    if (sibling == nullptr) {
        return false;
    }

    auto fieldKind = getNonRefFieldKind(*sibling);
    if ((fieldKind != Kind::Int) && (sibling->semanticType() != SemanticType::Length)) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Detached length prefix is expected to be of \"" << common::intStr() << "\" type "
            "or have semanticType=\"length\" property set.";
        return false;
    }

    return true;

}

std::size_t ParseDataFieldImpl::minLengthImpl() const
{
    if (m_state.m_length != 0U) {
        return m_state.m_length;
    }

    if (hasPrefixField()) {
        return getPrefixField()->minLength();
    }

    return 0U;
}

std::size_t ParseDataFieldImpl::maxLengthImpl() const
{
    if (m_state.m_length != 0U) {
        return m_state.m_length;
    }

    if (hasPrefixField()) {
        auto* prefixField = getPrefixField();
        assert(prefixField->kind() == ParseField::Kind::Int);
        auto& castedPrefix = static_cast<const ParseIntFieldImpl&>(*prefixField);
        auto len = castedPrefix.maxLength();
        common::addToLength(static_cast<std::size_t>(castedPrefix.maxValue()), len);
        return len;
    }

    return common::maxPossibleLength();
}

bool ParseDataFieldImpl::strToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const
{
    if (!protocol().isFieldValueReferenceSupported()) {
        return false;
    }

    if (!ref.empty()) {
        return false;
    }

    val = m_state.m_defaultValue;
    return true;
}

bool ParseDataFieldImpl::isValidRefTypeImpl(FieldRefType type) const
{
    return (type == FieldRefType_Size);
}

bool ParseDataFieldImpl::updateDefaultValue()
{
    if (!validateSinglePropInstance(common::defaultValueStr())) {
        return false;
    }

    do {
        auto iter = props().find(common::defaultValueStr());
        if (iter == props().end()) {
            break;
        }

        if (!strToValue(iter->second, m_state.m_defaultValue)) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
                "Property \"" << common::defaultValueStr() << "\" of element \"" << name() <<
                "\" has unexpected value (" << iter->second << "), expected to "
                "be hex values string with even number of non-white characters.";
            return false;
        }

    } while (false);

    if ((m_state.m_length != 0U) && (m_state.m_length < m_state.m_defaultValue.size())) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
            "The default value is too long for proper serialisation.";
    }

    return true;
}

bool ParseDataFieldImpl::updateDefaultValidValue()
{
    auto& propName = common::defaultValidValueStr();
    if (!validateSinglePropInstance(propName)) {
        return false;
    }

    auto iter = props().find(propName);
    if (iter == props().end()) {
        return true;
    }

    if (!protocol().isValidValueInStringAndDataSupported()) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) << 
            "Property \"" << common::defaultValidValueStr() << "\" for <data> field is not supported for DSL version " << protocol().currSchema().dslVersion() << ", ignoring...";        
        return true;
    }     

    if (!strToValue(iter->second, m_state.m_defaultValue)) {
        reportUnexpectedPropertyValue(propName, iter->second);
        return false;
    }    

    if ((m_state.m_length != 0U) && (m_state.m_length < m_state.m_defaultValue.size())) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
            "The default value is too long for proper serialisation.";
    }

    ValidValueInfo info;
    info.m_value = m_state.m_defaultValue;
    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();
    m_state.m_validValues.push_back(std::move(info));

    return true;
}

bool ParseDataFieldImpl::updateLength()
{
    if (!validateSinglePropInstance(common::lengthStr())) {
        return false;
    }

    auto iter = props().find(common::lengthStr());
    if (iter == props().end()) {
        return true;
    }

    bool ok = false;
    auto newVal = common::strToUnsigned(iter->second, &ok);
    if ((!ok) || (newVal == 0U)) {
        reportUnexpectedPropertyValue(common::lengthStr(), iter->second);
        return false;
    }

    if (m_state.m_length == newVal) {
        return true;
    }

    if (hasPrefixField()) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Cannot force fixed length after reusing data sequence with length prefix.";
        return false;
    }

    m_state.m_length = newVal;
    return true;
}

bool ParseDataFieldImpl::updatePrefix()
{
    if ((!checkPrefixFromRef()) ||
        (!checkPrefixAsChild())) {
        return false;
    }

    if (!hasPrefixField()) {
        return true;
    }

    if (m_state.m_length != 0U) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Length prefix field is not applicable to fixed length data sequences.";
        return false;
    }

    return true;
}

bool ParseDataFieldImpl::updateValidValues()
{
    if (!checkValidValueAsAttr(ParseXmlWrap::parseNodeProps(getNode()))) {
        return false;
    }

    auto children = ParseXmlWrap::getChildren(getNode(), common::validValueStr());
    for (auto* c : children) {
        if (!checkValidValueAsChild(c)) {
            return false;
        }
    }

    std::sort(
        m_state.m_validValues.begin(), m_state.m_validValues.end(),
        [](auto& elem1, auto& elem2) {
            return std::lexicographical_compare(elem1.m_value.begin(), elem1.m_value.end(), elem2.m_value.begin(), elem2.m_value.end());
        });

    return true;    
}

bool ParseDataFieldImpl::checkPrefixFromRef()
{
    if (!validateSinglePropInstance(common::lengthPrefixStr())) {
        return false;
    }

    auto iter = props().find(common::lengthPrefixStr());
    if (iter == props().end()) {
        return true;
    }

    auto& str = iter->second;
    if (str.empty()) {
        reportUnexpectedPropertyValue(common::lengthPrefixStr(), str);
        return false;
    }

    if (str[0] == common::siblingRefPrefix()) {
        if (!checkDetachedPrefixAllowed()) {
            return false;
        }

        m_state.m_detachedPrefixField = std::string(str, 1);
        common::normaliseString(m_state.m_detachedPrefixField);

        if (m_state.m_detachedPrefixField.empty()) {
            reportUnexpectedPropertyValue(common::lengthPrefixStr(), str);
            return false;
        }

        m_state.m_extPrefixField = nullptr;
        m_prefixField.reset();
        return true;
    }

    auto* field = protocol().findField(iter->second);
    if (field == nullptr) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Cannot find field referenced by \"" << common::lengthPrefixStr() <<
            "\" property (" << iter->second << ").";
        return false;
    }

    if ((field->kind() != Kind::Int) && (field->semanticType() != SemanticType::Length)) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "The field referenced by \"" << common::lengthPrefixStr() <<
            "\" property (" << iter->second << ") must be of type \"" << common::intStr() << 
            "\" or have semanticType=\"length\" property set.";
        return false;
    }

    m_prefixField.reset();
    m_state.m_extPrefixField = field;
    assert(hasPrefixField());
    m_state.m_detachedPrefixField.clear();
    return true;
}

bool ParseDataFieldImpl::checkPrefixAsChild()
{
    auto children = ParseXmlWrap::getChildren(getNode(), common::lengthPrefixStr());
    if (children.empty()) {
        return true;
    }

    if (1U < children.size()) {
        logError() << "There must be only one occurance of \"" << common::lengthPrefixStr() << "\".";
        return false;
    }

    auto child = children.front();
    auto prefixFields = ParseXmlWrap::getChildren(child);
    if (1U < prefixFields.size()) {
        logError() << ParseXmlWrap::logPrefix(child) <<
            "The \"" << common::lengthPrefixStr() << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto iter = props().find(common::lengthPrefixStr());
    bool hasInProps = iter != props().end();
    if (prefixFields.empty()) {
        if (hasInProps) {
            return true;
        }

        logError() << ParseXmlWrap::logPrefix(child) <<
            "The \"" << common::lengthPrefixStr() << "\" element "
            "is expected to define field as child element";

        return false;
    }

    if (hasInProps) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "The \"" << common::lengthPrefixStr() << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto fieldNode = prefixFields.front();
    assert(fieldNode->name != nullptr);
    std::string fieldKind(reinterpret_cast<const char*>(fieldNode->name));

    auto field = ParseFieldImpl::create(fieldKind, fieldNode, protocol());
    if (!field) {
        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return false;
    }

    field->setParent(this);
    if (!field->parse()) {
        return false;
    }

    if ((field->kind() != Kind::Int) && (field->semanticType() != SemanticType::Length)) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "The \"" << common::lengthPrefixStr() << "\" element must be of type \"" << common::intStr() << 
            "\" or have semanticType=\"length\" property set.";
        return false;
    }      

    m_state.m_extPrefixField = nullptr;
    m_prefixField = std::move(field);
    m_state.m_detachedPrefixField.clear();
    return true;
}


bool ParseDataFieldImpl::checkValidValueAsAttr(const ParseFieldImpl::PropsMap& xmlAttrs)
{
    auto iter = xmlAttrs.find(common::validValueStr());
    if (iter == xmlAttrs.end()) {
        return true;
    }

    if (!protocol().isValidValueInStringAndDataSupported()) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) << 
            "Property \"" << common::validValueStr() << "\" for <data> field is not supported for DSL version " << protocol().currSchema().dslVersion() << ", ignoring...";        
        return true;
    }    

    ValidValueInfo info;
    if (!strToValue(iter->second, info.m_value)) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
                      "Property value \"" << common::validValueStr() << "\" of <data> element \"" <<
                      name() << "\" cannot be properly parsed.";
        return false;
    }    

    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();

    m_state.m_validValues.push_back(std::move(info));
    return true;
}

bool ParseDataFieldImpl::checkValidValueAsChild(::xmlNodePtr child)
{
    std::string str;
    if (!ParseXmlWrap::parseNodeValue(child, protocol().logger(), str)) {
        return false;
    }

    if (!protocol().isValidValueInStringAndDataSupported()) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) << 
            "Property \"" << common::validValueStr() << "\" for <data> field is not supported for DSL version " << protocol().currSchema().dslVersion() << ", ignoring...";        
        return true;
    }        

    ValidValueInfo info;
    if (!strToValue(str, info.m_value)) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
                      "Property value \"" << common::validValueStr() << "\" of <data> element \"" <<
                      name() << "\" cannot be properly parsed.";
        return false;
    }    

    info.m_sinceVersion = getSinceVersion();
    info.m_deprecatedSince = getDeprecated();

    if (!ParseXmlWrap::getAndCheckVersions(child, name(), info.m_sinceVersion, info.m_deprecatedSince, protocol())) {
        return false;
    }

    m_state.m_validValues.push_back(std::move(info));
    return true;
}


const ParseFieldImpl* ParseDataFieldImpl::getPrefixField() const
{
    if (m_state.m_extPrefixField != nullptr) {
        assert(!m_prefixField);
        return m_state.m_extPrefixField;
    }

    assert(m_prefixField);
    return m_prefixField.get();
}

bool ParseDataFieldImpl::strToValue(const std::string& str, ValueType& val) const
{
    if ((!str.empty()) && (str[0] == common::stringRefPrefix()) && protocol().isFieldValueReferenceSupported()) {
        return protocol().strToData(std::string(str, 1), false, val);
    }

    std::string adjStr = str;
    adjStr.erase(
        std::remove(adjStr.begin(), adjStr.end(), ' '),
        adjStr.end());

    if ((adjStr.size() %2) != 0) {
        return false;
    }

    auto validChars =
        std::all_of(
            adjStr.begin(), adjStr.end(),
            [](char ch)
            {
                auto c = static_cast<char>(std::tolower(ch));
                return (('0' <= c) && (c <='9')) ||
                       (('a' <= c) && (c <= 'f'));
            });

    if (!validChars) {
        return false;
    }

    val.clear();
    val.reserve(adjStr.size() / 2U);
    std::string byteStr;
    for (auto ch : adjStr) {
        byteStr.push_back(ch);
        if (byteStr.size() <= 1U) {
            continue;
        }

        try {
            auto byte = static_cast<std::uint8_t>(std::stoul(byteStr, 0, 16));
            val.push_back(byte);
            byteStr.clear();
        }
        catch (...) {
            [[maybe_unused]] static constexpr bool Should_not_happen = false;
            assert(Should_not_happen);
            return false;
        }
    }

    assert(val.size() == (adjStr.size() / 2U));
    return true;
}

} // namespace parse

} // namespace commsdsl
