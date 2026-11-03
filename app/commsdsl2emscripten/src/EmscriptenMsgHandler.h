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

#include "commsdsl/gen/GenNamespace.h"
#include "commsdsl/gen/util.h"

#include <string>

namespace commsdsl2emscripten
{

class EmscriptenGenerator;
class EmscriptenNamespace;

class EmscriptenMsgHandler
{
public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using GenMessagesAccessList = commsdsl::gen::GenNamespace::GenMessagesAccessList;

    EmscriptenMsgHandler(EmscriptenGenerator& generator, const EmscriptenNamespace& parent);
    [[nodiscard]] bool emscriptenWrite() const;
    [[nodiscard]] std::string emscriptenClassName() const;
    [[nodiscard]] std::string emscriptenRelHeader() const;
    void emscriptenAddSourceFiles(GenStringsList& sources) const;

private:
    [[nodiscard]] bool emscriptenWriteHeaderInternal() const;
    [[nodiscard]] bool emscriptenWriteSrcInternal() const;

    [[nodiscard]] std::string emscriptenHeaderIncludesInternal() const;
    [[nodiscard]] std::string emscriptenHeaderHandleFuncsInternal() const;
    [[nodiscard]] std::string emscriptenSourceHandleFuncsInternal() const;
    [[nodiscard]] std::string emscriptenSourceWrapperClassInternal() const;
    [[nodiscard]] std::string emscriptenSourceWrapperFuncsInternal() const;
    [[nodiscard]] std::string emscriptenSourceBindInternal() const;
    [[nodiscard]] std::string emscriptenSourceBindFuncsInternal() const;
    [[nodiscard]] GenMessagesAccessList emscriptenMessagesListInternal() const;

    EmscriptenGenerator& m_emscriptenGenerator;
    const EmscriptenNamespace& m_parent;
};

} // namespace commsdsl2emscripten
