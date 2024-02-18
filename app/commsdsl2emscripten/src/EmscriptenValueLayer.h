//
// Copyright 2022 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/ValueLayer.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2emscripten
{

class EmscriptenGenerator;
class EmscriptenValueLayer final : public commsdsl::gen::ValueLayer, public EmscriptenLayer
{
    using Base = commsdsl::gen::ValueLayer;
    using EmscriptenBase = EmscriptenLayer;
public:
    EmscriptenValueLayer(EmscriptenGenerator& generator, commsdsl::parse::Layer dslObj, commsdsl::gen::Elem* parent);

protected:
    virtual bool emscriptenIsMainInterfaceSupportedImpl() const override;
    virtual std::string emscriptenHeaderExtraFuncsImpl() const override;    
    virtual std::string emscriptenSourceExtraFuncsImpl() const override;
};

} // namespace commsdsl2emscripten
