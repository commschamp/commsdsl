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

const ParseXmlWrap::NamesList& parseListSupportedTypes()
{
    static const ParseXmlWrap::NamesList Names = ParseFieldImpl::parseSupportedTypes();
    return Names;
}

ParseXmlWrap::NamesList parseGetExtraNames()
{
    auto names = parseListSupportedTypes();
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


ParseFieldImpl::Kind ParseListFieldImpl::parseKindImpl() const
{
    return Kind::List;
}

ParseListFieldImpl::ParseListFieldImpl(const ParseListFieldImpl& other)
  : Base(other),
    m_state(other.m_state)
{
    parseCloneFields(other);
}

ParseFieldImpl::Ptr ParseListFieldImpl::parseCloneImpl() const
{
    return Ptr(new ParseListFieldImpl(*this));
}

const ParseXmlWrap::NamesList& ParseListFieldImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::countStr(),
        common::elemFixedLengthStr()
    };

    return List;
}

const ParseXmlWrap::NamesList& ParseListFieldImpl::parseExtraPossiblePropsNamesImpl() const
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

const ParseXmlWrap::NamesList& ParseListFieldImpl::parseExtraChildrenNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = parseGetExtraNames();
    return List;
}

bool ParseListFieldImpl::parseReuseImpl(const ParseFieldImpl& other)
{
    assert(other.parseKind() == parseKind());
    auto& castedOther = static_cast<const ParseListFieldImpl&>(other);
    m_state = castedOther.m_state;
    parseCloneFields(castedOther);
    return true;
}

bool ParseListFieldImpl::parseVerifySiblingsImpl(const FieldsList& fields) const
{
    return 
        parseVerifySiblingsForPrefix(fields, m_state.m_detachedCountPrefixField) &&
        parseVerifySiblingsForPrefix(fields, m_state.m_detachedLengthPrefixField) &&
        parseVerifySiblingsForPrefix(fields, m_state.m_detachedElemLengthPrefixField);
}

bool ParseListFieldImpl::parseImpl()
{
    return
        parseUpdateElement() &&
        parseUpdateCount() &&
        parseUpdateCountPrefix() &&
        parseUpdateLengthPrefix() &&
        parseUpdateElemFixedLength() &&
        parseUpdateElemLengthPrefix() &&
        parseUpdateTermSuffix();
        
}

std::size_t ParseListFieldImpl::parseMinLengthImpl() const
{
    std::size_t extraLen = 0U;
    if (parseHasElemLengthPrefixField()) {
        extraLen += parseElemLengthPrefixField().parseMinLength();
    }

    if (m_state.m_count != 0U) {
        assert(parseElementField().parseValid());
        auto elemMinLength = parseElementField().parseMinLength();

        if (!m_state.m_elemFixedLength) {
            extraLen *= m_state.m_count;
        }

        return (m_state.m_count * elemMinLength) + extraLen;
    }

    if (parseHasCountPrefixField()) {
        return parseCountPrefixField().parseMinLength();
    }

    if (parseHasLengthPrefixField()) {
        return parseLengthPrefixField().parseMinLength();
    }

    if (parseHasTermSuffixField()) {
        return parseTermSuffixField().parseMinLength();
    }    

    return 0U;
}

std::size_t ParseListFieldImpl::parseMaxLengthImpl() const
{
    std::size_t extraLen = 0U;
    if (parseHasElemLengthPrefixField()) {
        common::addToLength(parseElemLengthPrefixField().parseMaxLength(), extraLen);
    }

    assert(parseElementField().parseValid());
    auto elemMaxLength = parseElementField().parseMaxLength();
    if (m_state.m_count != 0U) {

        if (!m_state.m_elemFixedLength) {
            extraLen = common::mulLength(extraLen, m_state.m_count);
        }

        common::addToLength(common::mulLength(elemMaxLength, m_state.m_count), extraLen);
        return extraLen;
    }

    if (parseHasCountPrefixField()) {
        auto* prefixField = parseGetCountPrefixField();
        assert(prefixField->parseKind() == ParseField::Kind::Int);
        auto& castedPrefix = static_cast<const ParseIntFieldImpl&>(*prefixField);
        std::size_t count = static_cast<std::size_t>(castedPrefix.parseMaxValue());
        auto result = common::mulLength(elemMaxLength, count);

        common::addToLength(castedPrefix.parseMaxLength(), result);

        if (!m_state.m_elemFixedLength) {
            extraLen = common::mulLength(extraLen, count);
        }

        common::addToLength(extraLen, result);
        return result;
    }

    if (parseHasLengthPrefixField()) {
        auto* prefixField = parseGetLengthPrefixField();
        assert(prefixField->parseKind() == ParseField::Kind::Int);
        auto& castedPrefix = static_cast<const ParseIntFieldImpl&>(*prefixField);
        std::size_t result = static_cast<std::size_t>(castedPrefix.parseMaxValue());
        common::addToLength(castedPrefix.parseMaxLength(), result);
        return result;
    }

    return common::maxPossibleLength();
}

bool ParseListFieldImpl::parseIsValidRefTypeImpl(FieldRefType type) const
{
    return (type == FieldRefType_Size);
}

void ParseListFieldImpl::parseCloneFields(const ParseListFieldImpl& other)
{
    if (other.m_elementField) {
        assert(other.m_state.m_extElementField == nullptr);
        m_elementField = other.m_elementField->parseClone();
    }

    if (other.m_countPrefixField) {
        assert(other.m_state.m_extCountPrefixField == nullptr);
        m_countPrefixField = other.m_countPrefixField->parseClone();
    }

    if (other.m_lengthPrefixField) {
        assert(other.m_state.m_extLengthPrefixField == nullptr);
        m_lengthPrefixField = other.m_lengthPrefixField->parseClone();
    }

    if (other.m_elemLengthPrefixField) {
        assert(other.m_state.m_extElemLengthPrefixField == nullptr);
        m_elemLengthPrefixField = other.m_elemLengthPrefixField->parseClone();
    }

    if (other.m_termSuffixField) {
        assert(other.m_state.m_extTermSuffixField == nullptr);
        m_termSuffixField = other.m_termSuffixField->parseClone();
    }    
}

bool ParseListFieldImpl::parseUpdateElement()
{
    if ((!parseCheckElementFromRef()) ||
        (!parseCheckElementAsChild())) {
        return false;
    }

    if (!parseHasElementField()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "List element hasn't been provided.";
        return false;
    }

    return true;
}


bool ParseListFieldImpl::parseUpdateCount()
{
    if (!parseValidateSinglePropInstance(common::countStr())) {
        return false;
    }

    auto iter = parseProps().find(common::countStr());
    if (iter == parseProps().end()) {
        return true;
    }

    bool ok = false;
    auto newVal = common::strToUnsigned(iter->second, &ok);
    if ((!ok) || (newVal == 0U)) {
        parseReportUnexpectedPropertyValue(common::countStr(), iter->second);
        return false;
    }

    if ((parseHasCountPrefixField()) || (parseHasLengthPrefixField()) || (parseHasTermSuffixField()) ||
        (!m_state.m_detachedCountPrefixField.empty()) ||
        (!m_state.m_detachedLengthPrefixField.empty()) ||
        (!m_state.m_detachedElemLengthPrefixField.empty()) ||
        (!m_state.m_detachedTermSuffixField.empty())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot use " << common::countStr() << " property after reusing list with " << 
            common::countPrefixStr() << ", " << common::lengthPrefixStr() << ", or" << common::termSuffixStr() << ".";
        return false;
    }

    m_state.m_count = newVal;
    return true;
}

bool ParseListFieldImpl::parseUpdateCountPrefix()
{
    if ((!parseCheckPrefixFromRef(common::countPrefixStr(), m_state.m_extCountPrefixField, m_countPrefixField, m_state.m_detachedCountPrefixField)) ||
        (!parseCheckPrefixAsChild(common::countPrefixStr(), m_state.m_extCountPrefixField, m_countPrefixField, m_state.m_detachedCountPrefixField))) {
        return false;
    }

    if ((!parseHasCountPrefixField()) && (m_state.m_detachedCountPrefixField.empty())) {
        return true;
    }

    if (m_state.m_count != 0U) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::countStr() << " and " << common::countPrefixStr() << " cannot be used together.";
        return false;
    }

    if (parseHasLengthPrefixField() || (!m_state.m_detachedLengthPrefixField.empty())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::lengthPrefixStr() << " and " << common::countPrefixStr() << " cannot be used together.";
        return false;
    }

    if (parseHasTermSuffixField() || (!m_state.m_detachedTermSuffixField.empty())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::termSuffixStr() << " and " << common::countPrefixStr() << " cannot be used together.";
        return false;
    }    

    return true;
}

bool ParseListFieldImpl::parseUpdateLengthPrefix()
{
    if ((!parseCheckPrefixFromRef(common::lengthPrefixStr(), m_state.m_extLengthPrefixField, m_lengthPrefixField, m_state.m_detachedLengthPrefixField)) ||
        (!parseCheckPrefixAsChild(common::lengthPrefixStr(), m_state.m_extLengthPrefixField, m_lengthPrefixField, m_state.m_detachedLengthPrefixField))) {
        return false;
    }

    if ((!parseHasLengthPrefixField()) && (m_state.m_detachedLengthPrefixField.empty())) {
        return true;
    }

    if (m_state.m_count != 0U) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::countStr() << " and " << common::lengthPrefixStr() << " cannot be used together.";
        return false;
    }

    if (parseHasCountPrefixField() || (!m_state.m_detachedCountPrefixField.empty())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::countPrefixStr() << " and " << common::lengthPrefixStr() << " cannot be used together.";
        return false;
    }

    if (parseHasTermSuffixField() || (!m_state.m_detachedTermSuffixField.empty())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::termSuffixStr() << " and " << common::lengthPrefixStr() << " cannot be used together.";
        return false;
    }     

    return true;
}

bool ParseListFieldImpl::parseUpdateElemLengthPrefix()
{
    if ((!parseCheckPrefixFromRef(common::elemLengthPrefixStr(), m_state.m_extElemLengthPrefixField, m_elemLengthPrefixField, m_state.m_detachedElemLengthPrefixField)) ||
        (!parseCheckPrefixAsChild(common::elemLengthPrefixStr(), m_state.m_extElemLengthPrefixField, m_elemLengthPrefixField, m_state.m_detachedElemLengthPrefixField))) {
        return false;
    }

    if ((!m_state.m_detachedElemLengthPrefixField.empty()) &&
        (!m_state.m_elemFixedLength)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Detached element length prefix is supported only for lists with fixed length elements. "
            "Set the \"" << common::elemFixedLengthStr() << "\" property.";
        return false;
    }

    return true;
}

bool ParseListFieldImpl::parseUpdateElemFixedLength()
{
    if (!parseValidateSinglePropInstance(common::elemFixedLengthStr())) {
        return false;
    }

    do {
        auto iter = parseProps().find(common::elemFixedLengthStr());
        if (iter == parseProps().end()) {
            break;
        }

        bool ok = false;
        m_state.m_elemFixedLength = common::parseStrToBool(iter->second, &ok);
        if (!ok) {
            parseReportUnexpectedPropertyValue(common::elemFixedLengthStr(), iter->second);
            return false;
        }
    } while (false);

    if (!m_state.m_elemFixedLength) {
        return true;
    }

    assert(parseHasElementField());
    auto elem = parseElementField();
    if (elem.parseMinLength() != elem.parseMaxLength()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot have \"" << common::elemFixedLengthStr() << "\" property being set to "
            "true when list element has variable length.";
        return false;
    }
    return true;
}

bool ParseListFieldImpl::parseUpdateTermSuffix()
{
    auto& prop = common::termSuffixStr();
    if ((!parseCheckPrefixFromRef(prop, m_state.m_extTermSuffixField, m_termSuffixField, m_state.m_detachedTermSuffixField)) ||
        (!parseCheckPrefixAsChild(prop, m_state.m_extTermSuffixField, m_termSuffixField, m_state.m_detachedTermSuffixField))) {
        return false;
    }

    if ((!parseHasTermSuffixField()) && (m_state.m_detachedTermSuffixField.empty())) {
        return true;
    }

    if (!parseProtocol().parseIsPropertySupported(prop)) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Usage of the " << prop << " property is not supported for the used dslVersion, ignoring...";
        m_state.m_extTermSuffixField = nullptr;
        m_state.m_detachedTermSuffixField.clear();
        m_termSuffixField.reset();
        return true;
    }

    if (m_state.m_count != 0U) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::termSuffixStr() << " and " << common::countStr() << " cannot be used together.";
        return false;
    }

    if (parseHasCountPrefixField() || (!m_state.m_detachedCountPrefixField.empty())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::termSuffixStr() << " and " << common::countPrefixStr() << " cannot be used together.";
        return false;
    }

    if (parseHasLengthPrefixField() || (!m_state.m_detachedLengthPrefixField.empty())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::termSuffixStr() << " and " << common::lengthPrefixStr() << " cannot be used together.";
        return false;
    }    

    return true;
}

bool ParseListFieldImpl::parseCheckElementFromRef()
{
    if (!parseValidateSinglePropInstance(common::elementStr())) {
        return false;
    }

    auto iter = parseProps().find(common::elementStr());
    if (iter == parseProps().end()) {
        return true;
    }

    auto* field = parseProtocol().parseFindField(iter->second);
    if (field == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot find field referenced by \"" << common::elementStr() <<
            "\" property (" << iter->second << ").";
        return false;
    }

    m_elementField.reset();
    m_state.m_extElementField = field;
    return true;
}

bool ParseListFieldImpl::parseCheckElementAsChild()
{
    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), common::elementStr());
    // if (children.empty()) {
    //     return true;
    // }

    if (1U < children.size()) {
        parseLogError() << "There must be only one occurance of \"" << common::elementStr() << "\".";
        return false;
    }

    auto fieldTypes = ParseXmlWrap::parseGetChildren(parseGetNode(), parseListSupportedTypes());
    if ((0U < children.size()) && (0U < fieldTypes.size())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
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
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "The \"" << common::listStr() << "\" element is expected to define only "
                "single child field";
            return false;
        }

        auto allChildren = ParseXmlWrap::parseGetChildren(parseGetNode());
        if (allChildren.size() != fieldTypes.size()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                  "The element type of \"" << common::listStr() <<
                  "\" must be defined inside \"<" << common::elementStr() << ">\" child element "
                  "when there are other property describing children.";
            return false;
        }

        if (parseProps().find(common::elementStr()) != parseProps().end()) {
            parseLogError() << "There must be only one occurance of \"" << common::elementStr() << "\" definition.";
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
        auto fields = ParseXmlWrap::parseGetChildren(child);
        if (1U < fields.size()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(child) <<
                "The \"" << common::elementStr() << "\" element is expected to define only "
                "single field";
            return false;
        }

        if (parseProps().find(common::elementStr()) == parseProps().end()) {
            if (!fields.empty()) {
                fieldNode = fields.front();
                break;
            }

            parseLogError() << ParseXmlWrap::parseLogPrefix(child) <<
                "The \"" << common::elementStr() << "\" node "
                "is expected to define field as child element";
            return false;
        }

        auto attrs = ParseXmlWrap::parseNodeProps(parseGetNode());
        if (attrs.find(common::fieldsStr()) != attrs.end()) {
            parseLogError() << "There must be only one occurance of \"" << common::elementStr() << "\" definition.";
            return false;
        }

        // The field element is parsed as property
        return true;
    } while (false);

    assert (fieldNode != nullptr);    

    std::string fieldKind(reinterpret_cast<const char*>(fieldNode->name));
    auto field = ParseFieldImpl::parseCreate(fieldKind, fieldNode, parseProtocol());
    if (!field) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(fieldNode) <<
            "Unknown field type \"" << fieldKind << "\".";
        return false;
    }

    field->parseSetParent(this);
    if (!field->parse()) {
        return false;
    }

    m_state.m_extElementField = nullptr;
    m_elementField = std::move(field);
    assert(m_elementField->parseExternalRef(false).empty());
    return true;
}

bool ParseListFieldImpl::parseCheckPrefixFromRef(
    const std::string& type,
    const ParseFieldImpl*& extField,
    ParseFieldImplPtr& locField,
    std::string& detachedPrefix)
{
    if (!parseValidateSinglePropInstance(type)) {
        return false;
    }

    auto iter = parseProps().find(type);
    if (iter == parseProps().end()) {
        return true;
    }

    auto& str = iter->second;
    if (str.empty()) {
        parseReportUnexpectedPropertyValue(type, str);
        return false;
    }

    if (str[0] == common::siblingRefPrefix()) {
        if (!parseCheckDetachedPrefixAllowed()) {
            return false;
        }

        detachedPrefix = std::string(str, 1);
        common::normaliseString(detachedPrefix);

        if (detachedPrefix.empty()) {
            parseReportUnexpectedPropertyValue(type, str);
            return false;
        }

        extField = nullptr;
        locField.reset();
        return true;
    }

    auto* field = parseProtocol().parseFindField(iter->second);
    if (field == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot find field referenced by \"" << type <<
            "\" property (" << iter->second << ").";
        return false;
    }

    if ((field->parseKind() != Kind::Int) && (field->parseSemanticType() != SemanticType::Length)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
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

bool ParseListFieldImpl::parseCheckPrefixAsChild(
    const std::string& type,
    const ParseFieldImpl*& extField,
    ParseFieldImplPtr& locField,
    std::string& detachedPrefix)
{
    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), type);
    if (children.empty()) {
        return true;
    }

    if (1U < children.size()) {
        parseLogError() << "There must be only one occurance of \"" << type << "\".";
        return false;
    }

    auto child = children.front();
    auto prefixFields = ParseXmlWrap::parseGetChildren(child);
    if (1U < prefixFields.size()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(child) <<
            "The \"" << type << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto iter = parseProps().find(type);
    bool hasInProps = iter != parseProps().end();
    if (prefixFields.empty()) {
        if (hasInProps) {
            return true;
        }

        parseLogError() << ParseXmlWrap::parseLogPrefix(child) <<
            "The \"" << type << "\" element "
            "is expected to define field as child element";
        return false;
    }

    if (hasInProps) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The \"" << type << "\" element is expected to define only "
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
            "The \"" << type << "\" element  must be of type \"" << common::intStr() << 
            "\" or have semanticType=\"length\" property set.";
        return false;
    }      

    extField = nullptr;
    locField = std::move(field);
    detachedPrefix.clear();
    return true;
}

const ParseFieldImpl* ParseListFieldImpl::parseGetCountPrefixField() const
{
    if (m_state.m_extCountPrefixField != nullptr) {
        assert(!m_countPrefixField);
        return m_state.m_extCountPrefixField;
    }

    assert(m_countPrefixField);
    return m_countPrefixField.get();
}

const ParseFieldImpl* ParseListFieldImpl::parseGetLengthPrefixField() const
{
    if (m_state.m_extLengthPrefixField != nullptr) {
        assert(!m_lengthPrefixField);
        return m_state.m_extLengthPrefixField;
    }

    assert(m_lengthPrefixField);
    return m_lengthPrefixField.get();
}

bool ParseListFieldImpl::parseVerifySiblingsForPrefix(
    const FieldsList& fields, 
    const std::string& detachedName) const
{
    if (detachedName.empty()) {
        return true;
    }

    auto* sibling = parseFindSibling(fields, detachedName);
    if (sibling == nullptr) {
        return false;
    }

    auto fieldKind = parseGetNonRefFieldKind(*sibling);
    if ((fieldKind != Kind::Int) && (sibling->parseSemanticType() != SemanticType::Length)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Detached prefix \"" << detachedName << "\" is expected to be of \"" << common::intStr() << "\" type "
            "or have semanticType=\"length\" property set.";
        return false;
    }

    return true;    
}

} // namespace parse

} // namespace commsdsl
