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

#include "ParseCustomLayerImpl.h"

#include "ParseProtocolImpl.h"
#include "parse_common.h"

#include <cassert>
#include <map>

namespace commsdsl
{

namespace parse
{

ParseCustomLayerImpl::ParseCustomLayerImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol)
{
}

ParseLayerImpl::ParseKind ParseCustomLayerImpl::parseKindImpl() const
{
    return ParseKind::Custom;
}

bool ParseCustomLayerImpl::parseImpl()
{
    return 
        Base::parseImpl() &&
        parseUpdateIdReplacement() &&
        parseUpdateSemanticLayerType() &&
        parseUpdateChecksumFrom() &&
        parseUpdateChecksumUntil();
}

bool ParseCustomLayerImpl::parseVerifyImpl(const ParseLayersList& layers)
{
    return parseVerifyChecksumInternal(layers);
}

const ParseXmlWrap::ParseNamesList& ParseCustomLayerImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::ParseNamesList List = {
        common::parseIdReplacementStr(),
        common::parseSemanticLayerTypeStr(),
        common::parseChecksumFromStr(),
        common::parseChecksumUntilStr()
    };

    return List;
}

bool ParseCustomLayerImpl::parseUpdateIdReplacement()
{
    auto& prop = common::parseIdReplacementStr();
    if (!parseValidateSinglePropInstance(prop)) {
        return false;
    }

    auto iter = parseProps().find(prop);
    if (iter == parseProps().end()) {
        return true;
    }

    bool ok = false;
    bool idReplacement = common::parseStrToBool(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(prop, iter->second);
        return false;
    }

    if (idReplacement) {
        m_sematicLayerType = ParseKind::Id;
    }

    return true;
}

bool ParseCustomLayerImpl::parseUpdateSemanticLayerType()
{
    auto& prop = common::parseSemanticLayerTypeStr();
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

    if (iter->second.empty()) {
        return true;
    }

    if (m_sematicLayerType != ParseKind::Custom) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot use \"" + prop + "\" property when semantic type was specified by other (deprecated) properties";        

        return false;
    }

    static const std::map<std::string, ParseKind> Map = {
        {common::parsePayloadStr(), ParseKind::Payload},
        {common::parseIdStr(), ParseKind::Id},
        {common::parseSizeStr(), ParseKind::Size},
        {common::parseSyncStr(), ParseKind::Sync},
        {common::parseChecksumStr(), ParseKind::Checksum},
        {common::parseValueStr(), ParseKind::Value},
        {common::parseCustomStr(), ParseKind::Custom},
    };

    auto kindIter = Map.find(iter->second);
    if (kindIter == Map.end()) {
        parseReportUnexpectedPropertyValue(prop, iter->second);
        return false;
    }

    m_sematicLayerType = kindIter->second;
    return true;
}

bool ParseCustomLayerImpl::parseUpdateChecksumFrom()
{
    auto& prop = common::parseChecksumFromStr();
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

    if (m_sematicLayerType != ParseKind::Checksum) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Property \"" << prop << "\" is not applicable to selected \"" << common::parseSemanticLayerTypeStr() << "\", ignoring...";
        return true;        
    }    

    m_checksumFromLayer = iter->second;
    return true;
}

bool ParseCustomLayerImpl::parseUpdateChecksumUntil()
{
    auto& prop = common::parseChecksumUntilStr();
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

    if (m_sematicLayerType != ParseKind::Checksum) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Property \"" << prop << "\" is not applicable to selected \"" << common::parseSemanticLayerTypeStr() << "\", ignoring...";
        return true;        
    }

    m_checksumUntilLayer = iter->second;
    return true;
}

bool ParseCustomLayerImpl::parseVerifyChecksumInternal(const ParseLayersList& layers) 
{
    if (m_sematicLayerType != ParseKind::Checksum) {
        return true;
    }

    auto thisIdx = parseFindThisLayerIndex(layers);
    assert(thisIdx < layers.size());

    if (m_checksumFromLayer.empty() && m_checksumUntilLayer.empty()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) << 
            "Custom layer with " + common::parseSemanticLayerTypeStr() << "=\"" << common::parseChecksumStr() << "\" must set \"" << 
            common::parseChecksumFromStr() << "\" or \"" << 
            common::parseChecksumUntilStr() << "\" property to indicate on what values checksum is calculated.";
        return false;
    }

    if (!m_checksumFromLayer.empty()) {
        auto fromIdx = parseFindLayerIndex(layers, m_checksumFromLayer);
        if (layers.size() <= fromIdx) {
            parseReportUnexpectedPropertyValue(common::parseChecksumFromStr(), m_checksumFromLayer);
            return false;
        }

        if (thisIdx <= fromIdx) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Layer \"" << m_checksumFromLayer << "\" must appear before the \"" << parseName() << "\".";
            return false;
        }
    }

    if (!m_checksumUntilLayer.empty()) {
        auto untilIdx = parseFindLayerIndex(layers, m_checksumUntilLayer);
        if (layers.size() <= untilIdx) {
            parseReportUnexpectedPropertyValue(common::parseChecksumUntilStr(), m_checksumUntilLayer);
            return false;
        }

        if (untilIdx <= thisIdx) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Layer \"" << m_checksumUntilLayer << "\" must appear after the \"" << parseName() << "\".";
            return false;
        }
    }

    return true;
}

} // namespace parse

} // namespace commsdsl
