//
// Copyright 2022 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "EmscriptenVariantField.h"

#include "EmscriptenGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace comms = commsdsl::gen::comms;
namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2emscripten
{

EmscriptenVariantField::EmscriptenVariantField(EmscriptenGenerator& generator, commsdsl::parse::ParseField dslObj, commsdsl::gen::GenElem* parent) : 
    Base(generator, dslObj, parent),
    EmscriptenBase(static_cast<Base&>(*this))
{
}

bool EmscriptenVariantField::genPrepareImpl()
{
    if (!Base::genPrepareImpl()) {
        return false;
    }

    emscriptenAssignMembers(genMembers());
    return true;
}

bool EmscriptenVariantField::genWriteImpl() const
{
    return emscriptenWrite();
}

std::string EmscriptenVariantField::emscriptenHeaderExtraCodePrefixImpl() const
{
    util::GenStringsList funcs;
    auto& gen = EmscriptenGenerator::cast(genGenerator());
    for (auto* m : emscriptenMembers()) {
        static const std::string MemTempl = 
            "virtual void handle_#^#NAME#$#(#^#MEM_CLASS#$#* field);\n";

        util::ReplacementMap memRepl = {
            {"NAME", comms::genAccessName(m->field().genParseObj().parseName())},
            {"MEM_CLASS", gen.emscriptenClassName(m->field())},
        };

        funcs.push_back(util::genProcessTemplate(MemTempl, memRepl));
    }

    static const std::string Templ = 
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "public:\n"
        "   #^#CLASS_NAME#$#() = default;\n"
        "   virtual ~#^#CLASS_NAME#$#() = default;\n\n"
        "   #^#FUNCS#$#\n"
        "};\n";

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenHandlerClassInternal()},
        {"FUNCS", util::genStrListToString(funcs, "", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenVariantField::emscriptenHeaderValueAccImpl() const
{
    return strings::genEmptyString();
}

std::string EmscriptenVariantField::emscriptenHeaderExtraPublicFuncsImpl() const
{
    auto& membersList = emscriptenMembers();
    util::GenStringsList cases;
    for (auto idx = 0U; idx < membersList.size(); ++idx) {
        static const std::string MemTempl = 
            "case #^#IDX#$#: handler.handle_#^#NAME#$#(accessField_#^#NAME#$#()); break;";

        util::ReplacementMap memRepl = {
            {"IDX", util::genNumToString(idx)},
            {"NAME", comms::genAccessName(membersList[idx]->field().genParseObj().parseName())},
        };

        cases.push_back(util::genProcessTemplate(MemTempl, memRepl));
    }

    static const std::string Templ = 
        "#^#MEMBERS_ACC#$#\n"
        "std::size_t currentField() const\n"
        "{\n"
        "    return Base::currentField();\n"
        "}\n\n"
        "void selectField(std::size_t idx)\n"
        "{\n"
        "    Base::selectField(idx);\n"
        "}\n\n"        
        "void reset()\n"
        "{\n"
        "    Base::reset();\n"
        "}\n\n" 
        "std::size_t totalFields() const\n"
        "{\n"
        "    return static_cast<unsigned long>(std::tuple_size<typename Base::Members>::value);\n"
        "}\n\n"        
        "void currentFieldExec(#^#HANDLER#$#& handler)\n"
        "{\n"
        "    switch (currentField()) {\n"
        "    #^#CASES#$#\n"
        "    }\n"
        "}\n";

    util::ReplacementMap repl = {
        {"MEMBERS_ACC", emscriptenHeaderMembersAccessInternal()},
        {"HANDLER", emscriptenHandlerClassInternal()},
        {"CASES", util::genStrListToString(cases, "\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenVariantField::emscriptenSourceExtraCodeImpl() const
{
    const std::string Templ = 
        "#^#HANDLE_FUNCS#$#\n"
        "struct #^#WRAPPER#$# : public emscripten::wrapper<#^#CLASS_NAME#$#>\n"
        "{\n"
        "    EMSCRIPTEN_WRAPPER(#^#WRAPPER#$#);\n\n"
        "    #^#FUNCS#$#\n"
        "};\n\n"
        "EMSCRIPTEN_BINDINGS(#^#CLASS_NAME#$#) {\n"
        "    emscripten::class_<#^#CLASS_NAME#$#>(\"#^#CLASS_NAME#$#\")\n"
        "        .constructor<>()\n"
        "        .allow_subclass<#^#WRAPPER#$#>(\"#^#WRAPPER#$#\")\n"
        "        #^#BINDS#$#\n"
        "        ;\n"
        "}\n";        
        ;

    util::ReplacementMap repl = {
        {"HANDLE_FUNCS", emscriptenSourceHandleFuncsInternal()},
        {"WRAPPER", emscriptenHandlerWrapperClassInternal()},
        {"CLASS_NAME", emscriptenHandlerClassInternal()},
        {"FUNCS", emscriptenSourceWrapperFuncsInternal()},
        {"BINDS", emscriptenSourceWrapperBindsInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenVariantField::emscriptenSourceBindValueAccImpl() const
{
    return strings::genEmptyString();
}

std::string EmscriptenVariantField::emscriptenSourceBindFuncsImpl() const
{
    util::GenStringsList access;
    for (auto* m : emscriptenMembers()) {
        static const std::string MemTempl = 
            ".function(\"initField_#^#NAME#$#\", &#^#CLASS_NAME#$#::initField_#^#NAME#$#, emscripten::allow_raw_pointers())\n"
            ".function(\"accessField_#^#NAME#$#\", &#^#CLASS_NAME#$#::accessField_#^#NAME#$#, emscripten::allow_raw_pointers())";

        util::ReplacementMap memRepl = {
            {"CLASS_NAME", emscriptenBindClassName()},
            {"NAME", comms::genAccessName(m->field().genParseObj().parseName())},
        };

        access.push_back(util::genProcessTemplate(MemTempl, memRepl));
    }

    static const std::string Templ = 
        "#^#MEMBERS_ACC#$#\n"
        ".function(\"currentField\", &#^#CLASS_NAME#$#::currentField)\n"
        ".function(\"selectField\", &#^#CLASS_NAME#$#::selectField)\n"
        ".function(\"reset\", &#^#CLASS_NAME#$#::reset)\n"
        ".function(\"totalFields\", &#^#CLASS_NAME#$#::totalFields)\n"
        ".function(\"currentFieldExec\", &#^#CLASS_NAME#$#::currentFieldExec)"
        ;

    util::ReplacementMap repl = {
        {"MEMBERS_ACC", util::genStrListToString(access, "\n", "")},
        {"CLASS_NAME", emscriptenBindClassName()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenVariantField::emscriptenHeaderMembersAccessInternal() const
{
    auto& gen = EmscriptenGenerator::cast(genGenerator());
    util::GenStringsList fields;
    for (auto* f : emscriptenMembers()) {
        static const std::string Templ = 
            "#^#FIELD_CLASS#$#* initField_#^#NAME#$#()\n"
            "{\n"
            "    return static_cast<#^#FIELD_CLASS#$#*>(&Base::initField_#^#NAME#$#());\n"
            "}\n\n"
            "#^#FIELD_CLASS#$#* accessField_#^#NAME#$#()\n"
            "{\n"
            "    return static_cast<#^#FIELD_CLASS#$#*>(&Base::accessField_#^#NAME#$#());\n"
            "}\n";            

        util::ReplacementMap repl = {
            {"FIELD_CLASS", gen.emscriptenClassName(f->field())},
            {"NAME", comms::genAccessName(f->field().genParseObj().parseName())},
        };

        fields.push_back(util::genProcessTemplate(Templ, repl));
    }

    return util::genStrListToString(fields, "\n", "");
}

std::string EmscriptenVariantField::emscriptenHandlerClassInternal() const
{
    auto& gen = EmscriptenGenerator::cast(genGenerator());
    return gen.emscriptenClassName(*this) + "_Handler";
}

std::string EmscriptenVariantField::emscriptenHandlerWrapperClassInternal() const
{
    return emscriptenHandlerClassInternal() + "Wrapper";
}

std::string EmscriptenVariantField::emscriptenSourceWrapperFuncsInternal() const
{
    util::GenStringsList funcs;
    auto& gen = EmscriptenGenerator::cast(genGenerator());
    for (auto* m : emscriptenMembers()) {
        static const std::string Templ = 
            "virtual void handle_#^#NAME#$#(#^#MEM_CLASS#$#* field) override\n"
            "{\n"
            "    call<void>(\"handle_#^#NAME#$#\", emscripten::val(field));\n"
            "}\n";

        util::ReplacementMap repl = {
            {"NAME", comms::genAccessName(m->field().genParseObj().parseName())},
            {"MEM_CLASS", gen.emscriptenClassName(m->field())},
        };

        funcs.push_back(util::genProcessTemplate(Templ, repl));
    }

    return util::genStrListToString(funcs, "\n", "");
}

std::string EmscriptenVariantField::emscriptenSourceWrapperBindsInternal() const
{
    util::GenStringsList funcs;
    util::ReplacementMap repl = {
        {"HANDLER", emscriptenHandlerClassInternal()},
    };

    auto& gen = EmscriptenGenerator::cast(genGenerator());
    for (auto* m : emscriptenMembers()) {
        static const std::string Templ = 
            ".function(\"handle_#^#NAME#$#\", emscripten::optional_override([](#^#HANDLER#$#& self, #^#MEM_CLASS#$#* field) { self.#^#HANDLER#$#::handle_#^#NAME#$#(field);}), emscripten::allow_raw_pointers())";

        repl["NAME"] = comms::genAccessName(m->field().genParseObj().parseName());
        repl["MEM_CLASS"] = gen.emscriptenClassName(m->field());
        funcs.push_back(util::genProcessTemplate(Templ, repl));
    }

    return util::genStrListToString(funcs, "\n", "");
}

std::string EmscriptenVariantField::emscriptenSourceHandleFuncsInternal() const
{
    util::GenStringsList funcs;
    auto& gen = EmscriptenGenerator::cast(genGenerator());

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenHandlerClassInternal()},
    };
        
    for (auto* m : emscriptenMembers()) {
        static const std::string Templ = 
            "void #^#CLASS_NAME#$#::handle_#^#NAME#$#(#^#MEM_CLASS#$#* field) { static_cast<void>(field); }";

        repl["NAME"] = comms::genAccessName(m->field().genParseObj().parseName());
        repl["MEM_CLASS"] = gen.emscriptenClassName(m->field());

        funcs.push_back(util::genProcessTemplate(Templ, repl));
    }

    return util::genStrListToString(funcs, "\n", "\n");
}

} // namespace commsdsl2emscripten
