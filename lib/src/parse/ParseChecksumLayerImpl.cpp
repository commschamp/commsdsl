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

#include "ParseProtocolImpl.h"
#include "parse_common.h"

#include <cassert>
#include <map>

namespace commsdsl
{

namespace parse
{

ParseChecksumLayerImpl::ParseChecksumLayerImpl(::xmlNodePtr node, ParseProtocolImpl& protocol)
  : Base(node, protocol),
    m_algName(&common::parseEmptyString()),
    m_from(&common::parseEmptyString()),
    m_until(&common::parseEmptyString())
{
    assert(parseFrom().empty());
    assert(parseUntil().empty());
}

ParseLayerImpl::ParseKind ParseChecksumLayerImpl::parseKindImpl() const
{
    return ParseKind::Checksum;
}

const ParseXmlWrap::ParseNamesList& ParseChecksumLayerImpl::parseExtraPropsNamesImpl() const
{
    static const ParseXmlWrap::ParseNamesList Names = {
        common::parseAlgStr(),
        common::parseAlgNameStr(),
        common::parseFromStr(),
        common::parseUntilStr(),
        common::parseVerifyBeforeReadStr()
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

bool ParseChecksumLayerImpl::parseVerifyImpl(const ParseLayerImpl::ParseLayersList& layers)
{
    auto thisIdx = parseFindThisLayerIndex(layers);
    assert(thisIdx < layers.size());

    if (parseFrom().empty() && parseUntil().empty()) {
        parseLogError() << ParseXmlWrap::parseLogPrefix(parseGetNode()) <<
            "Checksum layer must set \"" << common::parseFromStr() << "\" or \"" <<
            common::parseUntilStr() << "\" property to indicate on what values checksum is calculated.";
        return false;
    }

    if (!parseFrom().empty()) {
        auto fromIdx = parseFindLayerIndex(layers, parseFrom());
        if (layers.size() <= fromIdx) {
            parseReportUnexpectedPropertyValue(common::parseFromStr(), parseFrom());
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
            parseReportUnexpectedPropertyValue(common::parseUntilStr(), parseUntil());
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
    if (!parseValidateSinglePropInstance(common::parseAlgStr(), true)) {
        return false;
    }

    auto iter = parseProps().find(common::parseAlgStr());
    assert(iter != parseProps().end());
    auto algStr = common::parseToLowerCopy(iter->second);

    static const std::map<std::string, ParseAlg> Map = {
        std::make_pair("custom", ParseAlg::Custom),
        std::make_pair("sum", ParseAlg::Sum),
        std::make_pair("crc-ccitt", ParseAlg::Crc_CCITT),
        std::make_pair("crc_ccitt", ParseAlg::Crc_CCITT),
        std::make_pair("crc-16", ParseAlg::Crc_16),
        std::make_pair("crc_16", ParseAlg::Crc_16),
        std::make_pair("crc-32", ParseAlg::Crc_32),
        std::make_pair("crc_32", ParseAlg::Crc_32),
        std::make_pair("xor", ParseAlg::Xor),
    };

    auto algIter = Map.find(algStr);
    if (algIter == Map.end()) {
        parseReportUnexpectedPropertyValue(common::parseAlgStr(), iter->second);
        return false;
    }

    m_alg = algIter->second;
    if (m_alg != ParseAlg::Custom) {
        return true;
    }

    if (!parseValidateSinglePropInstance(common::parseAlgNameStr(), true)) {
        return false;
    }

    auto algNameIter = parseProps().find(common::parseAlgNameStr());
    m_algName = &algNameIter->second;

    if (!common::parseIsValidName(*m_algName)) {
        parseReportUnexpectedPropertyValue(common::parseAlgNameStr(), iter->second);
        return false;
    }

    return true;
}

bool ParseChecksumLayerImpl::parseUpdateFrom()
{
    return parseValidateAndUpdateStringPropValue(common::parseFromStr(), m_from);
}

bool ParseChecksumLayerImpl::parseUpdateUntil()
{
    return parseValidateAndUpdateStringPropValue(common::parseUntilStr(), m_until);
}

bool ParseChecksumLayerImpl::parseUpdateVerifyBeforeRead()
{
    if (!parseValidateSinglePropInstance(common::parseVerifyBeforeReadStr())) {
        return false;
    }

    auto iter = parseProps().find(common::parseVerifyBeforeReadStr());
    if (iter == parseProps().end()) {
        return true;
    }

    bool ok = false;
    m_verifyBeforeRead = common::parseStrToBool(iter->second, &ok);
    if (!ok) {
        parseReportUnexpectedPropertyValue(common::parseVerifyBeforeReadStr(), iter->second);
        return false;
    }
    return true;
}

} // namespace parse

} // namespace commsdsl
