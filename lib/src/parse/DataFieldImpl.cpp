//
// Copyright 2018 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "DataFieldImpl.h"

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

DataFieldImpl::DataFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}


FieldImpl::Kind DataFieldImpl::kindImpl() const
{
    return Kind::Data;
}

DataFieldImpl::DataFieldImpl(const DataFieldImpl& other)
  : Base(other),
    m_state(other.m_state)
{
    if (other.m_prefixField) {
        assert(other.m_state.m_extPrefixField == nullptr);
        m_prefixField = other.m_prefixField->clone();
    }
}

FieldImpl::Ptr DataFieldImpl::cloneImpl() const
{
    return Ptr(new DataFieldImpl(*this));
}

const XmlWrap::NamesList& DataFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::lengthStr(),
        common::defaultValueStr()
    };

    return List;
}

const XmlWrap::NamesList&DataFieldImpl::extraPossiblePropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::lengthPrefixStr(),
    };

    return List;
}

const XmlWrap::NamesList& DataFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::lengthPrefixStr()
    };

    return List;
}

bool DataFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const DataFieldImpl&>(other);
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

bool DataFieldImpl::parseImpl()
{
    return
        updateLength() &&
        updatePrefix() &&
        updateDefaultValue();
}

bool DataFieldImpl::verifySiblingsImpl(const FieldImpl::FieldsList& fields) const
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

std::size_t DataFieldImpl::minLengthImpl() const
{
    if (m_state.m_length != 0U) {
        return m_state.m_length;
    }

    if (hasPrefixField()) {
        return getPrefixField()->minLength();
    }

    return 0U;
}

std::size_t DataFieldImpl::maxLengthImpl() const
{
    if (m_state.m_length != 0U) {
        return m_state.m_length;
    }

    if (hasPrefixField()) {
        auto* prefixField = getPrefixField();
        assert(prefixField->kind() == Field::Kind::Int);
        auto& castedPrefix = static_cast<const IntFieldImpl&>(*prefixField);
        auto len = castedPrefix.maxLength();
        common::addToLength(static_cast<std::size_t>(castedPrefix.maxValue()), len);
        return len;
    }

    return common::maxPossibleLength();
}

bool DataFieldImpl::strToDataImpl(const std::string& ref, std::vector<std::uint8_t>& val) const
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

bool DataFieldImpl::updateDefaultValue()
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
            logError() << XmlWrap::logPrefix(getNode()) <<
                "Property \"" << common::defaultValueStr() << "\" of element \"" << name() <<
                "\" has unexpected value (" << iter->second << "), expected to "
                "be hex values string with even number of non-white characters.";
            return false;
        }

    } while (false);
    if ((m_state.m_length != 0U) &&
        (m_state.m_length < m_state.m_defaultValue.size())) {
        logWarning() << XmlWrap::logPrefix(getNode()) <<
            "The default value is too long "
            "for proper serialisation.";
    }

    return true;
}

bool DataFieldImpl::updateLength()
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
            "Cannot force fixed length after reusing data sequence with length prefix.";
        return false;
    }

    m_state.m_length = newVal;
    return true;
}

bool DataFieldImpl::updatePrefix()
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
            "Length prefix field is not applicable to fixed length data sequences.";
        return false;
    }

    return true;
}

bool DataFieldImpl::checkPrefixFromRef()
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
    assert(hasPrefixField());
    m_state.m_detachedPrefixField.clear();
    return true;
}

bool DataFieldImpl::checkPrefixAsChild()
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
    m_prefixField = std::move(field);
    m_state.m_detachedPrefixField.clear();
    return true;
}

const FieldImpl* DataFieldImpl::getPrefixField() const
{
    if (m_state.m_extPrefixField != nullptr) {
        assert(!m_prefixField);
        return m_state.m_extPrefixField;
    }

    assert(m_prefixField);
    return m_prefixField.get();
}

bool DataFieldImpl::strToValue(const std::string& str, ValueType& val) const
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
            static constexpr bool Should_not_happen = false;
            static_cast<void>(Should_not_happen);
            assert(Should_not_happen);
            return false;
        }
    }

    assert(val.size() == (adjStr.size() / 2U));
    return true;
}

} // namespace parse

} // namespace commsdsl
