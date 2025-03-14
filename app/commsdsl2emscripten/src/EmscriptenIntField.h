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

#include "commsdsl/gen/IntField.h"

namespace commsdsl2emscripten
{

class EmscriptenGenerator;
class EmscriptenIntField final : public commsdsl::gen::IntField, public EmscriptenField
{
    using Base = commsdsl::gen::IntField;
    using EmscriptenBase = EmscriptenField;
public:
    EmscriptenIntField(EmscriptenGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent);

protected:
    // Base overrides
    virtual bool writeImpl() const override;    

    // EmscriptenBase overrides
    virtual std::string emscriptenHeaderValueAccImpl() const override;
    virtual std::string emscriptenHeaderExtraPublicFuncsImpl() const override;
    virtual std::string emscriptenSourceBindFuncsImpl() const override;

private:
    std::string emscriptenHeaderSpecialsInternal() const;
    std::string emscriptenHeaderDisplayDecimalsInternal() const;
    std::string emscriptenHeaderScaledInternal() const;
    std::string emscriptenSourceSpecialsBindInternal() const;
    std::string emscriptenSourceDisplayDecimalsBindInternal() const;
    std::string emscriptenSourceScaledBindInternal() const;
};

} // namespace commsdsl2emscripten
