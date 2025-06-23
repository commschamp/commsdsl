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

#include "SwigLayer.h"

#include "commsdsl/gen/GenCustomLayer.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2swig
{

class SwigGenerator;
class SwigCustomLayer final : public commsdsl::gen::GenCustomLayer, public SwigLayer
{
    using Base = commsdsl::gen::GenCustomLayer;
    using SwigBase = SwigLayer;
public:
    SwigCustomLayer(SwigGenerator& generator, commsdsl::parse::ParseLayer dslObj, commsdsl::gen::GenElem* parent);
};

} // namespace commsdsl2swig
