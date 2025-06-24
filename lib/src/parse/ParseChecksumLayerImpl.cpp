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

#include "ParseChecksumLayerImpl.h"

#include <cassert>
#include <map>

#include "ParseProtocolImpl.h"
#include "parse_common.h"

namespace commsdsl
{

namespace parse
{

ParseChecksumLayerImpl::ParseChecksumLayerImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol),
    m_algName(&common::emptyString()),
    m_from(&common::emptyString()),
    m_until(&common::emptyString())
{
    assert(parseFrom().empty());
    assert(parseUntil().empty());
}

ParseLayerImpl::Kind ParseChecksumLayerImpl::parseKindImpl() const
{
    return Kind::Checksum;
}

const ParseXmlWrap::NamesList& ParseChecksumLayerImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::NamesList Names = {
        common::algStr(),
        common::algNameStr(),
        common::fromStr(),
        common::untilStr(),
        common::verifyBeforeReadStr()
    };
    return Names;
}

bool ParseChecksumLayerImpl::parseImpl()
{
    return
        parseUpdateAlg() &&
        parseUpdateFrom() &&
        parseUpdateUntil() &&
        parseUpdateVerifyBeforeRead();
}

bool ParseChecksumLayerImpl::parseVerifyImpl(const ParseLayerImpl::LayersList& layers)
{
    auto thisIdx = parseFindThisLayerIndex(layers);
    assert(thisIdx < layers.size());

    if (parseFrom().empty() && parseUntil().empty()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) << 
            "Checksum layer must set \"" << common::fromStr() << "\" or \"" << 
            common::untilStr() << "\" property to indicate on what values checksum is calculated.";
        return false;
    }

    if (!parseFrom().empty()) {
        auto fromIdx = parseFindLayerIndex(layers, parseFrom());
        if (layers.size() <= fromIdx) {
            parseReportUnexpectedPropertyValue(common::fromStr(), parseFrom());
            return false;
        }

        if (thisIdx <= fromIdx) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Layer \"" << parseFrom() << "\" must appear before the \"" << parseName() << "\".";
            return false;
        }
    }

    if (!parseUntil().empty()) {
        auto untilIdx = parseFindLayerIndex(layers, parseUntil());
        if (layers.size() <= untilIdx) {
            parseReportUnexpectedPropertyValue(common::untilStr(), parseUntil());
            return false;
        }

        if (untilIdx <= thisIdx) {
            parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
                "Layer \"" << parseUntil() << "\" must appear after the \"" << parseName() << "\".";
            return false;
        }
    }

    return true;
}

bool ParseChecksumLayerImpl::parseUpdateAlg()
{
    if (!parseValidateSinglePropInstance(common::algStr(), true)) {
        return false;
    }

    auto iter = parseProps().find(common::algStr());
    assert(iter != parseProps().end());
    auto algStr = common::toLowerCopy(iter->second);

    static const std::map<std::string, Alg> Map = {
        std::make_pair("custom", Alg::Custom),
        std::make_pair("sum", Alg::Sum),
        std::make_pair("crc-ccitt", Alg::Crc_CCITT),
        std::make_pair("crc_ccitt", Alg::Crc_CCITT),
        std::make_pair("crc-16", Alg::Crc_16),
        std::make_pair("crc_16", Alg::Crc_16),
        std::make_pair("crc-32", Alg::Crc_32),
        std::make_pair("crc_32", Alg::Crc_32),
        std::make_pair("xor", Alg::Xor),
    };

    auto algIter = Map.find(algStr);
    if (algIter == Map.end()) {
        parseReportUnexpectedPropertyValue(common::algStr(), iter->second);
        return false;
    }

    m_alg = algIter->second;
    if (m_alg != Alg::Custom) {
        return true;
    }

    if (!parseValidateSinglePropInstance(common::algNameStr(), true)) {
        return false;
    }

    auto algNameIter = parseProps().find(common::algNameStr());
    m_algName = &algNameIter->second;

    if (!common::isValidName(*m_algName)) {
        parseReportUnexpectedPropertyValue(common::algNameStr(), iter->second);
        return false;
    }

    return true;
}

bool ParseChecksumLayerImpl::parseUpdateFrom()
{
    return parseValidateAndUpdateStringPropValue(common::fromStr(), m_from);
}

bool ParseChecksumLayerImpl::parseUpdateUntil()
{
    return parseValidateAndUpdateStringPropValue(common::untilStr(), m_until);
}

bool ParseChecksumLayerImpl::parseUpdateVerifyBeforeRead()
{
    if (!parseValidateSinglePropInstance(common::verifyBeforeReadStr())) {
        return false;
    }

    auto iter = parseProps().find(common::verifyBeforeReadStr());
    if (iter == parseProps().end()) {
        return true;
    }

    bool ok = false;
    m_verifyBeforeRead = common::parseStrToBool(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::verifyBeforeReadStr(), iter->second);
        return false;
    }
    return true;
}


} // namespace parse

} // namespace commsdsl
