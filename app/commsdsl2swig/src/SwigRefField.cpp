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

#include "SwigRefField.h"

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

SwigRefField::SwigRefField(SwigGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    SwigBase(static_cast<GenBase&>(*this))
{
}

bool SwigRefField::genWriteImpl() const
{
    return swigWrite();
}

std::string SwigRefField::swigValueAccDeclImpl() const
{
    return strings::genEmptyString();
}

std::string SwigRefField::swigExtraPublicFuncsCodeImpl() const
{
    static const std::string Templ =
        "using RefType = #^#REF_TYPE#$#;\n"
        "#^#REF_TYPE#$#& ref()\n"
        "{\n"
        "    return static_cast<#^#REF_TYPE#$#&>(reinterpret_cast<#^#BASE_TYPE#$#&>(*this));\n"
        "}\n"
    ;

    auto& gen = SwigGenerator::swigCast(genGenerator());
    auto* field = SwigField::swigCast(genReferencedField());
    util::GenReplacementMap repl = {
        {"REF_TYPE", gen.swigClassName(field->swigGenField())},
        {"BASE_TYPE", field->swigTemplateScope()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string SwigRefField::swigPublicDeclImpl() const
{
    static const std::string Templ =
        "using RefType = #^#REF_TYPE#$#;\n"
        "#^#REF_TYPE#$#& ref();\n"
        "#^#FUNCS#$#\n";

    auto& gen = SwigGenerator::swigCast(genGenerator());
    util::GenReplacementMap repl = {
        {"REF_TYPE", gen.swigClassName(*genReferencedField())},
        {"FUNCS", swigCommonPublicFuncsDecl()}
    };

    return util::genProcessTemplate(Templ, repl);
}

void SwigRefField::swigAddDefImpl(GenStringsList& list) const
{
    // Make sure the referenced field is defined before
    SwigField::swigCast(genReferencedField())->swigAddDef(list);
}

void SwigRefField::swigAddMembersCodeImpl(GenStringsList& list) const
{
    // Make sure the referenced field is defined before
    SwigField::swigCast(genReferencedField())->swigAddCode(list);
}

} // namespace commsdsl2swig
