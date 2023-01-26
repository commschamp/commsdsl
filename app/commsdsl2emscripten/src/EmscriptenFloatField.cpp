//
// Copyright 2022 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "EmscriptenFloatField.h"

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

EmscriptenFloatField::EmscriptenFloatField(EmscriptenGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    EmscriptenBase(static_cast<Base&>(*this))
{
}

bool EmscriptenFloatField::writeImpl() const
{
    return emscriptenWrite();
}

std::string EmscriptenFloatField::emscriptenHeaderValueAccImpl() const
{
    return emscriptenHeaderValueAccByValue();
}

std::string EmscriptenFloatField::emscriptenHeaderExtraPublicFuncsImpl() const
{
    static const std::string Templ = 
        "static bool hasSpecials()\n"
        "{\n"
        "    return Base::hasSpecials();\n"
        "}\n\n"
        "#^#SPECIALS#$#\n"
        "static unsigned displayDecimals()\n"
        "{\n"
        "    return Base::displayDecimals();\n"
        "}\n";

    util::ReplacementMap repl = {
        {"SPECIALS", emscriptenHeaderSpecialsInternal()}
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenFloatField::emscriptenSourceBindFuncsImpl() const
{
    static const std::string Templ = 
        ".class_function(\"hasSpecials\", &#^#CLASS_NAME#$#::hasSpecials)\n"
        "#^#SPECIALS#$#\n"
        ".class_function(\"displayDecimals\", &#^#CLASS_NAME#$#::displayDecimals)";

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()},
        {"SPECIALS", emscriptenSourceSpecialsBindInternal()}
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenFloatField::emscriptenHeaderSpecialsInternal() const
{
    auto& specials = specialsSortedByValue();
    if (specials.empty()) {
        return strings::emptyString();
    }

    util::StringsList specialsList;
    auto& gen = EmscriptenGenerator::cast(generator());
    for (auto& s : specials) {
        if (!gen.doesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string Templ =
            "static ValueType value#^#SPEC_ACC#$#()\n"
            "{\n"
            "    return Base::value#^#SPEC_ACC#$#();\n"
            "}\n\n"
            "bool is#^#SPEC_ACC#$#() const\n"
            "{\n"
            "    return Base::is#^#SPEC_ACC#$#();\n"
            "}\n\n"
            "void set#^#SPEC_ACC#$#()\n"
            "{\n"
            "    Base::set#^#SPEC_ACC#$#();\n"
            "}\n"
        ;

        util::ReplacementMap repl = {
            {"SPEC_ACC", comms::className(s.first)},
        };

        specialsList.push_back(util::processTemplate(Templ, repl));
    }    

    return util::strListToString(specialsList, "\n", "");
}

std::string EmscriptenFloatField::emscriptenSourceSpecialsBindInternal() const
{
    auto& specials = specialsSortedByValue();
    if (specials.empty()) {
        return strings::emptyString();
    }

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()},
    };

    util::StringsList specialsList;
    auto& gen = EmscriptenGenerator::cast(generator());
    for (auto& s : specials) {
        if (!gen.doesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string Templ =
            ".class_function(\"value#^#SPEC_ACC#$#\", &#^#CLASS_NAME#$#::value#^#SPEC_ACC#$#)\n"
            ".function(\"is#^#SPEC_ACC#$#\", &#^#CLASS_NAME#$#::is#^#SPEC_ACC#$#)\n"
            ".function(\"set#^#SPEC_ACC#$#\", &#^#CLASS_NAME#$#::set#^#SPEC_ACC#$#)";

        repl["SPEC_ACC"] = comms::className(s.first);

        specialsList.push_back(util::processTemplate(Templ, repl));
    }    

    return util::strListToString(specialsList, "\n", "");    
}

} // namespace commsdsl2emscripten
