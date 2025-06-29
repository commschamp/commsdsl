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

#include "EmscriptenValueLayer.h"

#include "EmscriptenGenerator.h"
#include "EmscriptenInterface.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2emscripten
{

EmscriptenValueLayer::EmscriptenValueLayer(EmscriptenGenerator& generator, commsdsl::parse::ParseLayer dslObj, commsdsl::gen::GenElem* parent) : 
    Base(generator, dslObj, parent),
    EmscriptenBase(static_cast<Base&>(*this))
{
}

bool EmscriptenValueLayer::emscriptenIsMainInterfaceSupportedImpl() const
{
    auto& gen = EmscriptenGenerator::cast(genGenerator());
    auto* iFace = gen.emscriptenMainInterface();
    assert(iFace != nullptr);
    return genIsInterfaceSupported(iFace);
}

std::string EmscriptenValueLayer::emscriptenHeaderExtraFuncsImpl() const
{
    auto obj = valueDslObj();
    if (!obj.parsePseudo()) {
        return strings::genEmptyString();
    }

    static const std::string Templ = 
        "Field* pseudoField()\n"
        "{\n"
        "    return reinterpret_cast<Field*>(&Base::pseudoField());\n"
        "}\n";

    return Templ;
}

std::string EmscriptenValueLayer::emscriptenSourceExtraFuncsImpl() const
{
    auto obj = valueDslObj();
    if (!obj.parsePseudo()) {
        return strings::genEmptyString();
    }

    static const std::string Templ = 
        ".function(\"pseudoField\", &#^#CLASS_NAME#$#::pseudoField, emscripten::allow_raw_pointers())";

    auto& gen = EmscriptenGenerator::cast(genGenerator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.emscriptenClassName(*this)}
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2emscripten
