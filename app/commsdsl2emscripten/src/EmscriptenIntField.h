//
// Copyright 2022 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

#include "commsdsl/gen/GenIntField.h"

namespace commsdsl2emscripten
{

class EmscriptenGenerator;
class EmscriptenIntField final : public commsdsl::gen::GenIntField, public EmscriptenField
{
    using GenBase = commsdsl::gen::GenIntField;
    using EmscriptenBase = EmscriptenField;

public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;

    EmscriptenIntField(EmscriptenGenerator& generator, ParseField parseObj, GenElem* parent);

protected:
    // GenBase overrides
    [[nodiscard]] virtual bool genWriteImpl() const override;

    // EmscriptenBase overrides
    [[nodiscard]] virtual std::string emscriptenHeaderValueAccImpl() const override;
    [[nodiscard]] virtual std::string emscriptenHeaderExtraPublicFuncsImpl() const override;
    [[nodiscard]] virtual std::string emscriptenSourceBindFuncsImpl() const override;

private:
    [[nodiscard]] std::string emscriptenHeaderSpecialsInternal() const;
    [[nodiscard]] std::string emscriptenHeaderDisplayDecimalsInternal() const;
    [[nodiscard]] std::string emscriptenHeaderScaledInternal() const;
    [[nodiscard]] std::string emscriptenSourceSpecialsBindInternal() const;
    [[nodiscard]] std::string emscriptenSourceDisplayDecimalsBindInternal() const;
    [[nodiscard]] std::string emscriptenSourceScaledBindInternal() const;
};

} // namespace commsdsl2emscripten
