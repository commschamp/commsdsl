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

#include "EmscriptenIntField.h"

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

EmscriptenIntField::EmscriptenIntField(EmscriptenGenerator& generator, ParseField parseObj, GenElem* parent) : 
    GenBase(generator, parseObj, parent),
    EmscriptenBase(static_cast<GenBase&>(*this))
{
}

bool EmscriptenIntField::genWriteImpl() const
{
    return emscriptenWrite();
}

std::string EmscriptenIntField::emscriptenHeaderValueAccImpl() const
{
    return emscriptenHeaderValueAccByValue();
}

std::string EmscriptenIntField::emscriptenHeaderExtraPublicFuncsImpl() const
{
    static const std::string Templ = 
        "static bool hasSpecials()\n"
        "{\n"
        "    return Base::hasSpecials();\n"
        "}\n\n"
        "#^#SCPECIALS#$#\n"
        "#^#DISPLAY_DECIMALS#$#\n"
        "#^#SCALED#$#\n";

    util::GenReplacementMap repl = {
        {"SPECIALS", emscriptenHeaderSpecialsInternal()},
        {"DISPLAY_DECIMALS", emscriptenHeaderDisplayDecimalsInternal()},
        {"SCALED", emscriptenHeaderScaledInternal()},
    };     

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenIntField::emscriptenSourceBindFuncsImpl() const
{
    static const std::string Templ = 
        "#^#SCPECIALS#$#\n"
        "#^#DISPLAY_DECIMALS#$#\n"
        "#^#SCALED#$#\n";

    util::GenReplacementMap repl = {
        {"SPECIALS", emscriptenSourceSpecialsBindInternal()},
        {"DISPLAY_DECIMALS", emscriptenSourceDisplayDecimalsBindInternal()},
        {"SCALED", emscriptenSourceScaledBindInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenIntField::emscriptenHeaderSpecialsInternal() const
{
    auto& specials = genSpecialsSortedByValue();
    if (specials.empty()) {
        return strings::genEmptyString();
    }

    util::GenStringsList specialsList;
    auto& gen = EmscriptenGenerator::emscriptenCast(genGenerator());
    for (auto& s : specials) {
        if (!gen.genDoesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
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

        util::GenReplacementMap repl = {
            {"SPEC_ACC", comms::genClassName(s.first)},
        };

        specialsList.push_back(util::genProcessTemplate(Templ, repl));
    }    

    return util::genStrListToString(specialsList, "\n", "");
}

std::string EmscriptenIntField::emscriptenHeaderDisplayDecimalsInternal() const
{
    auto obj = genIntFieldParseObj();
    auto scaling = obj.parseScaling();
    std::string result;
    if (scaling.first != scaling.second) {
        result = 
            "static unsigned displayDecimals()\n"
            "{\n"
            "    return Base::displayDecimals();\n"
            "}\n";
    }

    return result;
}

std::string EmscriptenIntField::emscriptenHeaderScaledInternal() const
{
    auto obj = genIntFieldParseObj();
    auto scaling = obj.parseScaling();
    auto num = scaling.first;
    auto denom = scaling.second;

    if ((num == 1) && (denom == 1)) {
        return strings::genEmptyString();
    }

    static const std::string Templ = 
        "double getScaled() const\n"
        "{\n"
        "    return Base::getScaled<double>();\n"
        "}\n\n"
        "void setScaled(double val)\n"
        "{\n"
        "    Base::setScaled(val);\n"
        "}\n";

    return Templ;
}


std::string EmscriptenIntField::emscriptenSourceSpecialsBindInternal() const
{
    auto& specials = genSpecialsSortedByValue();
    if (specials.empty()) {
        return strings::genEmptyString();
    }

    util::GenStringsList specialsList;
    auto& gen = EmscriptenGenerator::emscriptenCast(genGenerator());
    util::GenReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()}
    };

    for (auto& s : specials) {
        if (!gen.genDoesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string Templ =
            ".class_function(\"value#^#SPEC_ACC#$#\", &#^#CLASS_NAME#$#::value#^#SPEC_ACC#$#)\n"
            ".function(\"is#^#SPEC_ACC#$#\", &#^#CLASS_NAME#$#::is#^#SPEC_ACC#$#)\n"
            ".function(\"set#^#SPEC_ACC#$#\", &#^#CLASS_NAME#$#::set#^#SPEC_ACC#$#)"
        ;

        repl["SPEC_ACC"] = comms::genClassName(s.first);
        specialsList.push_back(util::genProcessTemplate(Templ, repl));
    }    

    static const std::string Templ = 
        "#^#SPECIALS#$#\n"
        ".class_function(\"hasSpecials\", &#^#CLASS_NAME#$#::hasSpecials)";

    repl["SPECIALS"] = util::genStrListToString(specialsList, "\n", "");

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenIntField::emscriptenSourceDisplayDecimalsBindInternal() const
{
    auto obj = genIntFieldParseObj();
    auto scaling = obj.parseScaling();
    std::string result;
    if (scaling.first != scaling.second) {
        static const std::string Templ = 
            ".class_function(\"displayDecimals\", &#^#CLASS_NAME#$#::displayDecimals)";

        util::GenReplacementMap repl = {
            {"CLASS_NAME", emscriptenBindClassName()}
        };

        result = util::genProcessTemplate(Templ, repl);
    }

    return result;
}

std::string EmscriptenIntField::emscriptenSourceScaledBindInternal() const
{
    auto obj = genIntFieldParseObj();
    auto scaling = obj.parseScaling();
    auto num = scaling.first;
    auto denom = scaling.second;

    if ((num == 1) && (denom == 1)) {
        return strings::genEmptyString();
    }

    static const std::string Templ = 
        ".function(\"getScaled\", &#^#CLASS_NAME#$#::getScaled)\n"
        ".function(\"setScaled\", &#^#CLASS_NAME#$#::setScaled)";

    util::GenReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()}
    };

    return util::genProcessTemplate(Templ, repl);    
}

} // namespace commsdsl2emscripten
