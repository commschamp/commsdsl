//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/GenPayloadLayer.h"

#include "commsdsl/gen/GenGenerator.h"

#include <cassert>

namespace commsdsl
{

namespace gen
{

GenPayloadLayer::GenPayloadLayer(GenGenerator& generator, ParseLayer parseObj, GenElem* parent) :
    Base(generator, parseObj, parent)
{
    assert(parseObj.parseKind() == ParseLayer::Kind::Payload);
}

GenPayloadLayer::~GenPayloadLayer() = default;

GenPayloadLayer::ParsePayloadLayer GenPayloadLayer::genPayloadLayerParseObj() const
{
    return ParsePayloadLayer(genParseObj());
}

} // namespace gen

} // namespace commsdsl
