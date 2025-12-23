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

#include "ParseSyncLayerImpl.h"

#include "ParseProtocolImpl.h"
#include "parse_common.h"

#include <cassert>

namespace commsdsl
{

namespace parse
{

ParseSyncLayerImpl::ParseSyncLayerImpl(::xmlNodePtr node, ParseProtocolImpl& protocol) :
    Base(node, protocol)
{
}

ParseLayerImpl::ParseKind ParseSyncLayerImpl::parseKindImpl() const
{
    return ParseKind::Sync;
}

const ParseXmlWrap::ParseNamesList& ParseSyncLayerImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::ParseNamesList Names = {
        common::parseSeekFieldStr(),
    };

    return Names;
}

const ParseXmlWrap::ParseNamesList& ParseSyncLayerImpl::parseExtraPossiblePropsNamesImpl() const
{
    static const ParseXmlWrap::ParseNamesList Names = {
        common::parseEscFieldStr(),
    };

    return Names;
}

bool ParseSyncLayerImpl::parseImpl()
{
    return
        parseUpdateSeekFieldInternal() &&
        parseUpdateEscFieldInternal();
}

bool ParseSyncLayerImpl::parseUpdateSeekFieldInternal()
{
    auto& prop = common::parseSeekFieldStr();
    if (!parseValidateSinglePropInstance(prop)) {
        return false;
    }

    auto iter = parseProps().find(prop);
    if (iter == parseProps().end()) {
        return true;
    }

    if (!parseProtocol().parseIsPropertySupported(prop)) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Property \"" << prop << "\" is not supported for selected dslVersion, ignoring...";
        return true;
    }

    bool ok = false;
    m_seekField = common::parseStrToBool(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(prop, iter->second);
        return false;
    }

    return true;
}

bool ParseSyncLayerImpl::parseUpdateEscFieldInternal()
{
    if ((!parseCheckEscFieldFromRefInternal()) ||
        (!parseCheckEscFieldAsChildInternal())) {
        return false;
    }

    if (!parseHasEscField()) {
        return true;
    }

    auto& prop = common::parseEscFieldStr();
    if (!m_seekField) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The usage of \"" << prop << "\" is allowed only in conjunction with setting \"" << common::parseSeekFieldStr() << "\" to true.";
        return false;
    }

    if (!parseProtocol().parseIsPropertySupported(prop)) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Property \"" << prop << "\" is not supported for selected dslVersion, ignoring...";
        m_extEscField = nullptr;
        m_escField.reset();
        return true;
    }

    return true;
}

bool ParseSyncLayerImpl::parseCheckEscFieldFromRefInternal()
{
    auto& prop = common::parseEscFieldStr();
    if (!parseValidateSinglePropInstance(prop)) {
        return false;
    }

    auto iter = parseProps().find(prop);
    if (iter == parseProps().end()) {
        return true;
    }

    auto* field = parseProtocol().parseFindField(iter->second);
    if (field == nullptr) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot find field referenced by \"" << prop <<
            "\" property (" << iter->second << ").";
        return false;
    }

    m_extEscField = field;
    return true;
}

bool ParseSyncLayerImpl::parseCheckEscFieldAsChildInternal()
{
    auto& prop = common::parseEscFieldStr();
    auto children = ParseXmlWrap::parseGetChildren(parseGetNode(), prop);
    if (children.empty()) {
        return true;
    }

    if (1U < children.size()) {
        parseLogError() << "There must be only one occurance of \"" << prop << "\" child element.";
        return false;
    }

    if (m_extEscField != nullptr) {
        parseLogError() << "There must be only one occurance of \"" << prop << "\" definition.";
        return false;
    }

    auto child = children.front();
    auto fields = ParseXmlWrap::parseGetChildren(child);
    if (1U != fields.size()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(child) <<
            "The \"" << prop << "\" element is expected to define only "
            "single field";
        return false;
    }

    ::xmlNodePtr fieldNode = fields.front();
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

    m_escField = std::move(field);
    assert(m_escField->parseExternalRef(false).empty());
    return true;
}

} // namespace parse

} // namespace commsdsl
