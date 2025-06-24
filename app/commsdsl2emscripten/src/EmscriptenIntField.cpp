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

EmscriptenIntField::EmscriptenIntField(EmscriptenGenerator& generator, commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) : 
    Base(generator, dslObj, parent),
    EmscriptenBase(static_cast<Base&>(*this))
{
}

bool EmscriptenIntField::writeImpl() const
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

    util::ReplacementMap repl = {
        {"SPECIALS", emscriptenHeaderSpecialsInternal()},
        {"DISPLAY_DECIMALS", emscriptenHeaderDisplayDecimalsInternal()},
        {"SCALED", emscriptenHeaderScaledInternal()},
    };     

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenIntField::emscriptenSourceBindFuncsImpl() const
{
    static const std::string Templ = 
        "#^#SCPECIALS#$#\n"
        "#^#DISPLAY_DECIMALS#$#\n"
        "#^#SCALED#$#\n";

    util::ReplacementMap repl = {
        {"SPECIALS", emscriptenSourceSpecialsBindInternal()},
        {"DISPLAY_DECIMALS", emscriptenSourceDisplayDecimalsBindInternal()},
        {"SCALED", emscriptenSourceScaledBindInternal()},
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenIntField::emscriptenHeaderSpecialsInternal() const
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

std::string EmscriptenIntField::emscriptenHeaderDisplayDecimalsInternal() const
{
    auto obj = intDslObj();
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
    auto obj = intDslObj();
    auto scaling = obj.parseScaling();
    auto num = scaling.first;
    auto denom = scaling.second;

    if ((num == 1) && (denom == 1)) {
        return strings::emptyString();
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
    auto& specials = specialsSortedByValue();
    if (specials.empty()) {
        return strings::emptyString();
    }

    util::StringsList specialsList;
    auto& gen = EmscriptenGenerator::cast(generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()}
    };

    for (auto& s : specials) {
        if (!gen.doesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string Templ =
            ".class_function(\"value#^#SPEC_ACC#$#\", &#^#CLASS_NAME#$#::value#^#SPEC_ACC#$#)\n"
            ".function(\"is#^#SPEC_ACC#$#\", &#^#CLASS_NAME#$#::is#^#SPEC_ACC#$#)\n"
            ".function(\"set#^#SPEC_ACC#$#\", &#^#CLASS_NAME#$#::set#^#SPEC_ACC#$#)"
        ;

        repl["SPEC_ACC"] = comms::className(s.first);
        specialsList.push_back(util::processTemplate(Templ, repl));
    }    

    static const std::string Templ = 
        "#^#SPECIALS#$#\n"
        ".class_function(\"hasSpecials\", &#^#CLASS_NAME#$#::hasSpecials)";

    repl["SPECIALS"] = util::strListToString(specialsList, "\n", "");

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenIntField::emscriptenSourceDisplayDecimalsBindInternal() const
{
    auto obj = intDslObj();
    auto scaling = obj.parseScaling();
    std::string result;
    if (scaling.first != scaling.second) {
        static const std::string Templ = 
            ".class_function(\"displayDecimals\", &#^#CLASS_NAME#$#::displayDecimals)";

        util::ReplacementMap repl = {
            {"CLASS_NAME", emscriptenBindClassName()}
        };

        result = util::processTemplate(Templ, repl);
    }

    return result;
}

std::string EmscriptenIntField::emscriptenSourceScaledBindInternal() const
{
    auto obj = intDslObj();
    auto scaling = obj.parseScaling();
    auto num = scaling.first;
    auto denom = scaling.second;

    if ((num == 1) && (denom == 1)) {
        return strings::emptyString();
    }

    static const std::string Templ = 
        ".function(\"getScaled\", &#^#CLASS_NAME#$#::getScaled)\n"
        ".function(\"setScaled\", &#^#CLASS_NAME#$#::setScaled)";

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenBindClassName()}
    };

    return util::processTemplate(Templ, repl);    
}

} // namespace commsdsl2emscripten
