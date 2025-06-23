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

SwigIntField::SwigIntField(SwigGenerator& generator, commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

bool SwigIntField::writeImpl() const
{
    return swigWrite();
}

std::string SwigIntField::swigValueTypeDeclImpl() const
{
    static const std::string Templ = 
        "using ValueType = #^#TYPE#$#;\n";

    auto obj = intDslObj();
    util::ReplacementMap repl = {
        {"TYPE", SwigGenerator::cast(generator()).swigConvertIntType(obj.type(), obj.maxLength())}
    };

    return util::processTemplate(Templ, repl);
}

std::string SwigIntField::swigExtraPublicFuncsDeclImpl() const
{
    static const std::string Templ = 
        "static bool hasSpecials();\n"
        "#^#SCPECIALS#$#\n"
        "#^#DISPLAY_DECIMALS#$#\n"
        "#^#SCALED#$#\n";

    util::ReplacementMap repl {
        {"SCPECIALS", swigSpecialsDeclInternal()},
        {"DISPLAY_DECIMALS", swigDisplayDecimalsDeclInternal()},
        {"SCALED", swigScaledFuncsDeclInternal()}
    };

    return util::processTemplate(Templ, repl);
}

std::string SwigIntField::swigExtraPublicFuncsCodeImpl() const
{
    return swigScaledFuncsCodeInternal();
}

std::string SwigIntField::swigSpecialsDeclInternal() const
{
    auto& specials = specialsSortedByValue();
    if (specials.empty()) {
        return strings::emptyString();
    }

    util::StringsList specialsList;
    auto& gen = SwigGenerator::cast(generator());
    for (auto& s : specials) {
        if (!gen.doesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        static const std::string Templ =
            "static ValueType value#^#SPEC_ACC#$#();\n"
            "bool is#^#SPEC_ACC#$#() const;\n"
            "void set#^#SPEC_ACC#$#();\n"
        ;

        util::ReplacementMap repl = {
            {"SPEC_ACC", comms::className(s.first)},
        };

        specialsList.push_back(util::processTemplate(Templ, repl));
    }    

    return util::strListToString(specialsList, "", "");
}

std::string SwigIntField::swigDisplayDecimalsDeclInternal() const
{
    auto obj = intDslObj();
    auto scaling = obj.scaling();
    std::string result;
    if (scaling.first != scaling.second) {
        result = "static unsigned displayDecimals();";
    }

    return result;
}

std::string SwigIntField::swigScaledFuncsDeclInternal() const
{
    auto obj = intDslObj();
    auto scaling = obj.scaling();
    auto num = scaling.first;
    auto denom = scaling.second;

    if ((num == 1) && (denom == 1)) {
        return strings::emptyString();
    }

    std::string Templ = {
        "double getScaled() const;\n"
        "void setScaled(double val);\n"
    };

    return Templ;
}

std::string SwigIntField::swigScaledFuncsCodeInternal() const
{
    auto obj = intDslObj();
    auto scaling = obj.scaling();
    auto num = scaling.first;
    auto denom = scaling.second;

    if ((num == 1) && (denom == 1)) {
        return strings::emptyString();
    }
        
    static const std::string Templ = 
        "double getScaled() const { return Base::getScaled<double>(); }\n"
        "void setScaled(double val) {Base::setScaled(val); }\n"
    ;

    return Templ;        
}

} // namespace commsdsl2swig
