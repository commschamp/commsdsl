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

#include "commsdsl/gen/GenValueLayer.h"

#include "commsdsl/gen/GenGenerator.h"

#include <algorithm>
#include <cassert>

namespace commsdsl
{

namespace gen
{

GenValueLayer::GenValueLayer(GenGenerator& generator, ParseLayer parseObj, GenElem* parent) :
    Base(generator, parseObj, parent)
{
    assert(parseObj.parseKind() == ParseLayer::ParseKind::Value);
}

GenValueLayer::~GenValueLayer() = default;

bool GenValueLayer::genIsInterfaceSupported(const GenInterface* iFace) const
{
    auto obj = genValueLayerParseObj();
    auto supportedInterfaces = obj.parseInterfaces();

    if (supportedInterfaces.empty()) {
        return true;
    }

    return 
        std::any_of(
            supportedInterfaces.begin(), supportedInterfaces.end(),
            [this, iFace](auto& i)
            {
                return genGenerator().genFindInterface(i.parseExternalRef()) == iFace;
            });  
}

GenValueLayer::ParseValueLayer GenValueLayer::genValueLayerParseObj() const
{
    return ParseValueLayer(genParseObj());
}

} // namespace gen

} // namespace commsdsl
