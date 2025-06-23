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

#include "EmscriptenField.h"

#include "commsdsl/gen/GenSetField.h"

namespace commsdsl2emscripten
{

class EmscriptenGenerator;
class EmscriptenSetField final : public commsdsl::gen::GenSetField, public EmscriptenField
{
    using Base = commsdsl::gen::GenSetField;
    using EmscriptenBase = EmscriptenField;
public:
    EmscriptenSetField(EmscriptenGenerator& generator, commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent);

protected:
    // Base overrides
    virtual bool writeImpl() const override;    

    // EmscriptenBase overrides
    virtual std::string emscriptenHeaderValueAccImpl() const override;
    virtual std::string emscriptenHeaderExtraPublicFuncsImpl() const override;
    virtual std::string emscriptenSourceBindFuncsImpl() const override;
    virtual std::string emscriptenSourceBindExtraImpl() const override;

private:
};

} // namespace commsdsl2emscripten
