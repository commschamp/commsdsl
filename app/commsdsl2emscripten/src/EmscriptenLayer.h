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

#include "commsdsl/gen/GenLayer.h"
#include "commsdsl/gen/util.h"

#include <string>
#include <vector>

namespace commsdsl2emscripten
{

class EmscriptenLayer
{
public:
    using StringsList = commsdsl::gen::util::StringsList;
    
    explicit EmscriptenLayer(commsdsl::gen::GenLayer& layer);
    virtual ~EmscriptenLayer();

    static const EmscriptenLayer* cast(const commsdsl::gen::GenLayer* layer);

    commsdsl::gen::GenLayer& layer()
    {
        return m_layer;
    }

    const commsdsl::gen::GenLayer& layer() const
    {
        return m_layer;
    }

    bool emscriptenIsMainInterfaceSupported() const;
    std::string emscriptenFieldAccName() const;
    std::string emscriptenFieldAccFuncName() const;
    void emscriptenAddHeaderInclude(StringsList& includes) const;
    std::string emscriptenHeaderClass() const;
    std::string emscriptenSourceCode() const;

protected:
    virtual bool emscriptenIsMainInterfaceSupportedImpl() const;
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
    commsdsl::gen::GenLayer& m_layer;
};

} // namespace commsdsl2emscripten
