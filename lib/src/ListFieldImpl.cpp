//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
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

#include "ListFieldImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cassert>

#include "common.h"
#include "ProtocolImpl.h"
#include "IntFieldImpl.h"

namespace commsdsl
{

namespace
{

const XmlWrap::NamesList& listSupportedTypes()
{
    static const XmlWrap::NamesList Names = FieldImpl::supportedTypes();
    return Names;
}

XmlWrap::NamesList getExtraNames()
{
    auto names = listSupportedTypes();
    names.push_back(common::elementStr());
    names.push_back(common::countPrefixStr());
    names.push_back(common::lengthPrefixStr());
    names.push_back(common::elemLengthPrefixStr());
    return names;
}

} // namespace

ListFieldImpl::ListFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}


FieldImpl::Kind ListFieldImpl::kindImpl() const
{
    return Kind::List;
}

ListFieldImpl::ListFieldImpl(const ListFieldImpl& other)
  : Base(other),
    m_state(other.m_state)
{
    cloneFields(other);
}

FieldImpl::Ptr ListFieldImpl::cloneImpl() const
{
    return Ptr(new ListFieldImpl(*this));
}

const XmlWrap::NamesList& ListFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::countStr(),
        common::elemFixedLengthStr()
    };

    return List;
}

const XmlWrap::NamesList& ListFieldImpl::extraPossiblePropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::elementStr(),
        common::countPrefixStr(),
        common::lengthPrefixStr(),
        common::elemLengthPrefixStr(),
    };
    return List;
}

const XmlWrap::NamesList& ListFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList List = getExtraNames();
    return List;
}

bool ListFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const ListFieldImpl&>(other);
    m_state = castedOther.m_state;
    cloneFields(castedOther);
    return true;
}

bool ListFieldImpl::verifySiblingsImpl(const FieldsList& fields) const
{
    return 
        verifySiblingsForPrefix(fields, m_state.m_detachedCountPrefixField) &&
        verifySiblingsForPrefix(fields, m_state.m_detachedLengthPrefixField) &&
        verifySiblingsForPrefix(fields, m_state.m_detachedElemLengthPrefixField);
}

bool ListFieldImpl::parseImpl()
{
    return
        updateElement() &&
        updateCount() &&
        updateCountPrefix() &&
        updateLengthPrefix() &&
        updateElemFixedLength() &&
        updateElemLengthPrefix();
        
}

std::size_t ListFieldImpl::minLengthImpl() const
{
    std::size_t extraLen = 0U;
    if (hasElemLengthPrefixField()) {
        extraLen += elemLengthPrefixField().minLength();
    }

    if (m_state.m_count != 0U) {
        assert(elementField().valid());
        auto elemMinLength = elementField().minLength();

        if (!m_state.m_elemFixedLength) {
            extraLen *= m_state.m_count;
        }

        return (m_state.m_count * elemMinLength) + extraLen;
    }

    if (hasCountPrefixField()) {
        return countPrefixField().minLength();
    }

    if (hasLengthPrefixField()) {
        return lengthPrefixField().minLength();
    }

    return 0U;
}

std::size_t ListFieldImpl::maxLengthImpl() const
{
    static const std::size_t MaxLen = std::numeric_limits<std::size_t>::max();
    std::size_t extraLen = 0U;
    if (hasElemLengthPrefixField()) {
        extraLen += elemLengthPrefixField().maxLength();
    }

    assert(elementField().valid());
    auto elemMaxLength = elementField().maxLength();
    if (m_state.m_count != 0U) {

        if (!m_state.m_elemFixedLength) {
            extraLen *= m_state.m_count;
        }

        return (m_state.m_count * elemMaxLength) + extraLen;
    }

    if (hasCountPrefixField()) {
        auto* prefixField = getCountPrefixField();
        assert(prefixField->kind() == Field::Kind::Int);
        auto& castedPrefix = static_cast<const IntFieldImpl&>(*prefixField);
        std::size_t count = static_cast<std::size_t>(castedPrefix.maxValue());
        std::size_t result = std::min(MaxLen / count, elemMaxLength) * count;

        if ((MaxLen - castedPrefix.maxLength()) <= result) {
            return MaxLen;
        }

        result += castedPrefix.maxLength();


        if (m_state.m_elemFixedLength) {
            if ((MaxLen - extraLen) <= result) {
                return MaxLen;
            }

            return result + extraLen;
        }

        extraLen *= std::min(MaxLen / count, extraLen) * count;

        if ((MaxLen - extraLen) <= result) {
            return MaxLen;
        }

        result += extraLen;
        return result;
    }

    if (hasLengthPrefixField()) {
        auto* prefixField = getLengthPrefixField();
        assert(prefixField->kind() == Field::Kind::Int);
        auto& castedPrefix = static_cast<const IntFieldImpl&>(*prefixField);
        std::size_t result = static_cast<std::size_t>(castedPrefix.maxValue());
        if ((MaxLen - castedPrefix.maxLength()) <= result) {
            return MaxLen;
        }
        return result + castedPrefix.maxLength();
    }

    return MaxLen;
}

void ListFieldImpl::cloneFields(const ListFieldImpl& other)
{
    if (other.m_elementField) {
        assert(other.m_state.m_extElementField == nullptr);
        m_elementField = other.m_elementField->clone();
    }

    if (other.m_countPrefixField) {
        assert(other.m_state.m_extCountPrefixField == nullptr);
        m_countPrefixField = other.m_countPrefixField->clone();
    }

    if (other.m_lengthPrefixField) {
        assert(other.m_state.m_extLengthPrefixField == nullptr);
        m_lengthPrefixField = other.m_lengthPrefixField->clone();
    }

    if (other.m_elemLengthPrefixField) {
        assert(other.m_state.m_extElemLengthPrefixField == nullptr);
        m_elemLengthPrefixField = other.m_elemLengthPrefixField->clone();
    }
}

bool ListFieldImpl::updateElement()
{
    if ((!checkElementFromRef()) ||
        (!checkElementAsChild())) {
        return false;
    }

    if (!hasElementField()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "List element hasn't been provided.";
        return false;
    }

    return true;
}


bool ListFieldImpl::updateCount()
{
    if (!validateSinglePropInstance(common::countStr())) {
        return false;
    }

    auto iter = props().find(common::countStr());
    if (iter == props().end()) {
        return true;
    }

    bool ok = false;
    auto newVal = common::strToUnsigned(iter->second, &ok);
    if ((!ok) || (newVal == 0U)) {
        reportUnexpectedPropertyValue(common::countStr(), iter->second);
        return false;
    }

    if ((hasCountPrefixField()) || (hasLengthPrefixField())) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot force fixed length after reusing data sequence with count or length prefixes.";
        return false;
    }

    m_state.m_count = newVal;
    return true;
}

bool ListFieldImpl::updateCountPrefix()
{
    if ((!checkPrefixFromRef(common::countPrefixStr(), m_state.m_extCountPrefixField, m_countPrefixField, m_state.m_detachedCountPrefixField)) ||
        (!checkPrefixAsChild(common::countPrefixStr(), m_state.m_extCountPrefixField, m_countPrefixField, m_state.m_detachedCountPrefixField))) {
        return false;
    }

    if ((!hasCountPrefixField()) && (m_state.m_detachedCountPrefixField.empty())) {
        return true;
    }

    if (m_state.m_count != 0U) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Element count prefix is not applicable to fixed length data sequences.";
        return false;
    }

    if (hasLengthPrefixField() || (!m_state.m_detachedLengthPrefixField.empty())) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Element count and serialisation length prefixes cannot be used together.";
        return false;
    }

    return true;
}

bool ListFieldImpl::updateLengthPrefix()
{
    if ((!checkPrefixFromRef(common::lengthPrefixStr(), m_state.m_extLengthPrefixField, m_lengthPrefixField, m_state.m_detachedLengthPrefixField)) ||
        (!checkPrefixAsChild(common::lengthPrefixStr(), m_state.m_extLengthPrefixField, m_lengthPrefixField, m_state.m_detachedLengthPrefixField))) {
        return false;
    }

    if ((!hasLengthPrefixField()) && (m_state.m_detachedLengthPrefixField.empty())) {
        return true;
    }

    if (m_state.m_count != 0U) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Serialisation length prefix is not applicable to fixed length data sequences.";
        return false;
    }

    if (hasCountPrefixField() || (!m_state.m_detachedCountPrefixField.empty())) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Element count and serialisation length prefixes cannot be used together.";
        return false;
    }

    return true;
}

bool ListFieldImpl::updateElemLengthPrefix()
{
    if ((!checkPrefixFromRef(common::elemLengthPrefixStr(), m_state.m_extElemLengthPrefixField, m_elemLengthPrefixField, m_state.m_detachedElemLengthPrefixField)) ||
        (!checkPrefixAsChild(common::elemLengthPrefixStr(), m_state.m_extElemLengthPrefixField, m_elemLengthPrefixField, m_state.m_detachedElemLengthPrefixField))) {
        return false;
    }

    if ((!m_state.m_detachedElemLengthPrefixField.empty()) &&
        (!m_state.m_elemFixedLength)) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Detached element length prefix is supported only for lists with fixed length elements. "
            "Set the \"" << common::elemFixedLengthStr() << "\" property.";
        return false;
    }

    return true;
}

bool ListFieldImpl::updateElemFixedLength()
{
    if (!validateSinglePropInstance(common::elemFixedLengthStr())) {
        return false;
    }

    do {
        auto iter = props().find(common::elemFixedLengthStr());
        if (iter == props().end()) {
            break;
        }

        bool ok = false;
        m_state.m_elemFixedLength = common::strToBool(iter->second, &ok);
        if (!ok) {
            reportUnexpectedPropertyValue(common::elemFixedLengthStr(), iter->second);
            return false;
        }
    } while (false);

    if (!m_state.m_elemFixedLength) {
        return true;
    }

    assert(hasElementField());
    auto elem = elementField();
    if (elem.minLength() != elem.maxLength()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot have \"" << common::elemFixedLengthStr() << "\" property being set to "
            "true when list element has variable length.";
        return false;
    }
    return true;
}

bool ListFieldImpl::checkElementFromRef()
{
    if (!validateSinglePropInstance(common::elementStr())) {
        return false;
    }

    auto iter = props().find(common::elementStr());
    if (iter == props().end()) {
        return true;
    }

    auto* field = protocol().findField(iter->second);
    if (field == nullptr) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot find field referenced by \"" << common::elementStr() <<
            "\" property (" << iter->second << ").";
        return false;
    }

    m_elementField.reset();
    m_state.m_extElementField = field;
    return true;
}

bool ListFieldImpl::checkElementAsChild()
{
    auto children = XmlWrap::getChildren(getNode(), common::elementStr());
    // if (children.empty()) {
    //     return true;
    // }

    if (1U < children.size()) {
        logError() << "There must be only one occurance of \"" << common::elementStr() << "\".";
        return false;
    }

    auto fieldTypes = XmlWrap::getChildren(getNode(), listSupportedTypes());
    if ((0U < children.size()) && (0U < fieldTypes.size())) {
        logError() << XmlWrap::logPrefix(getNode()) <<
                  "The \"" << common::listStr() << "\" does not support "
                  "stand alone field as child element together with \"" <<
                  common::elementStr() << "\" child element.";
        return false;
    }   

    if (children.empty() && fieldTypes.empty()) {
        return true;
    }     

    ::xmlNodePtr fieldNode = nullptr;
    do {
        if (fieldTypes.empty()) {
            break;
        }

        if (1U < fieldTypes.size()) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                "The \"" << common::listStr() << "\" element is expected to define only "
                "single child field";
            return false;
        }

        auto allChildren = XmlWrap::getChildren(getNode());
        if (allChildren.size() != fieldTypes.size()) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                  "The element type of \"" << common::listStr() <<
                  "\" must be defined inside \"<" << common::elementStr() << ">\" child element "
                  "when there are other property describing children.";
            return false;
        }

        if (props().find(common::elementStr()) != props().end()) {
            logError() << "There must be only one occurance of \"" << common::elementStr() << "\" definition.";
            return false;
        }

        fieldNode = fieldTypes.front();
    } while (false);    

    do {
        if (fieldNode != nullptr) {
            assert(children.empty());
            break;
        }

        assert(!children.empty());

        auto child = children.front();
        auto fields = XmlWrap::getChildren(child);
        if (1U < fields.size()) {
            logError() << XmlWrap::logPrefix(child) <<
                "The \"" << common::elementStr() << "\" element is expected to define only "
                "single field";
            return false;
        }

        if (props().find(common::elementStr()) == props().end()) {
            if (!fields.empty()) {
                fieldNode = fields.front();
                break;
            }

            logError() << XmlWrap::logPrefix(child) <<
                "The \"" << common::elementStr() << "\" node "
                "is expected to define field as child element";
            return false;
        }

        auto attrs = XmlWrap::parseNodeProps(getNode());
        if (attrs.find(common::fieldsStr()) != attrs.end()) {
            logError() << "There must be only one occurance of \"" << common::elementStr() << "\" definition.";
            return false;
        }

        // The field element is parsed as property
        return true;
    } while (false);

    assert (fieldNode != nullptr);    

    std::string fieldKind(reinterpret_cast<const char*>(fieldNode->name));
    auto field = FieldImpl::create(fieldKind, fieldNode, protocol());
    if (!field) {
        logError() << XmlWrap::logPrefix(fieldNode) <<
            "Unknown field type \"" << fieldKind << "\".";
        return false;
    }

    field->setParent(this);
    if (!field->parse()) {
        return false;
    }

    m_state.m_extElementField = nullptr;
    m_elementField = std::move(field);
    assert(m_elementField->externalRef().empty());
    return true;
}

bool ListFieldImpl::checkPrefixFromRef(
    const std::string& type,
    const FieldImpl*& extField,
    FieldImplPtr& locField,
    std::string& detachedPrefix)
{
    if (!validateSinglePropInstance(type)) {
        return false;
    }

    auto iter = props().find(type);
    if (iter == props().end()) {
        return true;
    }

    auto& str = iter->second;
    if (str.empty()) {
        reportUnexpectedPropertyValue(type, str);
        return false;
    }

    if (str[0] == '$') {
        if (!checkDetachedPrefixAllowed()) {
            return false;
        }

        detachedPrefix = std::string(str, 1);
        common::normaliseString(detachedPrefix);

        if (detachedPrefix.empty()) {
            reportUnexpectedPropertyValue(type, str);
            return false;
        }

        extField = nullptr;
        locField.reset();
        return true;
    }

    auto* field = protocol().findField(iter->second);
    if (field == nullptr) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot find field referenced by \"" << type <<
            "\" property (" << iter->second << ").";
        return false;
    }

    if (field->kind() != Kind::Int) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "The field referenced by \"" << type <<
            "\" property (" << iter->second << ") must be of type \"" << common::intStr() << "\".";
        return false;
    }

    locField.reset();
    extField = field;
    detachedPrefix.clear();
    return true;
}

bool ListFieldImpl::checkPrefixAsChild(
    const std::string& type,
    const FieldImpl*& extField,
    FieldImplPtr& locField,
    std::string& detachedPrefix)
{
    auto children = XmlWrap::getChildren(getNode(), type);
    if (children.empty()) {
        return true;
    }

    if (1U < children.size()) {
        logError() << "There must be only one occurance of \"" << type << "\".";
        return false;
    }

    auto child = children.front();
    auto prefixFields = XmlWrap::getChildren(child);
    if (1U < prefixFields.size()) {
        logError() << XmlWrap::logPrefix(child) <<
            "The \"" << type << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto iter = props().find(type);
    bool hasInProps = iter != props().end();
    if (prefixFields.empty()) {
        if (hasInProps) {
            return true;
        }

        logError() << XmlWrap::logPrefix(child) <<
            "The \"" << type << "\" element "
            "is expected to define field as child element";
        return false;

        assert(hasInProps);
        return true;
    }

    if (hasInProps) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "The \"" << type << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto fieldNode = prefixFields.front();
    assert(fieldNode->name != nullptr);
    std::string fieldKind(reinterpret_cast<const char*>(fieldNode->name));
    if (fieldKind != common::intStr()) {
        logError() << XmlWrap::logPrefix(fieldNode) <<
            "The field defined by \"" << type <<
            "\" element must be of type \"" << common::intStr() << "\".";
        return false;
    }

    auto field = FieldImpl::create(fieldKind, fieldNode, protocol());
    if (!field) {
        assert(!"Should not happen");
        return false;
    }

    field->setParent(this);
    if (!field->parse()) {
        return false;
    }

    extField = nullptr;
    locField = std::move(field);
    detachedPrefix.clear();
    return true;
}

const FieldImpl* ListFieldImpl::getCountPrefixField() const
{
    if (m_state.m_extCountPrefixField != nullptr) {
        assert(!m_countPrefixField);
        return m_state.m_extCountPrefixField;
    }

    assert(m_countPrefixField);
    return m_countPrefixField.get();
}

const FieldImpl* ListFieldImpl::getLengthPrefixField() const
{
    if (m_state.m_extLengthPrefixField != nullptr) {
        assert(!m_lengthPrefixField);
        return m_state.m_extLengthPrefixField;
    }

    assert(m_lengthPrefixField);
    return m_lengthPrefixField.get();
}

bool ListFieldImpl::verifySiblingsForPrefix(
    const FieldsList& fields, 
    const std::string& detachedName) const
{
    if (detachedName.empty()) {
        return true;
    }

    auto* sibling = findSibling(fields, detachedName);
    if (sibling == nullptr) {
        return false;
    }

    auto fieldKind = getNonRefFieldKind(*sibling);
    if (fieldKind != Kind::Int) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Detached prefix \"" << detachedName << "\" is expected to be of \"" << common::intStr() << "\" type.";
        return false;
    }

    return true;    
}

} // namespace commsdsl
