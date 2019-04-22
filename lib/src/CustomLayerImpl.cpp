//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
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

namespace commsdsl
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
    if (!validateSinglePropInstance(common::idReplacementStr())) {
        return false;
    }

    auto iter = props().find(common::idReplacementStr());
    if (iter == props().end()) {
        return true;
    }

    bool ok = false;
    m_idReplacement = common::strToBool(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::idReplacementStr(), iter->second);
        return false;
    }
    return true;
}

const XmlWrap::NamesList& CustomLayerImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::idReplacementStr()
    };

    return List;
}

} // namespace commsdsl
