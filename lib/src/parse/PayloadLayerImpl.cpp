//
// Copyright 2018 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "PayloadLayerImpl.h"

#include "ProtocolImpl.h"
#include "common.h"

namespace commsdsl
{

namespace parse
{

PayloadLayerImpl::PayloadLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

LayerImpl::Kind PayloadLayerImpl::kindImpl() const
{
    return Kind::Payload;
}

bool PayloadLayerImpl::verifyImpl(const LayerImpl::LayersList& layers)
{
    return verifySingleLayer(layers, common::payloadStr());
}

bool PayloadLayerImpl::mustHaveFieldImpl() const
{
    return false;
}

} // namespace parse

} // namespace commsdsl
