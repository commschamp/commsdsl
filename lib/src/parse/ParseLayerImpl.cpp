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

#include "ParseLayerImpl.h"

#include "ParseChecksumLayerImpl.h"
#include "ParseCustomLayerImpl.h"
#include "ParseIdLayerImpl.h"
#include "ParseNamespaceImpl.h"
#include "ParsePayloadLayerImpl.h"
#include "ParseProtocolImpl.h"
#include "ParseSizeLayerImpl.h"
#include "ParseSyncLayerImpl.h"
#include "ParseValueLayerImpl.h"
#include "ParseXmlWrap.h"
#include "parse_common.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <set>

namespace commsdsl
{

namespace parse
{

ParseLayerImpl::ParsePtr ParseLayerImpl::parseCreate(
    const std::string& kind,
    ::xmlNodePtr node,
    ParseProtocolImpl& protocol)
{
    auto& map = parseCreateMap();

    auto iter = map.find(kind);
    if (iter == map.end()) {
        return ParsePtr();
    }

    return iter->second(node, protocol);
}

bool ParseLayerImpl::parse()
{
    m_props = ParseXmlWrap::parseNodeProps(m_node);

    if (!ParseXmlWrap::parseChildrenAsProps(m_node, parseCommonProps(), m_protocol.parseLogger(), m_props)) {
        return false;
    }

    auto& extraPropsNames = parseExtraPropsNamesImpl();
    do {
        if (extraPropsNames.empty()) {
            break;
        }

        if (!ParseXmlWrap::parseChildrenAsProps(m_node, extraPropsNames, m_protocol.parseLogger(), m_props)) {
            return false;
        }

    } while (false);

    if (!ParseXmlWrap::parseChildrenAsProps(m_node, parseCommonPossibleProps(), m_protocol.parseLogger(), m_props, false)) {
        return false;
    }

    bool result =
        parseUpdateName() &&
        parseUpdateDisplayName() &&
        parseUpdateDescription() &&
        parseUpdateField();

    if (!result) {
        return false;
    }

    if (!parseImpl()) {
        return false;
    }

    ParseXmlWrap::ParseNamesList expectedProps = parseCommonProps();
    expectedProps.insert(expectedProps.end(), parseCommonPossibleProps().begin(), parseCommonPossibleProps().end());
    expectedProps.insert(expectedProps.end(), extraPropsNames.begin(), extraPropsNames.end());
    if (!parseUpdateExtraAttrs(expectedProps)) {
        return false;
    }

    ParseXmlWrap::ParseNamesList expectedChildren = parseCommonProps();
    expectedChildren.insert(expectedChildren.end(), parseCommonPossibleProps().begin(), parseCommonPossibleProps().end());
    expectedChildren.insert(expectedChildren.end(), extraPropsNames.begin(), extraPropsNames.end());

    auto supportedFields = ParseFieldImpl::parseSupportedTypes();
    expectedChildren.insert(expectedChildren.end(), supportedFields.begin(), supportedFields.end());
    if (!parseUpdateExtraChildren(expectedChildren)) {
        return false;
    }
    return true;
}

const std::string& ParseLayerImpl::parseName() const
{
    assert(m_name != nullptr);
    return *m_name;
}

const std::string& ParseLayerImpl::parseDisplayName() const
{
    assert(m_displayName != nullptr);
    return *m_displayName;
}

const std::string& ParseLayerImpl::parseDescription() const
{
    assert(m_description != nullptr);
    return *m_description;
}

ParseXmlWrap::ParseNamesList ParseLayerImpl::parseSupportedTypes()
{
    ParseXmlWrap::ParseNamesList result;
    auto& map = parseCreateMap();
    result.reserve(map.size());
    std::transform(
        map.begin(), map.end(), std::back_inserter(result),
        [](auto& elem)
        {
            return elem.first;
        });
    return result;
}

ParseLayerImpl::ParseLayerImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : m_node(node),
    m_protocol(protocol),
    m_name(&common::parseEmptyString()),
    m_displayName(&common::parseEmptyString()),
    m_description(&common::parseEmptyString())
{
}

ParseLogWrapper ParseLayerImpl::parseLogError() const
{
    return commsdsl::parse::parseLogError(m_protocol.parseLogger());
}

ParseLogWrapper ParseLayerImpl::parseLogWarning() const
{
    return commsdsl::parse::parseLogWarning(m_protocol.parseLogger());
}

ParseLogWrapper ParseLayerImpl::parseLogInfo() const
{
    return commsdsl::parse::parseLogInfo(m_protocol.parseLogger());
}

ParseObject::ParseObjKind ParseLayerImpl::parseObjKindImpl() const
{
    return ParseObjKind::Layer;
}

const ParseXmlWrap::ParseNamesList& ParseLayerImpl::parseExtraPropsNamesImpl() const
{
    return ParseXmlWrap::parseEmptyNamesList();
}

bool ParseLayerImpl::parseImpl()
{
    return true;
}

bool ParseLayerImpl::parseVerifyImpl([[maybe_unused]] const ParseLayerImpl::ParseLayersList& layers)
{
    return true;
}

bool ParseLayerImpl::parseMustHaveFieldImpl() const
{
    return true;
}

bool ParseLayerImpl::parseValidateSinglePropInstance(const std::string& str, bool mustHave)
{
    return ParseXmlWrap::parseValidateSinglePropInstance(m_node, m_props, str, parseProtocol().parseLogger(), mustHave);
}

bool ParseLayerImpl::parseValidateAndUpdateStringPropValue(
    const std::string& str,
    const std::string*& valuePtr,
    bool mustHave)
{
    if (!parseValidateSinglePropInstance(str, mustHave)) {
        return false;
    }

    auto iter = m_props.find(str);
    if (iter != m_props.end()) {
        valuePtr = &iter->second;
    }

    assert(iter != m_props.end() || (!mustHave));
    return true;
}

void ParseLayerImpl::parseReportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue)
{
    ParseXmlWrap::parseReportUnexpectedPropertyValue(m_node, parseName(), propName, propValue, parseProtocol().parseLogger());
}

bool ParseLayerImpl::parseVerifySingleLayer(const ParseLayerImpl::ParseLayersList& layers, const std::string& kindStr)
{
    auto k = parseKind();
    for (auto& l : layers) {
        if (l.get() == this) {
            continue;
        }

        if (l->parseKind() == k) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(l->parseGetNode()) <<
                "Only single \"" << kindStr << "\" layer can exist in the frame.";
            return false;
        }
    }
    return true;
}

bool ParseLayerImpl::parseVerifyBeforePayload(const ParseLayerImpl::ParseLayersList& layers)
{
    auto thisIdx = parseFindThisLayerIndex(layers);
    auto payloadIdx = parseFindLayerIndex(layers, ParseKind::Payload);
    assert(thisIdx < layers.size());
    assert(payloadIdx < layers.size());

    if (payloadIdx <= thisIdx) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "This layer is expected to be before the \"" << common::parsePayloadStr() <<
            "\" one.";
        return false;
    }

    return true;
}

std::size_t ParseLayerImpl::parseFindThisLayerIndex(const ParseLayerImpl::ParseLayersList& layers) const
{
    auto iter =
        std::find_if(
            layers.begin(), layers.end(),
            [this](auto& l)
            {
                return this == l.get();
            });

    if (iter == layers.end()) {
        return std::numeric_limits<std::size_t>::max();
    }

    return static_cast<std::size_t>(std::distance(layers.begin(), iter));
}

std::size_t ParseLayerImpl::parseFindLayerIndex(
    const ParseLayerImpl::ParseLayersList& layers,
    ParseLayerImpl::ParseKind lKind)
{
    auto iter =
        std::find_if(
            layers.begin(), layers.end(),
            [lKind](auto& l)
            {
                return l->parseKind() == lKind;
            });

    if (iter == layers.end()) {
        return std::numeric_limits<std::size_t>::max();
    }

    return static_cast<std::size_t>(std::distance(layers.begin(), iter));
}

std::size_t ParseLayerImpl::parseFindLayerIndex(
    const ParseLayerImpl::ParseLayersList& layers,
    const std::string& name)
{
    auto iter =
        std::find_if(
            layers.begin(), layers.end(),
            [&name](auto& l)
            {
                return l->parseName() == name;
            });

    if (iter == layers.end()) {
        return std::numeric_limits<std::size_t>::max();
    }

    return static_cast<std::size_t>(std::distance(layers.begin(), iter));
}

const ParseXmlWrap::ParseNamesList& ParseLayerImpl::parseCommonProps()
{
    static const ParseXmlWrap::ParseNamesList CommonNames = {
        common::parseNameStr(),
        common::parseDescriptionStr(),
        common::parseDisplayNameStr(),
    };

    return CommonNames;
}

const ParseXmlWrap::ParseNamesList&ParseLayerImpl::parseCommonPossibleProps()
{
    static const ParseXmlWrap::ParseNamesList CommonNames = {
        common::parseFieldStr()
    };

    return CommonNames;
}

bool ParseLayerImpl::parseUpdateName()
{
    bool mustHave = m_name->empty();
    if (!parseValidateAndUpdateStringPropValue(common::parseNameStr(), m_name, mustHave)) {
        return false;
    }

    if (!common::parseIsValidName(*m_name)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                      "Invalid value for name property \"" << m_name << "\".";
        return false;
    }

    return true;
}

bool ParseLayerImpl::parseUpdateDisplayName()
{
    auto& propName = common::parseDisplayNameStr();
    if (!parseValidateAndUpdateStringPropValue(propName, m_displayName)) {
        return false;
    }

    if ((!parseDisplayName().empty()) && (!m_protocol.parseIsLayerDisplayNameSupported())) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The property \"" << propName << "\" of layer is not supported for dslVersion=" <<
                m_protocol.parseCurrSchema().parseDslVersion() << ".";
        m_displayName = &common::parseEmptyString();
    }

    return true;
}

bool ParseLayerImpl::parseUpdateDescription()
{
    return parseValidateAndUpdateStringPropValue(common::parseDescriptionStr(), m_description);
}

bool ParseLayerImpl::parseUpdateField()
{
    if ((!parseCheckFieldFromRef()) ||
        (!parseCheckFieldAsChild())) {
        return false;
    }

    if (parseHasField() == parseMustHaveFieldImpl()) {
        return true;
    }

    if (parseHasField()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "This layer mustn't specify field.";
        return false;
    }

    parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
        "This layer must specify field.";

    return false;
}

bool ParseLayerImpl::parseUpdateExtraAttrs(const ParseXmlWrap::ParseNamesList& names)
{
    auto extraAttrs = ParseXmlWrap::parseGetExtraAttributes(m_node, names, m_protocol);
    if (extraAttrs.empty()) {
        return true;
    }

    if (m_extraAttrs.empty()) {
        m_extraAttrs = std::move(extraAttrs);
        return true;
    }

    std::move(extraAttrs.begin(), extraAttrs.end(), std::inserter(m_extraAttrs, m_extraAttrs.end()));
    return true;
}

bool ParseLayerImpl::parseUpdateExtraChildren(const ParseXmlWrap::ParseNamesList& names)
{
    auto extraChildren = ParseXmlWrap::parseGetExtraChildren(m_node, names, m_protocol);
    if (extraChildren.empty()) {
        return true;
    }

    if (m_extraChildren.empty()) {
        m_extraChildren = std::move(extraChildren);
        return true;
    }

    m_extraChildren.reserve(m_extraChildren.size() + extraChildren.size());
    std::move(extraChildren.begin(), extraChildren.end(), std::back_inserter(m_extraChildren));
    return true;
}

bool ParseLayerImpl::parseCheckFieldFromRef()
{
    if (!parseValidateSinglePropInstance(common::parseFieldStr())) {
        return false;
    }

    auto iter = parseProps().find(common::parseFieldStr());
    if (iter == parseProps().end()) {
        return true;
    }

    auto* field = parseProtocol().parseFindField(iter->second);
    if (field == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot find field referenced by \"" << common::parseFieldStr() <<
            "\" property (" << iter->second << ").";
        return false;
    }

    m_extField = field;
    assert(!m_field);
    return true;
}

bool ParseLayerImpl::parseCheckFieldAsChild()
{
    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), common::parseFieldStr());
    if (1U < children.size()) {
        parseLogError() << "There must be only one occurance of \"" << common::parseFieldStr() << "\" child element.";
        return false;
    }

    auto fieldTypes = ParseXmlWrap::parseGetChildren(parseGetNode(), ParseFieldImpl::parseSupportedTypes());
    if ((0U < children.size()) && (0U < fieldTypes.size())) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                  "The frame layer element does not support "
                  "stand alone field as child element together with \"" <<
                  common::parseFieldStr() << "\" child element.";
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
                "The frame layer element is expected to define only "
                "single field";
            return false;
        }

        auto allChildren = ParseXmlWrap::parseGetChildren(parseGetNode());
        if (allChildren.size() != fieldTypes.size()) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                  "The field type of frame layer "
                  " must be defined inside \"<" << common::parseFieldsStr() << ">\" child element "
                  "when there are other property describing children.";
            return false;
        }

        if (parseProps().find(common::parseFieldStr()) != parseProps().end()) {
            parseLogError() << "There must be only one occurance of \"" << common::parseFieldStr() << "\" definition.";
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
                "The \"" << common::parseFieldStr() << "\" element is expected to define only "
                "single field";
            return false;
        }

        if (parseProps().find(common::parseFieldStr()) == parseProps().end()) {
            fieldNode = fields.front();
            break;
        }

        auto attrs = ParseXmlWrap::parseNodeProps(parseGetNode());
        if (attrs.find(common::parseFieldsStr()) != attrs.end()) {
            parseLogError() << "There must be only one occurance of \"" << common::parseFieldStr() << "\" definition.";
            return false;
        }

        // The field element is parsed as property
        return true;
    } while (false);

    assert (fieldNode != nullptr);

    std::string fieldKind(reinterpret_cast<const char*>(fieldNode->name));
    auto field = ParseFieldImpl::parseCreate(fieldKind, fieldNode, parseProtocol());
    if (!field) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Unknown field type \"" << fieldKind << "\"";
        return false;
    }

    field->parseSetParent(this);
    if (!field->parse()) {
        return false;
    }

    m_extField = nullptr;
    m_field = std::move(field);
    assert(m_field->parseExternalRef(false).empty());
    return true;
}

const ParseLayerImpl::ParseCreateMap& ParseLayerImpl::parseCreateMap()
{
    static const ParseCreateMap Map = {
        std::make_pair(
            common::parsePayloadStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return ParsePtr(new ParsePayloadLayerImpl(n, p));
            }),
        std::make_pair(
            common::parseIdStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return ParsePtr(new ParseIdLayerImpl(n, p));
            }),
        std::make_pair(
            common::parseSizeStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return ParsePtr(new ParseSizeLayerImpl(n, p));
            }),
        std::make_pair(
            common::parseSyncStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return ParsePtr(new ParseSyncLayerImpl(n, p));
            }),
        std::make_pair(
            common::parseChecksumStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return ParsePtr(new ParseChecksumLayerImpl(n, p));
            }),
        std::make_pair(
            common::parseValueStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return ParsePtr(new ParseValueLayerImpl(n, p));
            }),
        std::make_pair(
            common::parseCustomStr(),
            [](::xmlNodePtr n, ParseProtocolImpl& p)
            {
                return ParsePtr(new ParseCustomLayerImpl(n, p));
            }),
    };

    return Map;
}

} // namespace parse

} // namespace commsdsl
