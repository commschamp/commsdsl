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

    static const EmscriptenLayer* emscriptenCast(const GenLayer* layer);

    commsdsl::gen::GenLayer& emscriptenGenLayer()
    {
        return m_genLayer;
    }

    const commsdsl::gen::GenLayer& emscriptenGenLayer() const
    {
        return m_genLayer;
    }

    bool emscriptenIsInterfaceSupported(const EmscriptenInterface& iFace) const;
    std::string emscriptenFieldAccName() const;
    std::string emscriptenFieldAccFuncName() const;
    void emscriptenAddHeaderInclude(GenStringsList& includes) const;
    std::string emscriptenHeaderClass() const;
    std::string emscriptenSourceCode() const;

protected:
    virtual bool emscriptenIsInterfaceSupportedImpl(const EmscriptenInterface& iFace) const;
    virtual std::string emscriptenHeaderFieldDefImpl() const;
    virtual std::string emscriptenFieldClassNameImpl() const;
    virtual std::string emscriptenHeaderExtraFuncsImpl() const;
    virtual std::string emscriptenSourceFieldBindImpl() const;
    virtual std::string emscriptenSourceExtraFuncsImpl() const;

    std::string emscriptenTemplateScope() const;

private:
    std::string emscriptenHeaderFieldDefInternal() const;
    std::string emscriptenHeaderClassDefInternal() const;
    std::string emscriptenFieldClassNameInternal() const;
    std::string emscriptenSourceFieldBindInternal() const;
    std::string emscriptenSourceCodeInternal() const;

private:
    commsdsl::gen::GenLayer& m_genLayer;
};

} // namespace commsdsl2emscripten
