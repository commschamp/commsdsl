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

#include "SwigListField.h"

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

SwigListField::SwigListField(SwigGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

bool SwigListField::writeImpl() const
{
    return swigWrite();
}

std::string SwigListField::swigMembersDeclImpl() const
{
    auto* elem = SwigField::cast(memberElementField());
    if (elem == nullptr) {
        return strings::emptyString();
    }

    return elem->swigClassDecl();
}

std::string SwigListField::swigValueTypeDeclImpl() const
{
    static const std::string Templ = 
        "using ValueType = std::vector<#^#ELEM#$#>;\n";

    auto* elem = memberElementField();
    if (elem == nullptr) {
        elem = externalElementField();
    }

    assert(elem != nullptr);

    util::ReplacementMap repl = {
        {"ELEM", SwigGenerator::cast(generator()).swigClassName(*elem)}
    };

    return util::processTemplate(Templ, repl);
}

std::string SwigListField::swigValueAccDeclImpl() const
{
    return 
        "ValueType& value();\n" + 
        SwigBase::swigValueAccDeclImpl();
}

std::string SwigListField::swigExtraPublicFuncsCodeImpl() const
{
    static const std::string Templ = 
        "using ValueType = std::vector<#^#ELEM#$#>;\n\n"
        "ValueType& value()\n"
        "{\n"
        "    return reinterpret_cast<ValueType&>(Base::value());\n"
        "}\n\n"
        "const ValueType& getValue() const\n"
        "{\n"
        "    return reinterpret_cast<const ValueType&>(Base::getValue());\n"
        "}\n\n"
        "void setValue(const ValueType& val)\n"
        "{\n"
        "    Base::setValue(reinterpret_cast<const Base::ValueType&>(val));\n"
        "}\n";

    auto* elem = memberElementField();
    if (elem == nullptr) {
        elem = externalElementField();
    }

    assert(elem != nullptr);

    util::ReplacementMap repl = {
        {"ELEM", SwigGenerator::cast(generator()).swigClassName(*elem)}
    };

    return util::processTemplate(Templ, repl);        
}

void SwigListField::swigAddDefImpl(StringsList& list) const
{
    auto* elem = memberElementField();
    if (elem == nullptr) {
        elem = externalElementField();
    }   

    static const std::string Templ = 
        "%template(#^#CLASS_NAME#$#_ValueType) std::vector<#^#ELEM#$#>;";

    auto& gen = SwigGenerator::cast(generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"ELEM", gen.swigClassName(*elem)}
    }; 

    list.push_back(util::processTemplate(Templ, repl));
    SwigField::cast(elem)->swigAddDef(list);
}

void SwigListField::swigAddMembersCodeImpl(StringsList& list) const
{
    auto* elem = memberElementField();
    if (elem == nullptr) {
        elem = externalElementField();
    }    

    assert(elem != nullptr);

    SwigField::cast(elem)->swigAddCode(list);
}


} // namespace commsdsl2swig
