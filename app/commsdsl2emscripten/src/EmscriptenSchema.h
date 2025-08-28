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

#include "commsdsl/gen/GenSchema.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2emscripten
{

class EmscriptenGenerator;
class EmscriptenSchema final: public commsdsl::gen::GenSchema
{
    using GenBase = commsdsl::gen::GenSchema;

public:
    using ParseSchema = commsdsl::parse::ParseSchema;
    using GenElem = commsdsl::gen::GenElem;
    using GenStringsList = commsdsl::gen::util::GenStringsList;

    explicit EmscriptenSchema(EmscriptenGenerator& generator, ParseSchema parseObj, GenElem* parent);
    virtual ~EmscriptenSchema();

    static const EmscriptenSchema* emscriptenCast(const commsdsl::gen::GenSchema* schema)
    {
        return static_cast<const EmscriptenSchema*>(schema);
    }

    void emscriptenAddSourceFiles(GenStringsList& sources) const;
};

} // namespace commsdsl2emscripten
