//
// Copyright 2021 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "SwigFloatField.h"

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

SwigFloatField::SwigFloatField(SwigGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

bool SwigFloatField::writeImpl() const
{
    return swigWrite();
}

std::string SwigFloatField::swigValueTypeImpl() const
{
    static const std::string Templ = 
        "using ValueType = #^#TYPE#$#;\n";

    auto obj = floatDslObj();
    util::ReplacementMap repl = {
        {"TYPE", comms::cppFloatTypeFor(obj.type())}
    };

    return util::processTemplate(Templ, repl);
}

std::string SwigFloatField::swigExtraPublicFuncsImpl() const
{
    static const std::string Templ = 
        "bool hasSpecials();\n"
        "#^#SCPECIALS#$#\n"
        "static unsigned displayDecimals();\n";

    util::ReplacementMap repl {
        {"SCPECIALS", swigSpecialsDefInternal()},
    };

    return util::processTemplate(Templ, repl);
}


std::string SwigFloatField::swigSpecialsDefInternal() const
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

    static const std::string Templ = 
        "using SpecialNameInfo = std::pair<ValueType, const char*>;\n"
        "using SpecialNamesMapInfo = std::pair<const SpecialNameInfo*, #^#SIZE_T#$#>;\n"
        "static SpecialNamesMapInfo specialNamesMap();\n"
        "#^#SPECIALS#$#\n"
    ;

    util::ReplacementMap repl = {
        {"SPECIALS", util::strListToString(specialsList, "", "")},
        {"SIZE_T", gen.swigConvertCppType("std::size_t")}
    };

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2swig
