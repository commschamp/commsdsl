//
// Copyright 2018 - 2020 (C). Alex Robenko. All rights reserved.
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

#include "ChecksumLayerImpl.h"

#include <cassert>
#include <map>

#include "ProtocolImpl.h"
#include "common.h"

namespace commsdsl
{

ChecksumLayerImpl::ChecksumLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol),
    m_algName(&common::emptyString()),
    m_from(&common::emptyString()),
    m_until(&common::emptyString())
{
    assert(from().empty());
    assert(until().empty());
}

LayerImpl::Kind ChecksumLayerImpl::kindImpl() const
{
    return Kind::Checksum;
}

const XmlWrap::NamesList& ChecksumLayerImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList Names = {
        common::algStr(),
        common::algNameStr(),
        common::fromStr(),
        common::untilStr(),
        common::verifyBeforeReadStr()
    };
    return Names;
}

bool ChecksumLayerImpl::parseImpl()
{
    return
        updateAlg() &&
        updateFrom() &&
        updateUntil() &&
        updateVerifyBeforeRead();
}

bool ChecksumLayerImpl::verifyImpl(const LayerImpl::LayersList& layers)
{
    auto thisIdx = findThisLayerIndex(layers);
    assert(thisIdx < layers.size());

    if (from().empty() && until().empty()) {
        logError() << XmlWrap::logPrefix(getNode()) << 
            "Checksum layer must set \"" << common::fromStr() << "\" or \"" << 
            common::untilStr() << "\" property to indicate on what values checksum is calculated.";
        return false;
    }

    if (!from().empty()) {
        auto fromIdx = findLayerIndex(layers, from());
        if (layers.size() <= fromIdx) {
            reportUnexpectedPropertyValue(common::fromStr(), from());
            return false;
        }

        if (thisIdx <= fromIdx) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                "Layer \"" << from() << "\" must appear before the \"" << name() << "\".";
            return false;
        }
    }

    if (!until().empty()) {
        auto untilIdx = findLayerIndex(layers, until());
        if (layers.size() <= untilIdx) {
            reportUnexpectedPropertyValue(common::untilStr(), until());
            return false;
        }

        if (untilIdx <= thisIdx) {
            logError() << XmlWrap::logPrefix(getNode()) <<
                "Layer \"" << until() << "\" must appear after the \"" << name() << "\".";
            return false;
        }
    }

    return true;
}

bool ChecksumLayerImpl::updateAlg()
{
    if (!validateSinglePropInstance(common::algStr(), true)) {
        return false;
    }

    auto iter = props().find(common::algStr());
    assert(iter != props().end());
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
    };

    auto algIter = Map.find(algStr);
    if (algIter == Map.end()) {
        reportUnexpectedPropertyValue(common::algStr(), iter->second);
        return false;
    }

    m_alg = algIter->second;
    if (m_alg != Alg::Custom) {
        return true;
    }

    if (!validateSinglePropInstance(common::algNameStr(), true)) {
        return false;
    }

    auto algNameIter = props().find(common::algNameStr());
    m_algName = &algNameIter->second;

    if (!common::isValidName(*m_algName)) {
        reportUnexpectedPropertyValue(common::algNameStr(), iter->second);
        return false;
    }

    return true;
}

bool ChecksumLayerImpl::updateFrom()
{
    return validateAndUpdateStringPropValue(common::fromStr(), m_from);
}

bool ChecksumLayerImpl::updateUntil()
{
    return validateAndUpdateStringPropValue(common::untilStr(), m_until);
}

bool ChecksumLayerImpl::updateVerifyBeforeRead()
{
    if (!validateSinglePropInstance(common::verifyBeforeReadStr())) {
        return false;
    }

    auto iter = props().find(common::verifyBeforeReadStr());
    if (iter == props().end()) {
        return true;
    }

    bool ok = false;
    m_verifyBeforeRead = common::strToBool(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::verifyBeforeReadStr(), iter->second);
        return false;
    }
    return true;
}


} // namespace commsdsl
