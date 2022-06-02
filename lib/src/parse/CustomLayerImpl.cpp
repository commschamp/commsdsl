//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

#include "CustomLayerImpl.h"

#include "ProtocolImpl.h"
#include "common.h"

#include <map>

namespace commsdsl
{

namespace parse
{

CustomLayerImpl::CustomLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

LayerImpl::Kind CustomLayerImpl::kindImpl() const
{
    return Kind::Custom;
}

bool CustomLayerImpl::parseImpl()
{
    return 
        Base::parseImpl() &&
        updateIdReplacement() &&
        updateSemanticLayerType();
}

const XmlWrap::NamesList& CustomLayerImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::idReplacementStr(),
        common::semanticLayerTypeStr(),
    };

    return List;
}

bool CustomLayerImpl::updateIdReplacement()
{
    auto& prop = common::idReplacementStr();
    if (!validateSinglePropInstance(prop)) {
        return false;
    }

    auto iter = props().find(prop);
    if (iter == props().end()) {
        return true;
    }

    bool ok = false;
    bool idReplacement = common::strToBool(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(prop, iter->second);
        return false;
    }

    if (idReplacement) {
        m_sematicLayerType = Kind::Id;
    }

    return true;
}

bool CustomLayerImpl::updateSemanticLayerType()
{
    auto& prop = common::semanticLayerTypeStr();
    if (!validateSinglePropInstance(prop)) {
        return false;
    }

    auto iter = props().find(prop);
    if (iter == props().end()) {
        return true;
    }

    if (iter->second.empty()) {
        return true;
    }

    if (m_sematicLayerType != Kind::Custom) {
        logError() << XmlWrap::logPrefix(getNode()) <<
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
        reportUnexpectedPropertyValue(prop, iter->second);
        return false;
    }

    m_sematicLayerType = kindIter->second;
    return true;
}

} // namespace parse

} // namespace commsdsl
