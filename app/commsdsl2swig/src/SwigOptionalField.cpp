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

std::string SwigOptionalField::swigDefFuncs(const std::string& fieldType)
{
    static const std::string Templ = 
        "using Field = #^#FIELD_TYPE#$#;\n"
        "using Mode = comms_field_OptionalMode;\n\n"
        "Field& field();\n"
        "const Field& field() const;\n\n"
        "Mode getMode() const;\n"
        "void setMode(Mode val) const;\n"
        "bool isTenative() const;\n"
        "void setTenative();\n"
        "bool doesExist() const;\n"
        "void setExists();\n"
        "bool isMissing() const;\n"
        "void setMissing();\n"
    ;

    util::ReplacementMap repl = {
        {"FIELD_TYPE", fieldType}
    };

    return util::processTemplate(Templ, repl);
}

bool SwigOptionalField::writeImpl() const
{
    return swigWrite();
}

std::string SwigOptionalField::swigMembersDefImpl() const
{
    auto* mem = SwigField::cast(memberField());
    if (mem == nullptr) {
        return strings::emptyString();
    }

    return mem->swigClassDef();
}

std::string SwigOptionalField::swigValueTypeImpl() const
{
    return strings::emptyString();
}

std::string SwigOptionalField::swigValueAccImpl() const
{
    return strings::emptyString();
}

std::string SwigOptionalField::swigExtraPublicFuncsImpl() const
{
    auto* mem = SwigField::cast(memberField());
    if (mem == nullptr) {
        mem = SwigField::cast(externalField());
    }

    assert(mem != nullptr);

    return swigDefFuncs(SwigGenerator::cast(generator()).swigClassName(mem->field()));
}

} // namespace commsdsl2swig
