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
#include "EmscriptenLayer.h"

#include "commsdsl/gen/GenFrame.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2emscripten
{

class EmscriptenGenerator;
class EmscriptenNamespace;

class EmscriptenFrame final: public commsdsl::gen::GenFrame
{
    using GenBase = commsdsl::gen::GenFrame;

public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    explicit EmscriptenFrame(EmscriptenGenerator& generator, commsdsl::parse::ParseFrame parseObj, commsdsl::gen::GenElem* parent);
    virtual ~EmscriptenFrame();

    [[nodiscard]] static const EmscriptenFrame* emscriptenCast(const commsdsl::gen::GenFrame* obj)
    {
        return static_cast<const EmscriptenFrame*>(obj);
    }

    void emscriptenAddSourceFiles(GenStringsList& sources) const;
    [[nodiscard]] const EmscriptenNamespace* emscriptenFindInputNamespace() const;

    [[nodiscard]] const EmscriptenInterface* emscriptenInterface() const;

protected:
    [[nodiscard]] virtual bool genPrepareImpl() override;
    [[nodiscard]] virtual bool genWriteImpl() const override;

private:
    using EmscriptenLayersList = std::vector<EmscriptenLayer*>;

    [[nodiscard]] bool emscriptenWriteHeaderInternal() const;
    [[nodiscard]] bool emscriptenWriteSourceInternal() const;

    [[nodiscard]] std::string emscriptenHeaderIncludesInternal() const;
    [[nodiscard]] std::string emscriptenHeaderLayersInternal() const;
    [[nodiscard]] std::string emscriptenHeaderAllFieldsInternal() const;
    [[nodiscard]] std::string emscriptenHeaderAllFieldsNameInternal() const;
    [[nodiscard]] std::string emscriptenHeaderClassInternal() const;
    [[nodiscard]] std::string emscriptenHeaderLayersAccessInternal() const;
    [[nodiscard]] std::string emscriptenSourceLayersInternal() const;
    [[nodiscard]] std::string emscriptenSourceAllFieldsInternal() const;
    [[nodiscard]] std::string emscriptenSourceCodeInternal() const;
    [[nodiscard]] std::string emscriptenSourceBindInternal() const;
    [[nodiscard]] std::string emscriptenSourceLayersAccBindInternal() const;

    [[nodiscard]] const EmscriptenInterface* emscriptenInterfaceInternal() const;

    EmscriptenLayersList m_emscriptenLayers;
    bool m_validFrame = true;
};

} // namespace commsdsl2emscripten
