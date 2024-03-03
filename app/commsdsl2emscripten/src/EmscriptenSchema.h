//
// Copyright 2022 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "commsdsl/gen/Schema.h"
#include "commsdsl/gen/util.h"

namespace commsdsl2emscripten
{

class EmscriptenGenerator;
class EmscriptenSchema final: public commsdsl::gen::Schema
{
    using Base = commsdsl::gen::Schema;

public:
    using StringsList = commsdsl::gen::util::StringsList;

    explicit EmscriptenSchema(EmscriptenGenerator& generator, commsdsl::parse::Schema dslObj, Elem* parent);
    virtual ~EmscriptenSchema();

    static const EmscriptenSchema* cast(const commsdsl::gen::Schema* schema)
    {
        return static_cast<const EmscriptenSchema*>(schema);
    }

    void emscriptenAddSourceFiles(StringsList& sources) const;
};

} // namespace commsdsl2emscripten
