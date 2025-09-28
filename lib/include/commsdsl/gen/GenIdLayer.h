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

#pragma once

#include "commsdsl/CommsdslApi.h"
#include "commsdsl/gen/GenLayer.h"
#include "commsdsl/parse/ParseLayer.h"

#include <memory>
#include <vector>

namespace commsdsl
{

namespace gen
{

class COMMSDSL_API GenIdLayer : public GenLayer
{
    using Base = GenLayer;
public:
    using ParseLayer = commsdsl::parse::ParseLayer;
    using ParseIdLayer = commsdsl::parse::ParseIdLayer;

    GenIdLayer(GenGenerator& generator, ParseLayer dslObj, GenElem* parent = nullptr);
    virtual ~GenIdLayer();

protected:
    ParseIdLayer genIdLayerParseObj() const;
};

} // namespace gen

} // namespace commsdsl
