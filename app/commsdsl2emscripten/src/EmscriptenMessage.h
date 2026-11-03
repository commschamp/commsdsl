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

    [[nodiscard]] static const EmscriptenMessage* emscriptenCast(const commsdsl::gen::GenMessage* obj)
    {
        return static_cast<const EmscriptenMessage*>(obj);
    }

    [[nodiscard]] std::string emscriptenRelHeader() const;
    void emscriptenAddSourceFiles(GenStringsList& sources) const;

protected:
    [[nodiscard]] virtual bool genPrepareImpl() override;
    [[nodiscard]] virtual bool genWriteImpl() const override;

private:
    using EmscriptenFieldsList = EmscriptenField::EmscriptenFieldsList;

    [[nodiscard]] bool emscriptenWriteHeaderInternal() const;
    [[nodiscard]] bool emscriptenWriteSourceInternal() const;

    [[nodiscard]] std::string emscriptenHeaderIncludesInternal() const;
    [[nodiscard]] std::string emscriptenHeaderFieldsInternal() const;
    [[nodiscard]] std::string emscriptenHeaderClassInternal() const;
    [[nodiscard]] std::string emscriptenSourceFieldsInternal() const;
    [[nodiscard]] std::string emscriptenSourceCodeInternal() const;
    [[nodiscard]] const EmscriptenInterface* emscriptenGetInterfaceInternal() const;

    EmscriptenFieldsList m_emscriptenFields;
};

} // namespace commsdsl2emscripten
