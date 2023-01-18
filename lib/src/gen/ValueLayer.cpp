//
// Copyright 2021 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/ValueLayer.h"
#include "commsdsl/gen/Generator.h"

#include <algorithm>
#include <cassert>

namespace commsdsl
{

namespace gen
{

ValueLayer::ValueLayer(Generator& generator, commsdsl::parse::Layer dslObj, Elem* parent) :
    Base(generator, dslObj, parent)
{
    assert(dslObj.kind() == commsdsl::parse::Layer::Kind::Value);
}

ValueLayer::~ValueLayer() = default;

bool ValueLayer::isInterfaceSupported(const Interface* iFace) const
{
    auto obj = valueDslObj();
    auto supportedInterfaces = obj.interfaces();

    if (supportedInterfaces.empty()) {
        return true;
    }

    return 
        std::any_of(
            supportedInterfaces.begin(), supportedInterfaces.end(),
            [this, iFace](auto& i)
            {
                return generator().findInterface(i.externalRef()) == iFace;
            });  
}

commsdsl::parse::ValueLayer ValueLayer::valueDslObj() const
{
    return commsdsl::parse::ValueLayer(dslObj());
}

} // namespace gen

} // namespace commsdsl
