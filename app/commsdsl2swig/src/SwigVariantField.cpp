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

#include "SwigVariantField.h"

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

SwigVariantField::SwigVariantField(SwigGenerator& generator, commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) : 
    Base(generator, dslObj, parent),
    SwigBase(static_cast<Base&>(*this))
{
}

bool SwigVariantField::prepareImpl()
{
    return 
        Base::prepareImpl() &&
        swigPrepareInternal();
}

bool SwigVariantField::writeImpl() const
{
    return swigWrite();
}


std::string SwigVariantField::swigMembersDeclImpl() const
{
    StringsList memberDefs;
    memberDefs.reserve(m_swigMembers.size());

    for (auto* m : m_swigMembers) {
        memberDefs.push_back(m->swigClassDecl());
    }

    memberDefs.push_back(swigHandlerDeclInternal());

    return util::strListToString(memberDefs, "\n", "\n");
}

std::string SwigVariantField::swigValueAccDeclImpl() const
{
    return strings::emptyString();
}

std::string SwigVariantField::swigExtraPublicFuncsDeclImpl() const
{
    StringsList accFuncs;
    accFuncs.reserve(m_swigMembers.size());

    auto& gen = SwigGenerator::cast(generator());
    for (auto* m : m_swigMembers) {
        static const std::string Templ = {
            "#^#CLASS_NAME#$#& initField_#^#ACC_NAME#$#();\n"
            "#^#CLASS_NAME#$#& accessField_#^#ACC_NAME#$#();\n"
        };

        util::ReplacementMap repl = {
            {"CLASS_NAME", gen.swigClassName(m->field())},
            {"ACC_NAME", comms::accessName(m->field().dslObj().name())}
        };

        accFuncs.push_back(util::processTemplate(Templ, repl));
    }

    static const std::string Templ = 
        "#^#SIZE_T#$# currentField() const;\n"
        "void selectField(#^#SIZE_T#$# idx);\n"
        "void reset();\n"
        "#^#SIZE_T#$# totalFields() const;\n"
        "void currentFieldExec(#^#CLASS_NAME#$#_Handler& handler);\n\n"
        "#^#MEMBERS#$#\n";

    util::ReplacementMap repl = {
        {"SIZE_T", gen.swigConvertCppType("std::size_t")},
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"MEMBERS", util::strListToString(accFuncs, "\n", "")}
    };

    return util::processTemplate(Templ, repl);
}

std::string SwigVariantField::swigExtraPublicFuncsCodeImpl() const
{
    StringsList accFuncs;
    accFuncs.reserve(m_swigMembers.size());

    auto& gen = SwigGenerator::cast(generator());
    for (auto* m : m_swigMembers) {
        static const std::string Templ = {
            "#^#CLASS_NAME#$#& initField_#^#ACC_NAME#$#() { return static_cast<#^#CLASS_NAME#$#&>(Base::initField_#^#ACC_NAME#$#()); }\n"
            "#^#CLASS_NAME#$#& accessField_#^#ACC_NAME#$#() { return static_cast<#^#CLASS_NAME#$#&>(Base::accessField_#^#ACC_NAME#$#()); }\n"
        };

        util::ReplacementMap repl = {
            {"CLASS_NAME", gen.swigClassName(m->field())},
            {"ACC_NAME", comms::accessName(m->field().dslObj().name())}
        };

        accFuncs.push_back(util::processTemplate(Templ, repl));
    }

    StringsList cases;
    cases.reserve(m_swigMembers.size());
    for (auto idx = 0U; idx < m_swigMembers.size(); ++idx) {
        static const std::string Templ = 
            "case #^#IDX#$#: handler.handle_#^#ACC_NAME#$#(accessField_#^#ACC_NAME#$#()); break;\n";

        util::ReplacementMap repl = {
            {"ACC_NAME", comms::accessName(m_swigMembers[idx]->field().dslObj().name())},
            {"IDX", util::numToString(idx)}
        };

        cases.push_back(util::processTemplate(Templ, repl));        
    }

    static const std::string Templ = 
        "#^#MEMBERS#$#\n"
        "#^#SIZE_T#$# totalFields() const\n"
        "{\n"
        "    return static_cast<#^#SIZE_T#$#>(std::tuple_size<typename Base::Members>::value);\n"
        "}\n\n"
        "void currentFieldExec(#^#CLASS_NAME#$#_Handler& handler)\n"
        "{\n"
        "    switch(currentField()) {\n"
        "    #^#CASES#$#\n"
        "    };\n"
        "}\n"
        ;

    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"MEMBERS", util::strListToString(accFuncs, "\n", "")},
        {"CASES", util::strListToString(cases, "", "")},
        {"SIZE_T", gen.swigConvertCppType("std::size_t")},
    };

    return util::processTemplate(Templ, repl);
}

void SwigVariantField::swigAddDefImpl(StringsList& list) const
{
    static const std::string Templ = 
        "%feature(\"director\") #^#CLASS_NAME#$#_Handler;";

    auto& gen = SwigGenerator::cast(generator());
    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
    };

    list.push_back(util::processTemplate(Templ, repl));

    for (auto* m : m_swigMembers) {
        m->swigAddDef(list);
    }    
}

void SwigVariantField::swigAddMembersCodeImpl(StringsList& list) const
{
    for (auto* m : m_swigMembers) {
        m->swigAddCode(list);
    }    

    swigAddHandlerCodeInternal(list);
}

bool SwigVariantField::swigPrepareInternal()
{
    m_swigMembers = swigTransformFieldsList(members());
    return true;
}

std::string SwigVariantField::swigHandlerDeclInternal() const
{
    auto& gen = SwigGenerator::cast(generator());
    StringsList accessFuncs;
    for (auto* m : m_swigMembers) {
        static const std::string Templ = 
            "virtual void handle_#^#ACC_NAME#$#(#^#CLASS_NAME#$#& field);\n";

        util::ReplacementMap repl = {
            {"ACC_NAME", comms::accessName(m->field().dslObj().name())},
            {"CLASS_NAME", gen.swigClassName(m->field())}
        };

        accessFuncs.push_back(util::processTemplate(Templ, repl));
    }    

    static const std::string Templ = 
        "struct #^#CLASS_NAME#$#_Handler\n"
        "{\n"
        "    virtual ~#^#CLASS_NAME#$#_Handler();\n\n"
        "    #^#ACCESS_FUNCS#$#\n"
        "};\n"
    ;

    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"ACCESS_FUNCS", util::strListToString(accessFuncs, "", "")},
    };

    return util::processTemplate(Templ, repl);
}

void SwigVariantField::swigAddHandlerCodeInternal(StringsList& list) const
{
    auto& gen = SwigGenerator::cast(generator());
    StringsList accessFuncs;
    for (auto* m : m_swigMembers) {
        static const std::string Templ = 
            "virtual void handle_#^#ACC_NAME#$#(#^#CLASS_NAME#$#& field) { static_cast<void>(field); }\n";

        util::ReplacementMap repl = {
            {"ACC_NAME", comms::accessName(m->field().dslObj().name())},
            {"CLASS_NAME", gen.swigClassName(m->field())}
        };

        accessFuncs.push_back(util::processTemplate(Templ, repl));
    }    

    static const std::string Templ = 
        "struct #^#CLASS_NAME#$#_Handler\n"
        "{\n"
        "    virtual ~#^#CLASS_NAME#$#_Handler() = default;\n\n"
        "    #^#ACCESS_FUNCS#$#\n"
        "};\n"
    ;

    util::ReplacementMap repl = {
        {"CLASS_NAME", gen.swigClassName(*this)},
        {"ACCESS_FUNCS", util::strListToString(accessFuncs, "", "")},
    };

    list.push_back(util::processTemplate(Templ, repl));
}

} // namespace commsdsl2swig
