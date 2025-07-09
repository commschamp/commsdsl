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

#include "EmscriptenRefField.h"

#include "EmscriptenGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2emscripten
{

EmscriptenRefField::EmscriptenRefField(EmscriptenGenerator& generator, commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) : 
    Base(generator, dslObj, parent),
    EmscriptenBase(static_cast<Base&>(*this))
{
}

bool EmscriptenRefField::genWriteImpl() const
{
    return emscriptenWrite();
}

void EmscriptenRefField::emscriptenHeaderAddExtraIncludesImpl(StringsList& incs) const
{
    auto* refField = EmscriptenField::cast(genReferencedField());
    assert(refField != nullptr);
    incs.push_back(refField->emscriptenRelHeaderPath());
}

std::string EmscriptenRefField::emscriptenHeaderValueAccImpl() const
{
    return strings::genEmptyString();
}

std::string EmscriptenRefField::emscriptenHeaderExtraPublicFuncsImpl() const
{
    static const std::string Templ = 
        "#^#REF_FIELD#$#* ref()\n"
        "{\n"
        "    return\n"
        "        static_cast<#^#REF_FIELD#$#*>(\n"
        "            reinterpret_cast<#^#REF_BASE#$#*>(this));\n"
        "}\n";

    auto& gen = EmscriptenGenerator::cast(genGenerator());
    auto* refField = EmscriptenField::cast(genReferencedField());
    assert(refField != nullptr);

    util::GenReplacementMap repl = {
        {"REF_FIELD", gen.emscriptenClassName(refField->field())},
        {"REF_BASE", refField->emscriptenTemplateScope()}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenRefField::emscriptenSourceBindValueAccImpl() const
{
    return strings::genEmptyString();
}

std::string EmscriptenRefField::emscriptenSourceBindFuncsImpl() const
{
    static const std::string Templ = 
        ".function(\"ref\", &#^#CLASS_NAME#$#::ref, emscripten::allow_raw_pointers())";

    util::GenReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()}
    };

    return util::genProcessTemplate(Templ, repl);
}


} // namespace commsdsl2emscripten
