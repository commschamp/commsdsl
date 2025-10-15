//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CLayer.h"

#include "commsdsl/gen/GenCustomLayer.h"

namespace commsdsl2c
{

class CGenerator;
class CCustomLayer final : public commsdsl::gen::GenCustomLayer, public CLayer
{
    using GenBase = commsdsl::gen::GenCustomLayer;
    using CBase = CLayer;

public:
    using ParseLayer = commsdsl::parse::ParseLayer;
    using GenElem = commsdsl::gen::GenElem;

    CCustomLayer(CGenerator& generator, ParseLayer parseObj, GenElem* parent);
};

} // namespace commsdsl2c
