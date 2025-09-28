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

#include "ParseIntFieldImpl.h"
#include "ParseProtocolImpl.h"
#include "parse_common.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <type_traits>

namespace commsdsl
{

namespace parse
{

namespace
{

const ParseXmlWrap::ParseNamesList& parseListSupportedTypes()
{
    static const ParseXmlWrap::ParseNamesList Names = ParseFieldImpl::parseSupportedTypes();
    return Names;
}

ParseXmlWrap::ParseNamesList parseGetExtraNames()
{
    auto names = parseListSupportedTypes();
    names.push_back(common::parseElementStr());
    names.push_back(common::parseCountPrefixStr());
    names.push_back(common::parseLengthPrefixStr());
    names.push_back(common::parseElemLengthPrefixStr());
    names.push_back(common::parseTermSuffixStr());
    return names;
}

} // namespace

ParseListFieldImpl::ParseListFieldImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol)
{
}

ParseFieldImpl::ParseKind ParseListFieldImpl::parseKindImpl() const
{
    return ParseKind::List;
}

ParseListFieldImpl::ParseListFieldImpl(const ParseListFieldImpl& other)
  : Base(other),
    m_state(other.m_state)
{
    parseCloneFields(other);
}

ParseFieldImpl::ParsePtr ParseListFieldImpl::parseCloneImpl() const
{
    return ParsePtr(new ParseListFieldImpl(*this));
}

const ParseXmlWrap::ParseNamesList& ParseListFieldImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::ParseNamesList List = {
        common::parseCountStr(),
        common::parseElemFixedLengthStr()
    };

    return List;
}

const ParseXmlWrap::ParseNamesList& ParseListFieldImpl::parseExtraPossiblePropsNamesImpl() const
{
    static const ParseXmlWrap::ParseNamesList List = {
        common::parseElementStr(),
        common::parseCountPrefixStr(),
        common::parseLengthPrefixStr(),
        common::parseElemLengthPrefixStr(),
        common::parseTermSuffixStr(),
    };
    return List;
}

const ParseXmlWrap::ParseNamesList& ParseListFieldImpl::parseExtraChildrenNamesImpl() const
{
    static const ParseXmlWrap::ParseNamesList List = parseGetExtraNames();
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

bool ParseListFieldImpl::parseVerifySiblingsImpl(const ParseFieldsList& fields) const
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
        common::parseAddToLength(parseElemLengthPrefixField().parseMaxLength(), extraLen);
    }

    assert(parseElementField().parseValid());
    auto elemMaxLength = parseElementField().parseMaxLength();
    if (m_state.m_count != 0U) {

        if (!m_state.m_elemFixedLength) {
            extraLen = common::parseMulLength(extraLen, m_state.m_count);
        }

        common::parseAddToLength(common::parseMulLength(elemMaxLength, m_state.m_count), extraLen);
        return extraLen;
    }

    if (parseHasCountPrefixField()) {
        auto* prefixField = parseGetCountPrefixField();
        assert(prefixField->parseKind() == ParseField::ParseKind::Int);
        auto& castedPrefix = static_cast<const ParseIntFieldImpl&>(*prefixField);
        std::size_t count = static_cast<std::size_t>(castedPrefix.parseMaxValue());
        auto result = common::parseMulLength(elemMaxLength, count);

        common::parseAddToLength(castedPrefix.parseMaxLength(), result);

        if (!m_state.m_elemFixedLength) {
            extraLen = common::parseMulLength(extraLen, count);
        }

        common::parseAddToLength(extraLen, result);
        return result;
    }

    do {
        if (!parseHasLengthPrefixField()) {
            break;
        }

        auto* prefixField = parseGetLengthPrefixField();
        if (prefixField->parseKind() != ParseField::ParseKind::Int) {
            break;
        }

        auto& castedPrefix = static_cast<const ParseIntFieldImpl&>(*prefixField);
        auto result = castedPrefix.parseMaxLength();
        common::parseAddToLength(static_cast<std::size_t>(castedPrefix.parseMaxValue()), result);
        return result;
    } while (false);

    return common::parseMaxPossibleLength();
}

bool ParseListFieldImpl::parseIsValidRefTypeImpl(ParseFieldRefType type) const
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
    if (!parseValidateSinglePropInstance(common::parseCountStr())) {
        return false;
    }

    auto iter = parseProps().find(common::parseCountStr());
    if (iter == parseProps().end()) {
        return true;
    }

    bool ok = false;
    auto newVal = common::parseStrToUnsigned(iter->second, &ok);
    if ((!ok) || (newVal == 0U)) {
        parseReportUnexpectedPropertyValue(common::parseCountStr(), iter->second);
        return false;
    }

    if ((parseHasCountPrefixField()) || (parseHasLengthPrefixField()) || (parseHasTermSuffixField()) ||
        (!m_state.m_detachedCountPrefixField.empty()) ||
        (!m_state.m_detachedLengthPrefixField.empty()) ||
        (!m_state.m_detachedElemLengthPrefixField.empty()) ||
        (!m_state.m_detachedTermSuffixField.empty())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot use " << common::parseCountStr() << " property after reusing list with " <<
            common::parseCountPrefixStr() << ", " << common::parseLengthPrefixStr() << ", or" << common::parseTermSuffixStr() << ".";
        return false;
    }

    m_state.m_count = newVal;
    return true;
}

bool ParseListFieldImpl::parseUpdateCountPrefix()
{
    if ((!parseCheckPrefixFromRef(common::parseCountPrefixStr(), m_state.m_extCountPrefixField, m_countPrefixField, m_state.m_detachedCountPrefixField)) ||
        (!parseCheckPrefixAsChild(common::parseCountPrefixStr(), m_state.m_extCountPrefixField, m_countPrefixField, m_state.m_detachedCountPrefixField))) {
        return false;
    }

    if ((!parseHasCountPrefixField()) && (m_state.m_detachedCountPrefixField.empty())) {
        return true;
    }

    if (m_state.m_count != 0U) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::parseCountStr() << " and " << common::parseCountPrefixStr() << " cannot be used together.";
        return false;
    }

    if (parseHasLengthPrefixField() || (!m_state.m_detachedLengthPrefixField.empty())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::parseLengthPrefixStr() << " and " << common::parseCountPrefixStr() << " cannot be used together.";
        return false;
    }

    if (parseHasTermSuffixField() || (!m_state.m_detachedTermSuffixField.empty())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::parseTermSuffixStr() << " and " << common::parseCountPrefixStr() << " cannot be used together.";
        return false;
    }

    return true;
}

bool ParseListFieldImpl::parseUpdateLengthPrefix()
{
    if ((!parseCheckPrefixFromRef(common::parseLengthPrefixStr(), m_state.m_extLengthPrefixField, m_lengthPrefixField, m_state.m_detachedLengthPrefixField)) ||
        (!parseCheckPrefixAsChild(common::parseLengthPrefixStr(), m_state.m_extLengthPrefixField, m_lengthPrefixField, m_state.m_detachedLengthPrefixField))) {
        return false;
    }

    if ((!parseHasLengthPrefixField()) && (m_state.m_detachedLengthPrefixField.empty())) {
        return true;
    }

    if (m_state.m_count != 0U) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::parseCountStr() << " and " << common::parseLengthPrefixStr() << " cannot be used together.";
        return false;
    }

    if (parseHasCountPrefixField() || (!m_state.m_detachedCountPrefixField.empty())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::parseCountPrefixStr() << " and " << common::parseLengthPrefixStr() << " cannot be used together.";
        return false;
    }

    if (parseHasTermSuffixField() || (!m_state.m_detachedTermSuffixField.empty())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::parseTermSuffixStr() << " and " << common::parseLengthPrefixStr() << " cannot be used together.";
        return false;
    }

    return true;
}

bool ParseListFieldImpl::parseUpdateElemLengthPrefix()
{
    if ((!parseCheckPrefixFromRef(common::parseElemLengthPrefixStr(), m_state.m_extElemLengthPrefixField, m_elemLengthPrefixField, m_state.m_detachedElemLengthPrefixField)) ||
        (!parseCheckPrefixAsChild(common::parseElemLengthPrefixStr(), m_state.m_extElemLengthPrefixField, m_elemLengthPrefixField, m_state.m_detachedElemLengthPrefixField))) {
        return false;
    }

    if ((!m_state.m_detachedElemLengthPrefixField.empty()) &&
        (!m_state.m_elemFixedLength)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Detached element length prefix is supported only for lists with fixed length elements. "
            "Set the \"" << common::parseElemFixedLengthStr() << "\" property.";
        return false;
    }

    return true;
}

bool ParseListFieldImpl::parseUpdateElemFixedLength()
{
    if (!parseValidateSinglePropInstance(common::parseElemFixedLengthStr())) {
        return false;
    }

    do {
        auto iter = parseProps().find(common::parseElemFixedLengthStr());
        if (iter == parseProps().end()) {
            break;
        }

        bool ok = false;
        m_state.m_elemFixedLength = common::parseStrToBool(iter->second, &ok);
        if (!ok) {
            parseReportUnexpectedPropertyValue(common::parseElemFixedLengthStr(), iter->second);
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
            "Cannot have \"" << common::parseElemFixedLengthStr() << "\" property being set to "
            "true when list element has variable length.";
        return false;
    }
    return true;
}

bool ParseListFieldImpl::parseUpdateTermSuffix()
{
    auto& prop = common::parseTermSuffixStr();
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
            common::parseTermSuffixStr() << " and " << common::parseCountStr() << " cannot be used together.";
        return false;
    }

    if (parseHasCountPrefixField() || (!m_state.m_detachedCountPrefixField.empty())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::parseTermSuffixStr() << " and " << common::parseCountPrefixStr() << " cannot be used together.";
        return false;
    }

    if (parseHasLengthPrefixField() || (!m_state.m_detachedLengthPrefixField.empty())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            common::parseTermSuffixStr() << " and " << common::parseLengthPrefixStr() << " cannot be used together.";
        return false;
    }

    return true;
}

bool ParseListFieldImpl::parseCheckElementFromRef()
{
    if (!parseValidateSinglePropInstance(common::parseElementStr())) {
        return false;
    }

    auto iter = parseProps().find(common::parseElementStr());
    if (iter == parseProps().end()) {
        return true;
    }

    auto* field = parseProtocol().parseFindField(iter->second);
    if (field == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot find field referenced by \"" << common::parseElementStr() <<
            "\" property (" << iter->second << ").";
        return false;
    }

    m_elementField.reset();
    m_state.m_extElementField = field;
    return true;
}

bool ParseListFieldImpl::parseCheckElementAsChild()
{
    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), common::parseElementStr());
    // if (children.empty()) {
    //     return true;
    // }

    if (1U < children.size()) {
        parseLogError() << "There must be only one occurance of \"" << common::parseElementStr() << "\".";
        return false;
    }

    auto fieldTypes = ParseXmlWrap::parseGetChildren(parseGetNode(), parseListSupportedTypes());
    if ((0U < children.size()) && (0U < fieldTypes.size())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                  "The \"" << common::parseListStr() << "\" does not support "
                  "stand alone field as child element together with \"" <<
                  common::parseElementStr() << "\" child element.";
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
                "The \"" << common::parseListStr() << "\" element is expected to define only "
                "single child field";
            return false;
        }

        auto allChildren = ParseXmlWrap::parseGetChildren(parseGetNode());
        if (allChildren.size() != fieldTypes.size()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                  "The element type of \"" << common::parseListStr() <<
                  "\" must be defined inside \"<" << common::parseElementStr() << ">\" child element "
                  "when there are other property describing children.";
            return false;
        }

        if (parseProps().find(common::parseElementStr()) != parseProps().end()) {
            parseLogError() << "There must be only one occurance of \"" << common::parseElementStr() << "\" definition.";
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
                "The \"" << common::parseElementStr() << "\" element is expected to define only "
                "single field";
            return false;
        }

        if (parseProps().find(common::parseElementStr()) == parseProps().end()) {
            if (!fields.empty()) {
                fieldNode = fields.front();
                break;
            }

            parseLogError() << ParseXmlWrap::parseLogPrefix(child) <<
                "The \"" << common::parseElementStr() << "\" node "
                "is expected to define field as child element";
            return false;
        }

        auto attrs = ParseXmlWrap::parseNodeProps(parseGetNode());
        if (attrs.find(common::parseFieldsStr()) != attrs.end()) {
            parseLogError() << "There must be only one occurance of \"" << common::parseElementStr() << "\" definition.";
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

    if (str[0] == common::parseSiblingRefPrefix()) {
        if (!parseCheckDetachedPrefixAllowed()) {
            return false;
        }

        detachedPrefix = std::string(str, 1);
        common::parseNormaliseString(detachedPrefix);

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

    if ((field->parseKind() != ParseKind::Int) && (field->parseSemanticType() != ParseSemanticType::Length)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The field referenced by \"" << type <<
            "\" property (" << iter->second << ") must be of type \"" << common::parseIntStr() <<
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

    if ((field->parseKind() != ParseKind::Int) && (field->parseSemanticType() != ParseSemanticType::Length)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The \"" << type << "\" element  must be of type \"" << common::parseIntStr() <<
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
    const ParseFieldsList& fields,
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
    if ((fieldKind != ParseKind::Int) && (sibling->parseSemanticType() != ParseSemanticType::Length)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Detached prefix \"" << detachedName << "\" is expected to be of \"" << common::parseIntStr() << "\" type "
            "or have semanticType=\"length\" property set.";
        return false;
    }

    return true;
}

} // namespace parse

} // namespace commsdsl
