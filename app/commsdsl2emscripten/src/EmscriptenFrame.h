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

#include "commsdsl/gen/GenFrame.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2emscripten
{

class EmscriptenGenerator;
class EmscriptenNamespace;

class EmscriptenFrame final: public commsdsl::gen::GenFrame
{
    using Base = commsdsl::gen::GenFrame;

public:
    using StringsList = commsdsl::gen::util::StringsList;

    explicit EmscriptenFrame(EmscriptenGenerator& generator, commsdsl::parse::ParseFrame dslObj, commsdsl::gen::GenElem* parent);
    virtual ~EmscriptenFrame();

    static const EmscriptenFrame* cast(const commsdsl::gen::GenFrame* i)
    {
        return static_cast<const EmscriptenFrame*>(i);
    }        

    void emscriptenAddSourceFiles(StringsList& sources) const;
    const EmscriptenNamespace* emscriptenFindInputNamespace() const;


protected:    
    virtual bool prepareImpl() override;
    virtual bool writeImpl() const override;    

private:
    using EmscriptenLayersList = std::vector<EmscriptenLayer*>;

    bool emscriptenWriteHeaderInternal() const;
    bool emscriptenWriteSourceInternal() const;    

    std::string emscriptenHeaderIncludesInternal() const;
    std::string emscriptenHeaderLayersInternal() const;
    std::string emscriptenHeaderAllFieldsInternal() const;
    std::string emscriptenHeaderAllFieldsNameInternal() const;
    std::string emscriptenHeaderClassInternal() const;
    std::string emscriptenHeaderLayersAccessInternal() const;
    std::string emscriptenSourceLayersInternal() const;
    std::string emscriptenSourceAllFieldsInternal() const;
    std::string emscriptenSourceCodeInternal() const;
    std::string emscriptenSourceBindInternal() const;
    std::string emscriptenSourceLayersAccBindInternal() const;

    EmscriptenLayersList m_emscriptenLayers;
    bool m_validFrame = true;
};

} // namespace commsdsl2emscripten
