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

#include "commsdsl/gen/GenMessage.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2emscripten
{

class EmscriptenGenerator;
class EmscriptenInterface;
class EmscriptenMessage final: public commsdsl::gen::GenMessage
{
    using GenBase = commsdsl::gen::GenMessage;

public:
    using ParseMessage = commsdsl::parse::ParseMessage;
    using GenElem = commsdsl::gen::GenElem;
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    explicit EmscriptenMessage(EmscriptenGenerator& generator, ParseMessage parseObj, GenElem* parent);
    virtual ~EmscriptenMessage();

    static const EmscriptenMessage* emscriptenCast(const commsdsl::gen::GenMessage* i)
    {
        return static_cast<const EmscriptenMessage*>(i);
    }

    std::string emscriptenRelHeader() const;
    void emscriptenAddSourceFiles(GenStringsList& sources) const;

protected:
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;

private:
    using EmscriptenFieldsList = EmscriptenField::EmscriptenFieldsList;

    bool emscriptenWriteHeaderInternal() const;
    bool emscriptenWriteSourceInternal() const;

    std::string emscriptenHeaderIncludesInternal() const;
    std::string emscriptenHeaderFieldsInternal() const;
    std::string emscriptenHeaderClassInternal() const;
    std::string emscriptenSourceFieldsInternal() const;
    std::string emscriptenSourceCodeInternal() const;
    const EmscriptenInterface* emscriptenGetInterfaceInternal() const;

    EmscriptenFieldsList m_emscriptenFields;
};

} // namespace commsdsl2emscripten
