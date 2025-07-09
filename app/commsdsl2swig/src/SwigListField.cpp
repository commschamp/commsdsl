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

#include "SwigListField.h"

#include "SwigField.h"
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

SwigListField::SwigListField(SwigGenerator& generator, commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

bool SwigListField::genPrepareImpl() 
{
    if (!Base::genPrepareImpl()) {
        return false;
    }

    auto* elem = genMemberElementField();
    if (elem == nullptr) {
        elem = genExternalElementField();
    }

    assert(elem != nullptr);
    SwigField::cast(elem)->swigSetListElement();    

    return true;
}

bool SwigListField::genWriteImpl() const
{
    return swigWrite();
}

std::string SwigListField::swigMembersDeclImpl() const
{
    auto* elem = SwigField::cast(genMemberElementField());
    if (elem == nullptr) {
        return strings::genEmptyString();
    }

    return elem->swigClassDecl();
}

std::string SwigListField::swigValueTypeDeclImpl() const
{
    static const std::string Templ = 
        "using ValueType = std::vector<#^#ELEM#$#>;\n";

    auto* elem = genMemberElementField();
    if (elem == nullptr) {
        elem = genExternalElementField();
    }

    assert(elem != nullptr);

    util::GenReplacementMap repl = {
        {"ELEM", SwigGenerator::cast(genGenerator()).swigClassName(*elem)}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string SwigListField::swigValueAccDeclImpl() const
{
    return 
        "ValueType& value();\n" + 
        SwigBase::swigValueAccDeclImpl();
}

std::string SwigListField::swigExtraPublicFuncsCodeImpl() const
{
    std::string templ = 
        "using ValueType = std::vector<#^#ELEM#$#>;\n\n"
        "ValueType& value()\n"
        "{\n"
        "    return reinterpret_cast<ValueType&>(Base::value());\n"
        "}\n\n"
        "const ValueType& getValue() const\n"
        "{\n"
        "    return reinterpret_cast<const ValueType&>(Base::getValue());\n"
        "}\n";

    if (!field().genParseObj().parseIsFixedValue()) {
        templ += 
            "\n"
            "void setValue(const ValueType& val)\n"
            "{\n"
            "    Base::setValue(reinterpret_cast<const Base::ValueType&>(val));\n"
            "}\n";        
    }

    auto* elem = genMemberElementField();
    if (elem == nullptr) {
        elem = genExternalElementField();
    }

    assert(elem != nullptr);

    util::GenReplacementMap repl = {
        {"ELEM", SwigGenerator::cast(genGenerator()).swigClassName(*elem)}
    };

    return util::genProcessTemplate(templ, repl);        
}

void SwigListField::swigAddDefImpl(StringsList& list) const
{
    auto* elem = genMemberElementField();
    if (elem == nullptr) {
        elem = genExternalElementField();
    }   

    SwigField::cast(elem)->swigAddDef(list);
}

void SwigListField::swigAddMembersCodeImpl(StringsList& list) const
{
    auto* elem = genMemberElementField();
    if (elem == nullptr) {
        elem = genExternalElementField();
    }    

    assert(elem != nullptr);

    SwigField::cast(elem)->swigAddCode(list);
}


} // namespace commsdsl2swig
