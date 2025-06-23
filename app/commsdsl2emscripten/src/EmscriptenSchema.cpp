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

#include <algorithm>

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2emscripten
{

EmscriptenSchema::EmscriptenSchema(EmscriptenGenerator& generator, commsdsl::parse::ParseSchema dslObj, commsdsl::gen::GenElem* parent) :
    Base(generator, dslObj, parent)
{
}   

EmscriptenSchema::~EmscriptenSchema() = default;

void EmscriptenSchema::emscriptenAddSourceFiles(StringsList& sources) const
{
    for (auto& nPtr : namespaces()) {
        auto* n = EmscriptenNamespace::cast(nPtr.get());
        n->emscriptenAddSourceFiles(sources);
    }
}


} // namespace commsdsl2emscripten
