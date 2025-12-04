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

#include "commsdsl/gen/util.h"

#include <string>

namespace commsdsl2emscripten
{

class EmscriptenGenerator;
class EmscriptenComms
{
public:
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    static bool emscriptenWrite(EmscriptenGenerator& generator);
    static void emscriptenAddSourceFiles(const EmscriptenGenerator& generator, GenStringsList& sources);

private:
    explicit EmscriptenComms(EmscriptenGenerator& generator) : m_emscriptenGenerator(generator) {}

    bool emscriptenWriteErrorStatusInternal() const;
    bool emscriptenWriteOptionalModeInternal() const;

    EmscriptenGenerator& m_emscriptenGenerator;
};

} // namespace commsdsl2emscripten
