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

#include "EmscriptenInterface.h"

#include "commsdsl/gen/GenLayer.h"
#include "commsdsl/gen/util.h"

#include <string>
#include <vector>

namespace commsdsl2emscripten
{

class EmscriptenLayer
{
public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using GenLayer = commsdsl::gen::GenLayer;

    explicit EmscriptenLayer(GenLayer& layer);
    virtual ~EmscriptenLayer();

    [[nodiscard]] static const EmscriptenLayer* emscriptenCast(const GenLayer* layer);

    [[nodiscard]] commsdsl::gen::GenLayer& emscriptenGenLayer()
    {
        return m_genLayer;
    }

    [[nodiscard]] const commsdsl::gen::GenLayer& emscriptenGenLayer() const
    {
        return m_genLayer;
    }

    [[nodiscard]] bool emscriptenIsInterfaceSupported(const EmscriptenInterface& iFace) const;
    [[nodiscard]] std::string emscriptenFieldAccName() const;
    [[nodiscard]] std::string emscriptenFieldAccFuncName() const;
    void emscriptenAddHeaderInclude(GenStringsList& includes) const;
    [[nodiscard]] std::string emscriptenHeaderClass() const;
    [[nodiscard]] std::string emscriptenSourceCode() const;

protected:
    [[nodiscard]] virtual bool emscriptenIsInterfaceSupportedImpl(const EmscriptenInterface& iFace) const;
    [[nodiscard]] virtual std::string emscriptenHeaderFieldDefImpl() const;
    [[nodiscard]] virtual std::string emscriptenFieldClassNameImpl() const;
    [[nodiscard]] virtual std::string emscriptenHeaderExtraFuncsImpl() const;
    [[nodiscard]] virtual std::string emscriptenSourceFieldBindImpl() const;
    [[nodiscard]] virtual std::string emscriptenSourceExtraFuncsImpl() const;

    [[nodiscard]] std::string emscriptenTemplateScope() const;

private:
    [[nodiscard]] std::string emscriptenHeaderFieldDefInternal() const;
    [[nodiscard]] std::string emscriptenHeaderClassDefInternal() const;
    [[nodiscard]] std::string emscriptenFieldClassNameInternal() const;
    [[nodiscard]] std::string emscriptenSourceFieldBindInternal() const;
    [[nodiscard]] std::string emscriptenSourceCodeInternal() const;

private:
    commsdsl::gen::GenLayer& m_genLayer;
};

} // namespace commsdsl2emscripten
