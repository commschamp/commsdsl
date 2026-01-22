//
// Copyright 2018 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "ParseChecksumLayerImpl.h"
#include "ParseProtocolImpl.h"
#include "parse_common.h"

#include <cassert>

namespace commsdsl
{

namespace parse
{

ParseSyncLayerImpl::ParseSyncLayerImpl(::xmlNodePtr node, ParseProtocolImpl& protocol) :
    Base(node, protocol),
    m_from(&common::parseEmptyString())
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
        common::parseFromStr(),
        common::parseVerifyBeforeReadStr(),
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
        parseUpdateEscFieldInternal() &&
        parseUpdateVerifyBeforeReadInternal() &&
        parseUpdateFromInternal();
}

bool ParseSyncLayerImpl::parseVerifyImpl(const ParseLayersList& layers)
{
    if (!parseProtocol().parseIsSyncSuffixLayerSupported()) {
        return Base::parseVerifyImpl(layers);
    }

    auto thisIdx = parseFindThisLayerIndex(layers);
    assert(thisIdx < layers.size());

    auto payloadIdx = parseFindLayerIndex(layers, ParseKind::Payload);
    if (layers.size() <= payloadIdx) {

        // For some reason no payload is available, cannot verify
        return true;
    }

    m_afterPayload = (payloadIdx < thisIdx);
    if ((!m_afterPayload) && parseFrom().empty()) {
        return true;
    }

    if (!m_afterPayload) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The usage of \"" << common::parseFromStr() << "\" property is allowed only when <sync> layer follows <payload>.";
        return false;
    }

    // After payload
    if (m_seekField && parseFrom().empty()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The usage of \"" << common::parseFromStr() << "\" property is required when \""
                << common::parseSeekFieldStr() << "\" property is set to true for <sync> after <payload>.";
        return false;
    }

    if (parseFrom().empty()) {
        return true;
    }

    auto fromIdx = parseFindLayerIndex(layers, parseFrom());
    if (layers.size() <= fromIdx) {
        parseReportUnexpectedPropertyValue(common::parseFromStr(), parseFrom());
        return false;
    }

    if (payloadIdx < fromIdx) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Layer \"" << parseFrom() << "\" specified by the \"" << common::parseFromStr() << "\" must appear before <payload>.";
        return false;
    }

    return parseVerifySuffixLayersOrder(layers, payloadIdx, thisIdx, fromIdx);
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

bool ParseSyncLayerImpl::parseUpdateVerifyBeforeReadInternal()
{
    auto& prop = common::parseVerifyBeforeReadStr();
    if (!parseValidateSinglePropInstance(prop)) {
        return false;
    }

    auto iter = parseProps().find(prop);
    if (iter == parseProps().end()) {
        return true;
    }

    if (m_seekField) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The properties \"" << prop << "\" and \"" << common::parseSeekFieldStr() << "\" are mutually exclusive.";
        return false;
    }

    if (!parseProtocol().parseIsSyncSuffixLayerSupported()) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Property \"" << prop << "\" of <sync> layer is not supported for selected dslVersion, ignoring...";
        m_from = nullptr;
        return true;
    }

    bool ok = false;
    m_verifyBeforeRead = common::parseStrToBool(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(prop, iter->second);
        return false;
    }

    return true;
}

bool ParseSyncLayerImpl::parseUpdateFromInternal()
{
    auto& prop = common::parseFromStr();
    if (!parseValidateAndUpdateStringPropValue(prop, m_from)) {
        return false;
    }

    if (parseFrom().empty()) {
        return true;
    }

    if ((!m_seekField) && (!m_verifyBeforeRead)) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "The usage of \"" << prop << "\" is allowed only in conjunction with setting \"" <<
            common::parseSeekFieldStr() << "\" or \"" << common::parseVerifyBeforeReadStr() << "\" to true.";
        return false;
    }

    if (!parseProtocol().parseIsSyncSuffixLayerSupported()) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Property \"" << prop << "\" of <sync> layer is not supported for selected dslVersion, ignoring...";
        m_from = nullptr;
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
