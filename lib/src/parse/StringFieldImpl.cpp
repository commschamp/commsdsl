//
// Copyright 2018 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "StringFieldImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cassert>

#include "common.h"
#include "ProtocolImpl.h"
#include "IntFieldImpl.h"
#include "RefFieldImpl.h"
#include "util.h"

namespace commsdsl
{

namespace parse
{

namespace
{

} // namespace

StringFieldImpl::StringFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}


FieldImpl::Kind StringFieldImpl::kindImpl() const
{
    return Kind::String;
}

StringFieldImpl::StringFieldImpl(const StringFieldImpl& other)
  : Base(other),
    m_state(other.m_state)
{
    if (other.m_prefixField) {
        assert(other.m_state.m_extPrefixField == nullptr);
        m_prefixField = other.m_prefixField->clone();
    }
}

FieldImpl::Ptr StringFieldImpl::cloneImpl() const
{
    return Ptr(new StringFieldImpl(*this));
}

const XmlWrap::NamesList& StringFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::lengthStr(),
        common::encodingStr(),
        common::zeroTermSuffixStr(),
        common::defaultValueStr()
    };

    return List;
}

const XmlWrap::NamesList& StringFieldImpl::extraPossiblePropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::lengthPrefixStr(),
    };

    return List;
}

const XmlWrap::NamesList& StringFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::lengthPrefixStr()
    };

    return List;
}

bool StringFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const StringFieldImpl&>(other);
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

bool StringFieldImpl::parseImpl()
{
    return
        updateEncoding() &&
        updateLength() &&
        updatePrefix() &&
        updateZeroTerm() &&
        updateDefaultValue();
}

bool StringFieldImpl::verifySiblingsImpl(const FieldImpl::FieldsList& fields) const
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
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Detached length prefix is expected to be of \"" << common::intStr() << "\" type "
            "or have semanticType=\"length\" property set.";
        return false;
    }

    return true;
}

std::size_t StringFieldImpl::minLengthImpl() const
{
    if (m_state.m_length != 0U) {
        return m_state.m_length;
    }

    if (m_state.m_haxZeroSuffix) {
        return 1U;
    }

    if (hasPrefixField()) {
        return getPrefixField()->minLength();
    }

    return 0U;
}

std::size_t StringFieldImpl::maxLengthImpl() const
{
    if (m_state.m_length != 0U) {
        return m_state.m_length;
    }

    if (hasPrefixField()) {
        auto* prefixField = getPrefixField();
        assert(prefixField->kind() == Field::Kind::Int);
        auto& castedPrefix = static_cast<const IntFieldImpl&>(*prefixField);
        auto result = castedPrefix.maxLength();
        common::addToLength(static_cast<std::size_t>(castedPrefix.maxValue()), result);
        return result;
    }

    return common::maxPossibleLength();
}

bool StringFieldImpl::isComparableToValueImpl(const std::string& val) const
{
    if (val.empty()) {
        return true;        
    }

    static const char Prefix = common::stringRefPrefix();
    if (val[0] != Prefix) {
        return true;
    }

    auto* refField = protocol().findField(std::string(val, 1));
    if (refField == nullptr) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Referenced field (" + val + ") is not defined.";
        return false;
    }

    if (refField->kind() != Kind::String) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Referenced field (" + val + ") is not <string>.";        
        return false;
    }

    return true;
}

bool StringFieldImpl::strToStringImpl(const std::string& ref, std::string& val) const
{
    if (!protocol().isFieldValueReferenceSupported()) {
        return false;
    }

    if (ref.empty()) {
        val = m_state.m_defaultValue;
        return true;
    }

    return false;
}

bool StringFieldImpl::isValidRefTypeImpl(FieldRefType type) const
{
    return (type == FieldRefType_Size);
}

bool StringFieldImpl::updateDefaultValue()
{
    auto& propName = common::defaultValueStr();
    if (!validateSinglePropInstance(propName)) {
        return false;
    }

    auto iter = props().find(propName);
    do {
        if (iter == props().end()) {
            break;
        }

        if (!strToValue(iter->second, m_state.m_defaultValue)) {
            reportUnexpectedPropertyValue(propName, iter->second);
            return false;
        }
    } while (false);

    if ((m_state.m_length != 0U) && (m_state.m_length < m_state.m_defaultValue.size())) {
        logWarning() << XmlWrap::logPrefix(getNode()) <<
            "The default value (" << m_state.m_defaultValue << ") is too long "
            "for proper serialisation.";
    }

    return true;
}

bool StringFieldImpl::updateEncoding()
{
    if (!validateSinglePropInstance(common::encodingStr())) {
        return false;
    }

    auto iter = props().find(common::encodingStr());
    if (iter != props().end()) {
        m_state.m_encoding = iter->second;
    }

    return true;
}

bool StringFieldImpl::updateLength()
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
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot force fixed length after reusing string with length prefix.";
        return false;
    }

    if (m_state.m_haxZeroSuffix) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot force fixed length after reusing string with zero suffix.";
        return false;
    }

    m_state.m_length = newVal;
    return true;
}

bool StringFieldImpl::updatePrefix()
{
    if ((!checkPrefixFromRef()) ||
        (!checkPrefixAsChild())) {
        return false;
    }

    if (!hasPrefixField()) {
        return true;
    }

    if (m_state.m_length != 0U) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Length prefix field is not applicable to fixed length strings.";
        return false;
    }

    if (m_state.m_haxZeroSuffix) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Length prefix field is not applicable to zero terminated strings.";
        return false;
    }

    return true;
}

bool StringFieldImpl::updateZeroTerm()
{
    if (!validateSinglePropInstance(common::zeroTermSuffixStr())) {
        return false;
    }

    auto iter = props().find(common::zeroTermSuffixStr());
    if (iter == props().end()) {
        return true;
    }

    bool ok = false;
    auto newVal = common::strToBool(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::zeroTermSuffixStr(), iter->second);
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
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot apply zero suffix to fixed length strings.";
        return false;
    }

    if (hasPrefixField()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot apply zero suffix to strings having length prefix.";
        return false;
    }

    m_state.m_haxZeroSuffix = newVal;
    return true;
}

bool StringFieldImpl::checkPrefixFromRef()
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
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot find field referenced by \"" << common::lengthPrefixStr() <<
            "\" property (" << iter->second << ").";
        return false;
    }

    if ((field->kind() != Kind::Int) && (field->semanticType() != SemanticType::Length)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "The field referenced by \"" << common::lengthPrefixStr() <<
            "\" property (" << iter->second << ") must be of type \"" << common::intStr() << 
            "\" or have semanticType=\"length\" property set.";
        return false;
    }

    m_prefixField.reset();
    m_state.m_extPrefixField = field;
    m_state.m_detachedPrefixField.clear();
    assert(hasPrefixField());
    return true;
}

bool StringFieldImpl::checkPrefixAsChild()
{
    auto children = XmlWrap::getChildren(getNode(), common::lengthPrefixStr());
    if (children.empty()) {
        return true;
    }

    if (1U < children.size()) {
        logError() << "There must be only one occurance of \"" << common::lengthPrefixStr() << "\".";
        return false;
    }

    auto child = children.front();
    auto prefixFields = XmlWrap::getChildren(child);
    if (1U < prefixFields.size()) {
        logError() << XmlWrap::logPrefix(child) <<
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

        logError() << XmlWrap::logPrefix(child) <<
            "The \"" << common::lengthPrefixStr() << "\" element "
            "is expected to define field as child element";
        return false;
    }

    if (hasInProps) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "The \"" << common::lengthPrefixStr() << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto fieldNode = prefixFields.front();
    assert(fieldNode->name != nullptr);
    std::string fieldKind(reinterpret_cast<const char*>(fieldNode->name));

    auto field = FieldImpl::create(fieldKind, fieldNode, protocol());
    if (!field) {
        static constexpr bool Should_not_happen = false;
        static_cast<void>(Should_not_happen);
        assert(Should_not_happen);
        return false;
    }

    field->setParent(this);
    if (!field->parse()) {
        return false;
    }

    if ((field->kind() != Kind::Int) && (field->semanticType() != SemanticType::Length)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "The \"" << common::lengthPrefixStr() << "\" element must be of type \"" << common::intStr() << 
            "\" or have semanticType=\"length\" property set.";
        return false;
    }    

    m_state.m_extPrefixField = nullptr;
    m_state.m_detachedPrefixField.clear();
    m_prefixField = std::move(field);
    return true;
}

const FieldImpl* StringFieldImpl::getPrefixField() const
{
    if (m_state.m_extPrefixField != nullptr) {
        assert(!m_prefixField);
        return m_state.m_extPrefixField;
    }

    assert(m_prefixField);
    return m_prefixField.get();
}

bool StringFieldImpl::strToValue(const std::string& str, std::string& val) const
{
    return protocol().strToStringValue(str, val);
}


} // namespace parse

} // namespace commsdsl
