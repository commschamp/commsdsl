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
#include "EmscriptenInputMessages.h"
#include "EmscriptenMsgHandler.h"
#include "EmscriptenMsgId.h"

#include "commsdsl/gen/Namespace.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2emscripten
{

class EmscriptenGenerator;
class EmscriptenNamespace final: public commsdsl::gen::Namespace
{
    using Base = commsdsl::gen::Namespace;

public:
    using StringsList = commsdsl::gen::util::StringsList;

    explicit EmscriptenNamespace(EmscriptenGenerator& generator, commsdsl::parse::Namespace dslObj, Elem* parent);
    virtual ~EmscriptenNamespace();

    static const EmscriptenNamespace* cast(const commsdsl::gen::Namespace* schema)
    {
        return static_cast<const EmscriptenNamespace*>(schema);
    }

    void emscriptenAddSourceFiles(StringsList& sources) const;
    void emscriptenAddCommsMessageIncludes(StringsList& includes) const;
    void emscriptenAddInputMessageFwdIncludes(StringsList& includes) const;
    void emscriptenAddInputMessageIncludes(StringsList& includes) const;

    std::string emscriptenHandlerRelHeader() const;
    std::string emscriptenHandlerClassName() const;
    std::string emscriptenInputRelHeader() const;
    std::string emscriptenInputClassName() const;

    bool emscriptenHasInput() const;

protected:
    virtual bool writeImpl() const override;    

private:
    EmscriptenMsgId m_msgId; 
    EmscriptenMsgHandler m_handler; 
    EmscriptenInputMessages m_input;  
};

} // namespace commsdsl2emscripten
