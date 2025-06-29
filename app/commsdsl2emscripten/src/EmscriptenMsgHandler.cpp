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

#include "EmscriptenMsgHandler.h"

#include "EmscriptenGenerator.h"
#include "EmscriptenInterface.h"
#include "EmscriptenMessage.h"
#include "EmscriptenNamespace.h"
#include "EmscriptenProtocolOptions.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2emscripten
{

namespace 
{

const std::string ClassName("MsgHandler");
const std::string WrapperClassName(ClassName + "Wrapper");

} // namespace 
    

EmscriptenMsgHandler::EmscriptenMsgHandler(EmscriptenGenerator& generator, const EmscriptenNamespace& parent) :
    m_generator(generator),
    m_parent(parent)
{
}

bool EmscriptenMsgHandler::emscriptenWrite() const
{
    return 
        emscriptenWriteHeaderInternal() && 
        emscriptenWriteSrcInternal();
}

std::string EmscriptenMsgHandler::emscriptenClassName() const
{
    return m_generator.emscriptenScopeNameForNamespaceMember(ClassName, m_parent);
}

std::string EmscriptenMsgHandler::emscriptenRelHeader() const
{
    return m_generator.emscriptenRelHeaderForNamespaceMember(ClassName, m_parent);
}

void EmscriptenMsgHandler::emscriptenAddSourceFiles(StringsList& sources) const
{
    sources.push_back(m_generator.emscriptenRelSourceForNamespaceMember(ClassName, m_parent));
}

bool EmscriptenMsgHandler::emscriptenWriteHeaderInternal() const
{
    auto filePath = m_generator.emscriptenAbsHeaderForNamespaceMember(ClassName, m_parent);
    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.genCreateDirectory(dirPath)) {
        return false;
    }       

    m_generator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }     

    const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "public:\n"
        "    #^#CLASS_NAME#$#() = default;\n"
        "    virtual ~#^#CLASS_NAME#$#() = default;\n\n"
        "    #^#FUNCS#$#\n"
        "    void handle(#^#INTERFACE#$#& msg) { handle_#^#INTERFACE#$#(&msg); }\n"
        "    virtual void handle_#^#INTERFACE#$#(#^#INTERFACE#$#* msg);\n"
        "};\n"
        ;

    auto* iFace = m_generator.emscriptenMainInterface();
    assert(iFace != nullptr);

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"CLASS_NAME", emscriptenClassName()},
        {"INCLUDES", emscriptenHeaderIncludesInternal()},
        {"COMMS_INTERFACE", comms::genScopeFor(*iFace, m_generator)},
        {"INTERFACE", m_generator.emscriptenClassName(*iFace)},
        {"FUNCS", emscriptenHeaderHandleFuncsInternal()},
    };

    auto str = commsdsl::gen::util::genProcessTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool EmscriptenMsgHandler::emscriptenWriteSrcInternal() const
{
    auto filePath = m_generator.emscriptenAbsSourceForNamespaceMember(ClassName, m_parent);
    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.genCreateDirectory(dirPath)) {
        return false;
    }       

    m_generator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }     

    const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#include \"#^#HEADER#$#\"\n\n"
        "#include <emscripten/bind.h>\n"
        "#include <emscripten/val.h>\n\n"
        "#^#INCLUDES#$#\n"
        "#^#FUNCS#$#\n"
        "#^#WRAPPER#$#\n"
        "#^#BIND#$#\n"
        ;

    util::StringsList includes;
    m_parent.emscriptenAddInputMessageIncludes(includes);
    if (!m_parent.emscriptenHasInput()) {
        auto allNs = m_generator.genGetAllNamespaces();
        for (auto* ns : allNs) {
            EmscriptenNamespace::cast(ns)->emscriptenAddInputMessageIncludes(includes);
        }
    }

    comms::genPrepareIncludeStatement(includes);

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"HEADER", emscriptenRelHeader()},
        {"CLASS_NAME", emscriptenClassName()},
        {"INCLUDES", util::genStrListToString(includes, "\n", "\n")},
        {"FUNCS", emscriptenSourceHandleFuncsInternal()},
        {"WRAPPER", emscriptenSourceWrapperClassInternal()},
        {"BIND", emscriptenSourceBindInternal()},
    };

    auto str = commsdsl::gen::util::genProcessTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string EmscriptenMsgHandler::emscriptenHeaderIncludesInternal() const
{
    auto* iFace = m_generator.emscriptenMainInterface();
    assert(iFace != nullptr);
    auto* parentNs = iFace->genParentNamespace();
    assert(parentNs != nullptr);

    util::StringsList includes = {
        iFace->emscriptenRelHeader()
    };

    auto* emscriptenNs = EmscriptenNamespace::cast(parentNs);
    emscriptenNs->emscriptenAddCommsMessageIncludes(includes);
    emscriptenNs->emscriptenAddInputMessageFwdIncludes(includes);
    
    if (!emscriptenNs->emscriptenHasInput()) {
        auto allNs = m_generator.genGetAllNamespaces();
        for (auto* ns : allNs) {
            EmscriptenNamespace::cast(ns)->emscriptenAddInputMessageFwdIncludes(includes);
        }
    }

    EmscriptenProtocolOptions::emscriptenAddInclude(m_generator, includes);

    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

std::string EmscriptenMsgHandler::emscriptenHeaderHandleFuncsInternal() const
{
    auto* iFace = m_generator.emscriptenMainInterface();
    assert(iFace != nullptr);

    util::ReplacementMap repl = {
        {"INTERFACE", m_generator.emscriptenClassName(*iFace)},
    };

    if (EmscriptenProtocolOptions::emscriptenIsDefined(m_generator)) {
        repl["PROT_OPTS"] = ", " + EmscriptenProtocolOptions::emscriptenClassName(m_generator);
    }
    
    util::StringsList funcs;

    auto allMessages = m_generator.genGetAllMessagesIdSorted();
    funcs.reserve(allMessages.size());
    
    for (auto* m : allMessages) {
        if (!m->genIsReferenced()) {
            continue;
        }

        static const std::string Templ = 
            "void handle(#^#COMMS_CLASS#$#<#^#INTERFACE#$##^#PROT_OPTS#$#>& msg);\n"
            "virtual void handle_#^#CLASS_NAME#$#(#^#CLASS_NAME#$#* msg);\n";

        repl["COMMS_CLASS"] = comms::genScopeFor(*m, m_generator);
        repl["CLASS_NAME"] = m_generator.emscriptenClassName(*m);
        funcs.push_back(util::genProcessTemplate(Templ, repl));
    }

    return util::genStrListToString(funcs, "\n", "\n");
}

std::string EmscriptenMsgHandler::emscriptenSourceHandleFuncsInternal() const
{
    auto* iFace = m_generator.emscriptenMainInterface();
    assert(iFace != nullptr);

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenClassName()},
        {"INTERFACE", m_generator.emscriptenClassName(*iFace)},
    };

    if (EmscriptenProtocolOptions::emscriptenIsDefined(m_generator)) {
        repl["PROT_OPTS"] = ", " + EmscriptenProtocolOptions::emscriptenClassName(m_generator);
    }    

    util::StringsList funcs;

    auto allMessages = m_generator.genGetAllMessagesIdSorted();
    funcs.reserve(allMessages.size() + 1U);
    
    for (auto* m : allMessages) {
        if (!m->genIsReferenced()) {
            continue;
        }

        static const std::string Templ = 
            "void #^#CLASS_NAME#$#::handle(#^#COMMS_CLASS#$#<#^#INTERFACE#$##^#PROT_OPTS#$#>& msg) { handle_#^#MSG_CLASS#$#(static_cast<#^#MSG_CLASS#$#*>(&msg)); }\n"
            "void #^#CLASS_NAME#$#::handle_#^#MSG_CLASS#$#(#^#MSG_CLASS#$#* msg) { handle_#^#INTERFACE#$#(msg); }\n";

        repl["COMMS_CLASS"] = comms::genScopeFor(*m, m_generator);
        repl["MSG_CLASS"] = m_generator.emscriptenClassName(*m);
        funcs.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string InterfaceTempl = 
        "void #^#CLASS_NAME#$#::handle_#^#INTERFACE#$#(#^#INTERFACE#$#* msg) { static_cast<void>(msg); }\n";

    funcs.push_back(util::genProcessTemplate(InterfaceTempl, repl));
    return util::genStrListToString(funcs, "", "\n");
}

std::string EmscriptenMsgHandler::emscriptenSourceWrapperClassInternal() const
{
    const std::string Templ = 
        "struct #^#WRAPPER#$# : public emscripten::wrapper<#^#CLASS_NAME#$#>\n"
        "{\n"
        "    EMSCRIPTEN_WRAPPER(#^#WRAPPER#$#);\n\n"
        "    #^#FUNCS#$#\n"
        "};\n";

    util::ReplacementMap repl = {
        {"WRAPPER", m_generator.emscriptenScopeNameForRoot(WrapperClassName)},
        {"CLASS_NAME", emscriptenClassName()},
        {"FUNCS", emscriptenSourceWrapperFuncsInternal()},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenMsgHandler::emscriptenSourceWrapperFuncsInternal() const
{
    const std::string Templ = 
        "virtual void handle_#^#TYPE#$#(#^#TYPE#$#* msg) override\n"
        "{\n"
        "    call<void>(\"handle_#^#TYPE#$#\", emscripten::val(msg));\n"
        "}\n";


    util::StringsList funcs;

    auto allMessages = m_generator.genGetAllMessagesIdSorted();
    funcs.reserve(allMessages.size() + 1U);
    
    for (auto* m : allMessages) {
        if (!m->genIsReferenced()) {
            continue;
        }

        util::ReplacementMap repl = {
            {"TYPE", m_generator.emscriptenClassName(*m)}
        };

        funcs.push_back(util::genProcessTemplate(Templ, repl));
    }

    auto* iFace = m_generator.emscriptenMainInterface();
    assert(iFace != nullptr);

    util::ReplacementMap repl = {
        {"TYPE", m_generator.emscriptenClassName(*iFace)}
    };    

    funcs.push_back(util::genProcessTemplate(Templ, repl));
    return util::genStrListToString(funcs, "\n", "\n");
}

std::string EmscriptenMsgHandler::emscriptenSourceBindInternal() const
{
    const std::string Templ = 
        "EMSCRIPTEN_BINDINGS(#^#CLASS_NAME#$#) {\n"
        "    emscripten::class_<#^#CLASS_NAME#$#>(\"#^#CLASS_NAME#$#\")\n"
        "        .constructor<>()\n"
        "        .allow_subclass<#^#WRAPPER#$#>(\"#^#WRAPPER#$#\")\n"
        "        #^#FUNCS#$#\n"
        "        ;\n"
        "}\n";

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenClassName()},
        {"WRAPPER", m_generator.emscriptenScopeNameForRoot(WrapperClassName)},
        {"FUNCS", emscriptenSourceBindFuncsInternal()}
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string EmscriptenMsgHandler::emscriptenSourceBindFuncsInternal() const
{
    const std::string Templ = 
        ".function(\"handle_#^#TYPE#$#\", emscripten::optional_override([](#^#HANDLER#$#& self, #^#TYPE#$#* msg) { self.#^#HANDLER#$#::handle_#^#TYPE#$#(msg);}), emscripten::allow_raw_pointers())";

    util::ReplacementMap repl = {
        {"HANDLER", emscriptenClassName()},
    };

    util::StringsList funcs;

    auto allMessages = m_generator.genGetAllMessagesIdSorted();
    funcs.reserve(allMessages.size() + 1U);
    
    for (auto* m : allMessages) {
        if (!m->genIsReferenced()) {
            continue;
        }

        repl["TYPE"] = m_generator.emscriptenClassName(*m);
        funcs.push_back(util::genProcessTemplate(Templ, repl));
    }

    auto* iFace = m_generator.emscriptenMainInterface();
    assert(iFace != nullptr);

    repl["TYPE"] = m_generator.emscriptenClassName(*iFace);
    funcs.push_back(util::genProcessTemplate(Templ, repl));

    return util::genStrListToString(funcs, "\n", "");
}

} // namespace commsdsl2emscripten
