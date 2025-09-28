//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "SwigIntField.h"

#include "SwigGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2swig
{

SwigIntField::SwigIntField(SwigGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    SwigBase(static_cast<GenBase&>(*this))
{
}

bool SwigIntField::genWriteImpl() const
{
    return swigWrite();
}

std::string SwigIntField::swigValueTypeDeclImpl() const
{
    static const std::string Templ =
        "using ValueType = #^#TYPE#$#;\n";

    auto obj = genIntFieldParseObj();
    util::GenReplacementMap repl = {
        {"TYPE", SwigGenerator::swigCast(genGenerator()).swigConvertIntType(obj.parseType(), obj.parseMaxLength())}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string SwigIntField::swigExtraPublicFuncsDeclImpl() const
{
    static const std::string Templ =
        "static bool hasSpecials();\n"
        "#^#SCPECIALS#$#\n"
        "#^#DISPLAY_DECIMALS#$#\n"
        "#^#SCALED#$#\n";

    util::GenReplacementMap repl {
        {"SCPECIALS", swigSpecialsDeclInternal()},
        {"DISPLAY_DECIMALS", swigDisplayDecimalsDeclInternal()},
        {"SCALED", swigScaledFuncsDeclInternal()}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string SwigIntField::swigExtraPublicFuncsCodeImpl() const
{
    return swigScaledFuncsCodeInternal();
}

std::string SwigIntField::swigSpecialsDeclInternal() const
{
    auto& specials = genSpecialsSortedByValue();
    if (specials.empty()) {
        return strings::genEmptyString();
    }

    util::GenStringsList specialsList;
    auto& gen = SwigGenerator::swigCast(genGenerator());
    for (auto& s : specials) {
        if (!gen.genDoesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string Templ =
            "static ValueType value#^#SPEC_ACC#$#();\n"
            "bool is#^#SPEC_ACC#$#() const;\n"
            "void set#^#SPEC_ACC#$#();\n"
        ;

        util::GenReplacementMap repl = {
            {"SPEC_ACC", comms::genClassName(s.first)},
        };

        specialsList.push_back(util::genProcessTemplate(Templ, repl));
    }

    return util::genStrListToString(specialsList, "", "");
}

std::string SwigIntField::swigDisplayDecimalsDeclInternal() const
{
    auto obj = genIntFieldParseObj();
    auto scaling = obj.parseScaling();
    std::string result;
    if (scaling.first != scaling.second) {
        result = "static unsigned displayDecimals();";
    }

    return result;
}

std::string SwigIntField::swigScaledFuncsDeclInternal() const
{
    auto obj = genIntFieldParseObj();
    auto scaling = obj.parseScaling();
    auto num = scaling.first;
    auto denom = scaling.second;

    if ((num == 1) && (denom == 1)) {
        return strings::genEmptyString();
    }

    std::string Templ = {
        "double getScaled() const;\n"
        "void setScaled(double val);\n"
    };

    return Templ;
}

std::string SwigIntField::swigScaledFuncsCodeInternal() const
{
    auto obj = genIntFieldParseObj();
    auto scaling = obj.parseScaling();
    auto num = scaling.first;
    auto denom = scaling.second;

    if ((num == 1) && (denom == 1)) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "double getScaled() const { return Base::getScaled<double>(); }\n"
        "void setScaled(double val) {Base::setScaled(val); }\n"
    ;

    return Templ;
}

} // namespace commsdsl2swig
