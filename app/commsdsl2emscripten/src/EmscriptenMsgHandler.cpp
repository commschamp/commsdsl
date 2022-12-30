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

#include "EmscriptenMsgHandler.h"

#include "EmscriptenAllMessages.h"
#include "EmscriptenGenerator.h"
#include "EmscriptenInterface.h"
#include "EmscriptenMessage.h"
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
    

bool EmscriptenMsgHandler::emscriptenWrite(EmscriptenGenerator& generator)
{
    EmscriptenMsgHandler obj(generator);
    return 
        obj.emscriptenWriteHeaderInternal() && 
        obj.emscriptenWriteSrcInternal();
}

std::string EmscriptenMsgHandler::emscriptenClassName(const EmscriptenGenerator& generator)
{
    return generator.emscriptenScopeNameForRoot(ClassName);
}

std::string EmscriptenMsgHandler::emscriptenRelHeader(const EmscriptenGenerator& generator)
{
    return generator.emscriptenRelHeaderForRoot(ClassName);
}

bool EmscriptenMsgHandler::emscriptenWriteHeaderInternal() const
{
    auto filePath = m_generator.emscriptenAbsHeaderForRoot(ClassName);
    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.createDirectory(dirPath)) {
        return false;
    }       

    m_generator.logger().info("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }     

    const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#^#INCLUDES#$#\n"
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "public:\n"
        "    #^#CLASS_NAME#$#() = default;\n"
        "    virtual ~#^#CLASS_NAME#$#() = default;\n\n"
        "    #^#FUNCS#$#\n"
        "    void handle(#^#COMMS_INTERFACE#$#& msg)\n"
        "    {\n"
        "        handle_#^#INTERFACE#$#(&msg);\n"
        "    }\n\n"
        "    virtual void handle_#^#INTERFACE#$#(#^#INTERFACE#$#* msg);\n"
        "};\n"
        ;

    auto* iFace = m_generator.emscriptenMainInterface();
    assert(iFace != nullptr);

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"CLASS_NAME", emscriptenClassName(m_generator)},
        {"INCLUDES", emscriptenHeaderIncludesInternal()},
        {"COMMS_INTERFACE", comms::scopeFor(*iFace, m_generator)},
        {"INTERFACE", m_generator.emscriptenClassName(*iFace)},
        {"FUNCS", emscriptenHeaderHandleFuncsInternal()},
    };

    auto str = commsdsl::gen::util::processTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool EmscriptenMsgHandler::emscriptenWriteSrcInternal() const
{
    auto filePath = m_generator.emscriptenAbsSourceForRoot(ClassName);
    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.createDirectory(dirPath)) {
        return false;
    }       

    m_generator.logger().info("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }     

    const std::string Templ = 
        "#^#GENERATED#$#\n\n"
        "#include \"#^#HEADER#$#\"\n\n"
        "#include <emscripten/bind.h>\n"
        "#include <emscripten/val.h>\n"
        "#include \"#^#ALL_MESSAGES#$#\"\n\n"
        "#^#FUNCS#$#\n"
        "#^#WRAPPER#$#\n"
        "#^#BIND#$#\n"
        ;

    util::ReplacementMap repl = {
        {"GENERATED", EmscriptenGenerator::fileGeneratedComment()},
        {"HEADER", emscriptenRelHeader(m_generator)},
        {"CLASS_NAME", emscriptenClassName(m_generator)},
        {"ALL_MESSAGES", EmscriptenAllMessages::emscriptenRelHeader(m_generator)},
        {"FUNCS", emscriptenSourceHandleFuncsInternal()},
        {"WRAPPER", emscriptenSourceWrapperClassInternal()},
        {"BIND", emscriptenSourceBindInternal()},
    };

    auto str = commsdsl::gen::util::processTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string EmscriptenMsgHandler::emscriptenHeaderIncludesInternal() const
{
    auto* iFace = m_generator.emscriptenMainInterface();
    assert(iFace != nullptr);

    util::StringsList includes = {
        comms::relHeaderForInput(strings::allMessagesStr(), m_generator),
        EmscriptenAllMessages::emscriptenRelFwdHeader(m_generator),
        iFace->emscriptenRelHeader()
    };

    comms::prepareIncludeStatement(includes);
    return util::strListToString(includes, "\n", "\n");
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

    auto allMessages = m_generator.getAllMessagesIdSorted();
    funcs.reserve(allMessages.size());
    
    for (auto* m : allMessages) {
        if (!m->isReferenced()) {
            continue;
        }

        static const std::string Templ = 
            "void handle(#^#COMMS_CLASS#$#<#^#INTERFACE#$##^#PROT_OPTS#$#>& msg)\n"
            "{\n"
            "    handle_#^#CLASS_NAME#$#(&msg);\n"
            "}\n\n"
            "virtual void handle_#^#CLASS_NAME#$#(#^#CLASS_NAME#$#* msg);\n";

        repl["COMMS_CLASS"] = comms::scopeFor(*m, m_generator);
        repl["CLASS_NAME"] = m_generator.emscriptenClassName(*m);
        funcs.push_back(util::processTemplate(Templ, repl));
    }

    return util::strListToString(funcs, "\n", "\n");
}

std::string EmscriptenMsgHandler::emscriptenSourceHandleFuncsInternal() const
{
    auto* iFace = m_generator.emscriptenMainInterface();
    assert(iFace != nullptr);

    util::ReplacementMap repl = {
        {"CLASS_NAME", emscriptenClassName(m_generator)},
        {"INTERFACE", m_generator.emscriptenClassName(*iFace)},
    };

    util::StringsList funcs;

    auto allMessages = m_generator.getAllMessagesIdSorted();
    funcs.reserve(allMessages.size() + 1U);
    
    for (auto* m : allMessages) {
        if (!m->isReferenced()) {
            continue;
        }

        static const std::string Templ = 
            "void #^#CLASS_NAME#$#::handle_#^#MSG_CLASS#$#(#^#MSG_CLASS#$#* msg) { handle_#^#INTERFACE#$#(msg); }\n";

        repl["MSG_CLASS"] = m_generator.emscriptenClassName(*m);
        funcs.push_back(util::processTemplate(Templ, repl));
    }

    static const std::string InterfaceTempl = 
        "void #^#CLASS_NAME#$#::handle_#^#INTERFACE#$#(#^#INTERFACE#$#* msg) { static_cast<void>(msg); }\n";


    funcs.push_back(util::processTemplate(InterfaceTempl, repl));
    return util::strListToString(funcs, "", "\n");
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
        {"CLASS_NAME", emscriptenClassName(m_generator)},
        {"FUNCS", emscriptenSourceWrapperFuncsInternal()},
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenMsgHandler::emscriptenSourceWrapperFuncsInternal() const
{
    const std::string Templ = 
        "virtual void handle_#^#TYPE#$#(#^#TYPE#$#* msg) override\n"
        "{\n"
        "    call<void>(\"handle_#^#TYPE#$#\", emscripten::val(msg));\n"
        "}\n";


    util::StringsList funcs;

    auto allMessages = m_generator.getAllMessagesIdSorted();
    funcs.reserve(allMessages.size() + 1U);
    
    for (auto* m : allMessages) {
        if (!m->isReferenced()) {
            continue;
        }

        util::ReplacementMap repl = {
            {"TYPE", m_generator.emscriptenClassName(*m)}
        };

        funcs.push_back(util::processTemplate(Templ, repl));
    }

    auto* iFace = m_generator.emscriptenMainInterface();
    assert(iFace != nullptr);

    util::ReplacementMap repl = {
        {"TYPE", m_generator.emscriptenClassName(*iFace)}
    };    

    funcs.push_back(util::processTemplate(Templ, repl));
    return util::strListToString(funcs, "\n", "\n");
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
        {"CLASS_NAME", emscriptenClassName(m_generator)},
        {"WRAPPER", m_generator.emscriptenScopeNameForRoot(WrapperClassName)},
        {"FUNCS", emscriptenSourceBindFuncsInternal()}
    };

    return util::processTemplate(Templ, repl);
}

std::string EmscriptenMsgHandler::emscriptenSourceBindFuncsInternal() const
{
    const std::string Templ = 
        ".function(\"handle_#^#TYPE#$#\", emscripten::optional_override([](#^#HANDLER#$#& self, #^#TYPE#$#* msg) { return self.#^#HANDLER#$#::handle_#^#TYPE#$#(msg);}), emscripten::allow_raw_pointers())";

    util::ReplacementMap repl = {
        {"HANDLER", emscriptenClassName(m_generator)},
    };

    util::StringsList funcs;

    auto allMessages = m_generator.getAllMessagesIdSorted();
    funcs.reserve(allMessages.size() + 1U);
    
    for (auto* m : allMessages) {
        if (!m->isReferenced()) {
            continue;
        }

        repl["TYPE"] = m_generator.emscriptenClassName(*m);
        funcs.push_back(util::processTemplate(Templ, repl));
    }

    auto* iFace = m_generator.emscriptenMainInterface();
    assert(iFace != nullptr);

    repl["TYPE"] = m_generator.emscriptenClassName(*iFace);
    funcs.push_back(util::processTemplate(Templ, repl));

    return util::strListToString(funcs, "\n", "");
}

} // namespace commsdsl2emscripten
