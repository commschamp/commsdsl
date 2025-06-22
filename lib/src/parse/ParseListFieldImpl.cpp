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

#include "ParseListFieldImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cassert>

#include "parse_common.h"
#include "ParseProtocolImpl.h"
#include "ParseIntFieldImpl.h"

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseXmlWrap::NamesList& listSupportedTypes()
{
    static const ParseXmlWrap::NamesList Names = ParseFieldImpl::supportedTypes();
    return Names;
}

ParseXmlWrap::NamesList getExtraNames()
{
    auto names = listSupportedTypes();
    names.push_back(common::elementStr());
    names.push_back(common::countPrefixStr());
    names.push_back(common::lengthPrefixStr());
    names.push_back(common::elemLengthPrefixStr());
    names.push_back(common::termSuffixStr());
    return names;
}

} // namespace

ParseListFieldImpl::ParseListFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol)
{
}


ParseFieldImpl::Kind ParseListFieldImpl::kindImpl() const
{
    return Kind::List;
}

ParseListFieldImpl::ParseListFieldImpl(const ParseListFieldImpl& other)
  : Base(other),
    m_state(other.m_state)
{
    cloneFields(other);
}

ParseFieldImpl::Ptr ParseListFieldImpl::cloneImpl() const
{
    return Ptr(new ParseListFieldImpl(*this));
}

const ParseXmlWrap::NamesList& ParseListFieldImpl::extraPropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::countStr(),
        common::elemFixedLengthStr()
    };

    return List;
}

const ParseXmlWrap::NamesList& ParseListFieldImpl::extraPossiblePropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::elementStr(),
        common::countPrefixStr(),
        common::lengthPrefixStr(),
        common::elemLengthPrefixStr(),
        common::termSuffixStr(),
    };
    return List;
}

const ParseXmlWrap::NamesList& ParseListFieldImpl::extraChildrenNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = getExtraNames();
    return List;
}

bool ParseListFieldImpl::reuseImpl(const ParseFieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const ParseListFieldImpl&>(other);
    m_state = castedOther.m_state;
    cloneFields(castedOther);
    return true;
}

bool ParseListFieldImpl::verifySiblingsImpl(const FieldsList& fields) const
{
    return 
        verifySiblingsForPrefix(fields, m_state.m_detachedCountPrefixField) &&
        verifySiblingsForPrefix(fields, m_state.m_detachedLengthPrefixField) &&
        verifySiblingsForPrefix(fields, m_state.m_detachedElemLengthPrefixField);
}

bool ParseListFieldImpl::parseImpl()
{
    return
        updateElement() &&
        updateCount() &&
        updateCountPrefix() &&
        updateLengthPrefix() &&
        updateElemFixedLength() &&
        updateElemLengthPrefix() &&
        updateTermSuffix();
        
}

std::size_t ParseListFieldImpl::minLengthImpl() const
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

    if (hasTermSuffixField()) {
        return termSuffixField().minLength();
    }    

    return 0U;
}

std::size_t ParseListFieldImpl::maxLengthImpl() const
{
    std::size_t extraLen = 0U;
    if (hasElemLengthPrefixField()) {
        common::addToLength(elemLengthPrefixField().maxLength(), extraLen);
    }

    assert(elementField().valid());
    auto elemMaxLength = elementField().maxLength();
    if (m_state.m_count != 0U) {

        if (!m_state.m_elemFixedLength) {
            extraLen = common::mulLength(extraLen, m_state.m_count);
        }

        common::addToLength(common::mulLength(elemMaxLength, m_state.m_count), extraLen);
        return extraLen;
    }

    if (hasCountPrefixField()) {
        auto* prefixField = getCountPrefixField();
        assert(prefixField->kind() == ParseField::Kind::Int);
        auto& castedPrefix = static_cast<const ParseIntFieldImpl&>(*prefixField);
        std::size_t count = static_cast<std::size_t>(castedPrefix.maxValue());
        auto result = common::mulLength(elemMaxLength, count);

        common::addToLength(castedPrefix.maxLength(), result);

        if (!m_state.m_elemFixedLength) {
            extraLen = common::mulLength(extraLen, count);
        }

        common::addToLength(extraLen, result);
        return result;
    }

    if (hasLengthPrefixField()) {
        auto* prefixField = getLengthPrefixField();
        assert(prefixField->kind() == ParseField::Kind::Int);
        auto& castedPrefix = static_cast<const ParseIntFieldImpl&>(*prefixField);
        std::size_t result = static_cast<std::size_t>(castedPrefix.maxValue());
        common::addToLength(castedPrefix.maxLength(), result);
        return result;
    }

    return common::maxPossibleLength();
}

bool ParseListFieldImpl::isValidRefTypeImpl(FieldRefType type) const
{
    return (type == FieldRefType_Size);
}

void ParseListFieldImpl::cloneFields(const ParseListFieldImpl& other)
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

    if (other.m_termSuffixField) {
        assert(other.m_state.m_extTermSuffixField == nullptr);
        m_termSuffixField = other.m_termSuffixField->clone();
    }    
}

bool ParseListFieldImpl::updateElement()
{
    if ((!checkElementFromRef()) ||
        (!checkElementAsChild())) {
        return false;
    }

    if (!hasElementField()) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "List element hasn't been provided.";
        return false;
    }

    return true;
}


bool ParseListFieldImpl::updateCount()
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

    if ((hasCountPrefixField()) || (hasLengthPrefixField()) || (hasTermSuffixField()) ||
        (!m_state.m_detachedCountPrefixField.empty()) ||
        (!m_state.m_detachedLengthPrefixField.empty()) ||
        (!m_state.m_detachedElemLengthPrefixField.empty()) ||
        (!m_state.m_detachedTermSuffixField.empty())) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Cannot use " << common::countStr() << " property after reusing list with " << 
            common::countPrefixStr() << ", " << common::lengthPrefixStr() << ", or" << common::termSuffixStr() << ".";
        return false;
    }

    m_state.m_count = newVal;
    return true;
}

bool ParseListFieldImpl::updateCountPrefix()
{
    if ((!checkPrefixFromRef(common::countPrefixStr(), m_state.m_extCountPrefixField, m_countPrefixField, m_state.m_detachedCountPrefixField)) ||
        (!checkPrefixAsChild(common::countPrefixStr(), m_state.m_extCountPrefixField, m_countPrefixField, m_state.m_detachedCountPrefixField))) {
        return false;
    }

    if ((!hasCountPrefixField()) && (m_state.m_detachedCountPrefixField.empty())) {
        return true;
    }

    if (m_state.m_count != 0U) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            common::countStr() << " and " << common::countPrefixStr() << " cannot be used together.";
        return false;
    }

    if (hasLengthPrefixField() || (!m_state.m_detachedLengthPrefixField.empty())) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            common::lengthPrefixStr() << " and " << common::countPrefixStr() << " cannot be used together.";
        return false;
    }

    if (hasTermSuffixField() || (!m_state.m_detachedTermSuffixField.empty())) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            common::termSuffixStr() << " and " << common::countPrefixStr() << " cannot be used together.";
        return false;
    }    

    return true;
}

bool ParseListFieldImpl::updateLengthPrefix()
{
    if ((!checkPrefixFromRef(common::lengthPrefixStr(), m_state.m_extLengthPrefixField, m_lengthPrefixField, m_state.m_detachedLengthPrefixField)) ||
        (!checkPrefixAsChild(common::lengthPrefixStr(), m_state.m_extLengthPrefixField, m_lengthPrefixField, m_state.m_detachedLengthPrefixField))) {
        return false;
    }

    if ((!hasLengthPrefixField()) && (m_state.m_detachedLengthPrefixField.empty())) {
        return true;
    }

    if (m_state.m_count != 0U) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            common::countStr() << " and " << common::lengthPrefixStr() << " cannot be used together.";
        return false;
    }

    if (hasCountPrefixField() || (!m_state.m_detachedCountPrefixField.empty())) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            common::countPrefixStr() << " and " << common::lengthPrefixStr() << " cannot be used together.";
        return false;
    }

    if (hasTermSuffixField() || (!m_state.m_detachedTermSuffixField.empty())) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            common::termSuffixStr() << " and " << common::lengthPrefixStr() << " cannot be used together.";
        return false;
    }     

    return true;
}

bool ParseListFieldImpl::updateElemLengthPrefix()
{
    if ((!checkPrefixFromRef(common::elemLengthPrefixStr(), m_state.m_extElemLengthPrefixField, m_elemLengthPrefixField, m_state.m_detachedElemLengthPrefixField)) ||
        (!checkPrefixAsChild(common::elemLengthPrefixStr(), m_state.m_extElemLengthPrefixField, m_elemLengthPrefixField, m_state.m_detachedElemLengthPrefixField))) {
        return false;
    }

    if ((!m_state.m_detachedElemLengthPrefixField.empty()) &&
        (!m_state.m_elemFixedLength)) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Detached element length prefix is supported only for lists with fixed length elements. "
            "Set the \"" << common::elemFixedLengthStr() << "\" property.";
        return false;
    }

    return true;
}

bool ParseListFieldImpl::updateElemFixedLength()
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
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Cannot have \"" << common::elemFixedLengthStr() << "\" property being set to "
            "true when list element has variable length.";
        return false;
    }
    return true;
}

bool ParseListFieldImpl::updateTermSuffix()
{
    auto& prop = common::termSuffixStr();
    if ((!checkPrefixFromRef(prop, m_state.m_extTermSuffixField, m_termSuffixField, m_state.m_detachedTermSuffixField)) ||
        (!checkPrefixAsChild(prop, m_state.m_extTermSuffixField, m_termSuffixField, m_state.m_detachedTermSuffixField))) {
        return false;
    }

    if ((!hasTermSuffixField()) && (m_state.m_detachedTermSuffixField.empty())) {
        return true;
    }

    if (!protocol().isPropertySupported(prop)) {
        logWarning() << ParseXmlWrap::logPrefix(getNode()) <<
            "Usage of the " << prop << " property is not supported for the used dslVersion, ignoring...";
        m_state.m_extTermSuffixField = nullptr;
        m_state.m_detachedTermSuffixField.clear();
        m_termSuffixField.reset();
        return true;
    }

    if (m_state.m_count != 0U) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            common::termSuffixStr() << " and " << common::countStr() << " cannot be used together.";
        return false;
    }

    if (hasCountPrefixField() || (!m_state.m_detachedCountPrefixField.empty())) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            common::termSuffixStr() << " and " << common::countPrefixStr() << " cannot be used together.";
        return false;
    }

    if (hasLengthPrefixField() || (!m_state.m_detachedLengthPrefixField.empty())) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            common::termSuffixStr() << " and " << common::lengthPrefixStr() << " cannot be used together.";
        return false;
    }    

    return true;
}

bool ParseListFieldImpl::checkElementFromRef()
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
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Cannot find field referenced by \"" << common::elementStr() <<
            "\" property (" << iter->second << ").";
        return false;
    }

    m_elementField.reset();
    m_state.m_extElementField = field;
    return true;
}

bool ParseListFieldImpl::checkElementAsChild()
{
    auto children = ParseXmlWrap::getChildren(getNode(), common::elementStr());
    // if (children.empty()) {
    //     return true;
    // }

    if (1U < children.size()) {
        logError() << "There must be only one occurance of \"" << common::elementStr() << "\".";
        return false;
    }

    auto fieldTypes = ParseXmlWrap::getChildren(getNode(), listSupportedTypes());
    if ((0U < children.size()) && (0U < fieldTypes.size())) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
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
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
                "The \"" << common::listStr() << "\" element is expected to define only "
                "single child field";
            return false;
        }

        auto allChildren = ParseXmlWrap::getChildren(getNode());
        if (allChildren.size() != fieldTypes.size()) {
            logError() << ParseXmlWrap::logPrefix(getNode()) <<
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
        auto fields = ParseXmlWrap::getChildren(child);
        if (1U < fields.size()) {
            logError() << ParseXmlWrap::logPrefix(child) <<
                "The \"" << common::elementStr() << "\" element is expected to define only "
                "single field";
            return false;
        }

        if (props().find(common::elementStr()) == props().end()) {
            if (!fields.empty()) {
                fieldNode = fields.front();
                break;
            }

            logError() << ParseXmlWrap::logPrefix(child) <<
                "The \"" << common::elementStr() << "\" node "
                "is expected to define field as child element";
            return false;
        }

        auto attrs = ParseXmlWrap::parseNodeProps(getNode());
        if (attrs.find(common::fieldsStr()) != attrs.end()) {
            logError() << "There must be only one occurance of \"" << common::elementStr() << "\" definition.";
            return false;
        }

        // The field element is parsed as property
        return true;
    } while (false);

    assert (fieldNode != nullptr);    

    std::string fieldKind(reinterpret_cast<const char*>(fieldNode->name));
    auto field = ParseFieldImpl::create(fieldKind, fieldNode, protocol());
    if (!field) {
        logError() << ParseXmlWrap::logPrefix(fieldNode) <<
            "Unknown field type \"" << fieldKind << "\".";
        return false;
    }

    field->setParent(this);
    if (!field->parse()) {
        return false;
    }

    m_state.m_extElementField = nullptr;
    m_elementField = std::move(field);
    assert(m_elementField->externalRef(false).empty());
    return true;
}

bool ParseListFieldImpl::checkPrefixFromRef(
    const std::string& type,
    const ParseFieldImpl*& extField,
    ParseFieldImplPtr& locField,
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

    if (str[0] == common::siblingRefPrefix()) {
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
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Cannot find field referenced by \"" << type <<
            "\" property (" << iter->second << ").";
        return false;
    }

    if ((field->kind() != Kind::Int) && (field->semanticType() != SemanticType::Length)) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "The field referenced by \"" << type <<
            "\" property (" << iter->second << ") must be of type \"" << common::intStr() << 
            "\" or have semanticType=\"length\" property set.";
        return false;
    }

    locField.reset();
    extField = field;
    detachedPrefix.clear();
    return true;
}

bool ParseListFieldImpl::checkPrefixAsChild(
    const std::string& type,
    const ParseFieldImpl*& extField,
    ParseFieldImplPtr& locField,
    std::string& detachedPrefix)
{
    auto children = ParseXmlWrap::getChildren(getNode(), type);
    if (children.empty()) {
        return true;
    }

    if (1U < children.size()) {
        logError() << "There must be only one occurance of \"" << type << "\".";
        return false;
    }

    auto child = children.front();
    auto prefixFields = ParseXmlWrap::getChildren(child);
    if (1U < prefixFields.size()) {
        logError() << ParseXmlWrap::logPrefix(child) <<
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

        logError() << ParseXmlWrap::logPrefix(child) <<
            "The \"" << type << "\" element "
            "is expected to define field as child element";
        return false;
    }

    if (hasInProps) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "The \"" << type << "\" element is expected to define only "
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
            "The \"" << type << "\" element  must be of type \"" << common::intStr() << 
            "\" or have semanticType=\"length\" property set.";
        return false;
    }      

    extField = nullptr;
    locField = std::move(field);
    detachedPrefix.clear();
    return true;
}

const ParseFieldImpl* ParseListFieldImpl::getCountPrefixField() const
{
    if (m_state.m_extCountPrefixField != nullptr) {
        assert(!m_countPrefixField);
        return m_state.m_extCountPrefixField;
    }

    assert(m_countPrefixField);
    return m_countPrefixField.get();
}

const ParseFieldImpl* ParseListFieldImpl::getLengthPrefixField() const
{
    if (m_state.m_extLengthPrefixField != nullptr) {
        assert(!m_lengthPrefixField);
        return m_state.m_extLengthPrefixField;
    }

    assert(m_lengthPrefixField);
    return m_lengthPrefixField.get();
}

bool ParseListFieldImpl::verifySiblingsForPrefix(
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
    if ((fieldKind != Kind::Int) && (sibling->semanticType() != SemanticType::Length)) {
        logError() << ParseXmlWrap::logPrefix(getNode()) <<
            "Detached prefix \"" << detachedName << "\" is expected to be of \"" << common::intStr() << "\" type "
            "or have semanticType=\"length\" property set.";
        return false;
    }

    return true;    
}

} // namespace parse

} // namespace commsdsl
