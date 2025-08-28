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

#include "EmscriptenSchema.h"

#include "EmscriptenGenerator.h"
#include "EmscriptenNamespace.h"

namespace commsdsl2emscripten
{

EmscriptenSchema::EmscriptenSchema(EmscriptenGenerator& generator, ParseSchema parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}   

EmscriptenSchema::~EmscriptenSchema() = default;

void EmscriptenSchema::emscriptenAddSourceFiles(GenStringsList& sources) const
{
    for (auto& nPtr : genNamespaces()) {
        auto* n = EmscriptenNamespace::emscriptenCast(nPtr.get());
        n->emscriptenAddSourceFiles(sources);
    }
}


} // namespace commsdsl2emscripten
