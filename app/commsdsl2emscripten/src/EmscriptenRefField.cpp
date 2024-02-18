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

EmscriptenRefField::EmscriptenRefField(EmscriptenGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    EmscriptenBase(static_cast<Base&>(*this))
{
}

bool EmscriptenRefField::writeImpl() const
{
    return emscriptenWrite();
}

void EmscriptenRefField::emscriptenHeaderAddExtraIncludesImpl(StringsList& incs) const
{
    auto* refField = EmscriptenField::cast(referencedField());
    assert(refField != nullptr);
    incs.push_back(refField->emscriptenRelHeaderPath());
}

std::string EmscriptenRefField::emscriptenHeaderValueAccImpl() const
{
    return strings::emptyString();
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

    auto& gen = EmscriptenGenerator::cast(generator());
    auto* refField = EmscriptenField::cast(referencedField());
    assert(refField != nullptr);

    util::ReplacementMap repl = {
        {"REF_FIELD", gen.emscriptenClassName(refField->field())},
        {"REF_BASE", refField->emscriptenTemplateScope()}
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenRefField::emscriptenSourceBindValueAccImpl() const
{
    return strings::emptyString();
}

std::string EmscriptenRefField::emscriptenSourceBindFuncsImpl() const
{
    static const std::string Templ = 
        ".function(\"ref\", &#^#CLASS_NAME#$#::ref, emscripten::allow_raw_pointers())";

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()}
    };

    return util::processTemplate(Templ, repl);
}


} // namespace commsdsl2emscripten
