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

ParseLayerImpl::Kind ParseCustomLayerImpl::parseKindImpl() const
{
    return Kind::Custom;
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

bool ParseCustomLayerImpl::parseVerifyImpl(const LayersList& layers)
{
    return parseVerifyChecksumInternal(layers);
}

const ParseXmlWrap::NamesList& ParseCustomLayerImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList List = {
        common::idReplacementStr(),
        common::semanticLayerTypeStr(),
        common::checksumFromStr(),
        common::checksumUntilStr()
    };

    return List;
}

bool ParseCustomLayerImpl::parseUpdateIdReplacement()
{
    auto& prop = common::idReplacementStr();
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
        m_sematicLayerType = Kind::Id;
    }

    return true;
}

bool ParseCustomLayerImpl::parseUpdateSemanticLayerType()
{
    auto& prop = common::semanticLayerTypeStr();
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

    if (m_sematicLayerType != Kind::Custom) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Cannot use \"" + prop + "\" property when semantic type was specified by other (deprecated) properties";        

        return false;
    }

    static const std::map<std::string, Kind> Map = {
        {common::payloadStr(), Kind::Payload},
        {common::idStr(), Kind::Id},
        {common::sizeStr(), Kind::Size},
        {common::syncStr(), Kind::Sync},
        {common::checksumStr(), Kind::Checksum},
        {common::valueStr(), Kind::Value},
        {common::customStr(), Kind::Custom},
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
    auto& prop = common::checksumFromStr();
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

    if (m_sematicLayerType != Kind::Checksum) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Property \"" << prop << "\" is not applicable to selected \"" << common::semanticLayerTypeStr() << "\", ignoring...";
        return true;        
    }    

    m_checksumFromLayer = iter->second;
    return true;
}

bool ParseCustomLayerImpl::parseUpdateChecksumUntil()
{
    auto& prop = common::checksumUntilStr();
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

    if (m_sematicLayerType != Kind::Checksum) {
        parseLogWarning() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Property \"" << prop << "\" is not applicable to selected \"" << common::semanticLayerTypeStr() << "\", ignoring...";
        return true;        
    }

    m_checksumUntilLayer = iter->second;
    return true;
}

bool ParseCustomLayerImpl::parseVerifyChecksumInternal(const LayersList& layers) 
{
    if (m_sematicLayerType != Kind::Checksum) {
        return true;
    }

    auto thisIdx = parseFindThisLayerIndex(layers);
    assert(thisIdx < layers.size());

    if (m_checksumFromLayer.empty() && m_checksumUntilLayer.empty()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) << 
            "Custom layer with " + common::semanticLayerTypeStr() << "=\"" << common::checksumStr() << "\" must set \"" << 
            common::checksumFromStr() << "\" or \"" << 
            common::checksumUntilStr() << "\" property to indicate on what values checksum is calculated.";
        return false;
    }

    if (!m_checksumFromLayer.empty()) {
        auto fromIdx = parseFindLayerIndex(layers, m_checksumFromLayer);
        if (layers.size() <= fromIdx) {
            parseReportUnexpectedPropertyValue(common::checksumFromStr(), m_checksumFromLayer);
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
            parseReportUnexpectedPropertyValue(common::checksumUntilStr(), m_checksumUntilLayer);
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
