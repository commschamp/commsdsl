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

#include "SwigBundleField.h"

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

SwigBundleField::SwigBundleField(SwigGenerator& generator, commsdsl::parse::Field dslObj, commsdsl::gen::Elem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

bool SwigBundleField::prepareImpl()
{
    return 
        Base::prepareImpl() &&
        swigPrepareInternal();
}

bool SwigBundleField::writeImpl() const
{
    return swigWrite();
}

bool SwigBundleField::swigPrepareInternal()
{
    m_swigMembers = swigTransformFieldsList(members());
    return true;
}

std::string SwigBundleField::swigMembersDeclImpl() const
{
    StringsList memberDefs;
    memberDefs.reserve(m_swigMembers.size());

    for (auto* m : m_swigMembers) {
        memberDefs.push_back(m->swigClassDecl());
    }

    return util::strListToString(memberDefs, "\n", "\n");
}

std::string SwigBundleField::swigValueAccDeclImpl() const
{
    if (dslObj().semanticType() != commsdsl::parse::Field::SemanticType::Length) {
        return strings::emptyString();
    }

    return swigSemanticTypeLengthValueAccDecl();
}

std::string SwigBundleField::swigExtraPublicFuncsDeclImpl() const
{
    StringsList accFuncs;
    accFuncs.reserve(m_swigMembers.size());

    auto& gen = SwigGenerator::cast(generator());
    for (auto* m : m_swigMembers) {
        static const std::string Templ = 
            "#^#CLASS_NAME#$#& field_#^#ACC_NAME#$#();\n"
        ;

        util::ReplacementMap repl = {
            {"CLASS_NAME", gen.swigClassName(m->field())},
            {"ACC_NAME", comms::accessName(m->field().dslObj().name())}
        };

        accFuncs.push_back(util::processTemplate(Templ, repl));
    }

    return util::strListToString(accFuncs, "", "");
}

std::string SwigBundleField::swigExtraPublicFuncsCodeImpl() const
{
    StringsList accFuncs;
    accFuncs.reserve(m_swigMembers.size());

    auto& gen = SwigGenerator::cast(generator());
    for (auto* m : m_swigMembers) {
        static const std::string Templ = 
            "#^#CLASS_NAME#$#& field_#^#ACC_NAME#$#()\n"
            "{\n"
            "    return static_cast<#^#CLASS_NAME#$#&>(Base::field_#^#ACC_NAME#$#());\n"
            "}\n"
            "const #^#CLASS_NAME#$#& field_#^#ACC_NAME#$#() const\n"
            "{\n"
            "    return static_cast<const #^#CLASS_NAME#$#&>(Base::field_#^#ACC_NAME#$#());\n"
            "}\n"
        ;

        util::ReplacementMap repl = {
            {"CLASS_NAME", gen.swigClassName(m->field())},
            {"ACC_NAME", comms::accessName(m->field().dslObj().name())}
        };

        accFuncs.push_back(util::processTemplate(Templ, repl));
    }

    std::string valueAccCode;
    if (dslObj().semanticType() == commsdsl::parse::Field::SemanticType::Length) {
        valueAccCode = swigSemanticTypeLengthValueAccCode();
    }

    static const std::string Templ = 
        "#^#VALUE_ACC#$#\n"
        "#^#MEM_ACC#$#\n";

    util::ReplacementMap repl = {
        {"VALUE_ACC", std::move(valueAccCode)},
        {"MEM_ACC", util::strListToString(accFuncs, "\n", "")}
    };

    return util::processTemplate(Templ, repl);
}


void SwigBundleField::swigAddDefImpl(StringsList& list) const
{
    for (auto* m : m_swigMembers) {
        m->swigAddDef(list);
    }    
}

void SwigBundleField::swigAddMembersCodeImpl(StringsList& list) const
{
    for (auto* m : m_swigMembers) {
        m->swigAddCode(list);
    }    
}


} // namespace commsdsl2swig
