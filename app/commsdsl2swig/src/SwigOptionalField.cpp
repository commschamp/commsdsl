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

#include "SwigOptionalField.h"

#include "SwigComms.h"
#include "SwigGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2swig
{

SwigOptionalField::SwigOptionalField(SwigGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

std::string SwigOptionalField::swigDeclFuncs(const SwigGenerator& generator, const std::string& fieldType)
{
    static const std::string Templ = 
        "using Field = #^#FIELD_TYPE#$#;\n"
        "using Mode = #^#OPT_MODE#$#;\n\n"
        "Field& field();\n\n"
        "Mode getMode() const;\n"
        "void setMode(Mode val);\n"
        "bool isTentative() const;\n"
        "void setTentative();\n"
        "bool doesExist() const;\n"
        "void setExists();\n"
        "bool isMissing() const;\n"
        "void setMissing();\n"
    ;

    auto& gen = SwigGenerator::cast(generator);
    util::ReplacementMap repl = {
        {"FIELD_TYPE", fieldType},
        {"OPT_MODE", SwigComms::swigOptionalModeClassName(gen)}
    };

    return util::processTemplate(Templ, repl);
}

bool SwigOptionalField::writeImpl() const
{
    return swigWrite();
}

std::string SwigOptionalField::swigMembersDeclImpl() const
{
    auto* mem = SwigField::cast(memberField());
    if (mem == nullptr) {
        return strings::emptyString();
    }

    return mem->swigClassDecl();
}


std::string SwigOptionalField::swigValueTypeDeclImpl() const
{
    return strings::emptyString();
}

std::string SwigOptionalField::swigValueAccDeclImpl() const
{
    return strings::emptyString();
}

std::string SwigOptionalField::swigExtraPublicFuncsDeclImpl() const
{
    auto* mem = SwigField::cast(memberField());
    if (mem == nullptr) {
        mem = SwigField::cast(externalField());
    }

    assert(mem != nullptr);
    auto& gen = SwigGenerator::cast(generator());
    return swigDeclFuncs(gen, gen.swigClassName(mem->field()));
}

void SwigOptionalField::swigAddDefImpl(StringsList& list) const
{
    auto* mem = SwigField::cast(memberField());
    if (mem == nullptr) {
        mem = SwigField::cast(externalField());
    }

    assert(mem != nullptr);
    mem->swigAddDef(list);
}

void SwigOptionalField::swigAddMembersCodeImpl(StringsList& list) const
{
    auto* mem = SwigField::cast(memberField());
    if (mem == nullptr) {
        mem = SwigField::cast(externalField());
    }

    assert(mem != nullptr);
    mem->swigAddCode(list);
}

} // namespace commsdsl2swig
