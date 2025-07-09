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

SwigFloatField::SwigFloatField(SwigGenerator& generator, commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

bool SwigFloatField::genWriteImpl() const
{
    return swigWrite();
}

std::string SwigFloatField::swigValueTypeDeclImpl() const
{
    static const std::string Templ = 
        "using ValueType = #^#TYPE#$#;\n";

    auto obj = genFloatFieldParseObj();
    util::GenReplacementMap repl = {
        {"TYPE", comms::genCppFloatTypeFor(obj.parseType())}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string SwigFloatField::swigExtraPublicFuncsDeclImpl() const
{
    static const std::string Templ = 
        "bool hasSpecials();\n"
        "#^#SCPECIALS#$#\n"
        "static unsigned displayDecimals();\n";

    util::GenReplacementMap repl {
        {"SCPECIALS", swigSpecialsDeclInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string SwigFloatField::swigSpecialsDeclInternal() const
{
    auto& specials = genSpecialsSortedByValue();
    if (specials.empty()) {
        return strings::genEmptyString();
    }

    util::GenStringsList specialsList;
    auto& gen = SwigGenerator::cast(genGenerator());
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

} // namespace commsdsl2swig
