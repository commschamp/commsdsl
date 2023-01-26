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

#pragma once

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/parse/Layer.h"
#include "commsdsl/gen/Layer.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class COMMSDSL_API PayloadLayer : public Layer
{
    using Base = Layer;
public:

    PayloadLayer(Generator& generator, commsdsl::parse::Layer dslObj, Elem* parent = nullptr);
    virtual ~PayloadLayer();

protected:    
    commsdsl::parse::PayloadLayer payloadDslObj() const;
};

} // namespace gen

} // namespace commsdsl
