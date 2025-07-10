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

#include "commsdsl/gen/GenVariantField.h"

namespace commsdsl2emscripten
{

class EmscriptenGenerator;
class EmscriptenVariantField final : public commsdsl::gen::GenVariantField, public EmscriptenField
{
    using GenBase = commsdsl::gen::GenVariantField;
    using EmscriptenBase = EmscriptenField;

public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;

    EmscriptenVariantField(EmscriptenGenerator& generator, ParseField parseObj, GenElem* parent);

protected:
    // GenBase overrides
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;    

    // EmscriptenBase overrides
    virtual std::string emscriptenHeaderExtraCodePrefixImpl() const override;
    virtual std::string emscriptenHeaderValueAccImpl() const override;
    virtual std::string emscriptenHeaderExtraPublicFuncsImpl() const override;
    virtual std::string emscriptenSourceExtraCodeImpl() const override;
    virtual std::string emscriptenSourceBindValueAccImpl() const override;
    virtual std::string emscriptenSourceBindFuncsImpl() const override;

private:
    std::string emscriptenHeaderMembersAccessInternal() const;
    std::string emscriptenHandlerClassInternal() const;
    std::string emscriptenHandlerWrapperClassInternal() const;
    std::string emscriptenSourceWrapperFuncsInternal() const;
    std::string emscriptenSourceWrapperBindsInternal() const;
    std::string emscriptenSourceHandleFuncsInternal() const;
};

} // namespace commsdsl2emscripten
