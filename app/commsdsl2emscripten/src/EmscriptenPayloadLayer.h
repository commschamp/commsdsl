//
// Copyright 2022 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "EmscriptenLayer.h"

#include "commsdsl/gen/GenPayloadLayer.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2emscripten
{

class EmscriptenGenerator;
class EmscriptenPayloadLayer final : public commsdsl::gen::GenPayloadLayer, public EmscriptenLayer
{
    using GenBase = commsdsl::gen::GenPayloadLayer;
    using EmscriptenBase = EmscriptenLayer;

public:
    using ParseLayer = commsdsl::parse::ParseLayer;
    using GenElem = commsdsl::gen::GenElem;

    EmscriptenPayloadLayer(EmscriptenGenerator& generator, ParseLayer parseObj, GenElem* parent);

protected:
    virtual std::string emscriptenHeaderFieldDefImpl() const override;
    virtual std::string emscriptenFieldClassNameImpl() const override;
    virtual std::string emscriptenSourceFieldBindImpl() const override;
};

} // namespace commsdsl2emscripten
