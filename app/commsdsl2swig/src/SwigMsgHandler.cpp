//
// Copyright 2019 - 2023 (C). Alex Robenko. All rights reserved.
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

#include "SwigMsgHandler.h"

#include "SwigGenerator.h"
#include "SwigInterface.h"
#include "SwigProtocolOptions.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <limits>

namespace util = commsdsl::gen::util;
namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2swig
{

namespace 
{

const std::string ClassName("MsgHandler");

} // namespace

bool SwigMsgHandler::swigWrite(SwigGenerator& generator)
{
    SwigMsgHandler obj(generator);
    return obj.swigWriteInternal();
}

void SwigMsgHandler::swigAddFwdCode(const SwigGenerator& generator, StringsList& list)
{
    const std::string Templ =  
        "class #^#CLASS_NAME#$#;\n";

    util::ReplacementMap repl = {
        {"CLASS_NAME", swigClassName(generator)}
    };

    list.push_back(util::processTemplate(Templ, repl));
}

void SwigMsgHandler::swigAddClassCode(const SwigGenerator& generator, StringsList& list)
{
    auto* iFace = generator.swigMainInterface();
    assert(iFace != nullptr);
    auto interfaceClassName = generator.swigClassName(*iFace);

    auto allMessages = generator.getAllMessagesIdSorted();
    util::StringsList handleFuncs;
    handleFuncs.reserve(allMessages.size());

    for (auto* m : allMessages) {
        if (!m->isReferenced()) {
            continue;
        }

        static const std::string Templ = 
            "void handle(#^#MESSAGE#$#& msg) { handle_#^#MESSAGE#$#(msg); }\n"
            "virtual void handle_#^#MESSAGE#$#(#^#MESSAGE#$#& msg);\n";

        util::ReplacementMap repl = {
            {"MESSAGE", generator.swigClassName(*m)},
        };

        handleFuncs.push_back(util::processTemplate(Templ, repl));
    }

    static const std::string Templ = 
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "public:\n"
        "     virtual ~#^#CLASS_NAME#$#() = default;\n\n"
        "     #^#HANDLE_FUNCS#$#\n"
        "     void handle(#^#INTERFACE#$#& msg) { handle_#^#INTERFACE#$#(msg); }\n"
        "     virtual void handle_#^#INTERFACE#$#(#^#INTERFACE#$#& msg) { static_cast<void>(msg); }\n"
        "};\n";

    util::ReplacementMap repl = {
        {"CLASS_NAME",swigClassName(generator)},
        {"INTERFACE", interfaceClassName},
        {"HANDLE_FUNCS", util::strListToString(handleFuncs, "\n", "")},
    };

    list.push_back(util::processTemplate(Templ, repl));
}

void SwigMsgHandler::swigAddFuncsCode(const SwigGenerator& generator, StringsList& list)
{
    auto* iFace = generator.swigMainInterface();
    assert(iFace != nullptr);
    auto interfaceClassName = generator.swigClassName(*iFace);

    auto allMessages = generator.getAllMessagesIdSorted();
    list.reserve(list.size() + allMessages.size());

    for (auto* m : allMessages) {
        if (!m->isReferenced()) {
            continue;
        }

        static const std::string Templ = 
            "void #^#CLASS_NAME#$#::handle_#^#MESSAGE#$#(#^#MESSAGE#$#& msg) { handle_#^#INTERFACE#$#(msg); }";

        util::ReplacementMap repl = {
            {"INTERFACE", interfaceClassName},
            {"MESSAGE", generator.swigClassName(*m)},
            {"CLASS_NAME",swigClassName(generator)},
        };

        list.push_back(util::processTemplate(Templ, repl));
    }

    list.push_back(strings::emptyString());
}

void SwigMsgHandler::swigAddDef(const SwigGenerator& generator, StringsList& list)
{
    static const std::string Templ = 
        "%feature(\"director\") #^#CLASS_NAME#$#;";

    util::ReplacementMap repl = {
        {"CLASS_NAME", swigClassName(generator)},
    };    

    list.push_back(util::processTemplate(Templ, repl));

    list.push_back(SwigGenerator::swigDefInclude(comms::relHeaderForRoot(ClassName, generator)));    
}

std::string SwigMsgHandler::swigClassName(const SwigGenerator& generator)
{
    return generator.swigScopeNameForRoot(ClassName);
}

bool SwigMsgHandler::swigWriteInternal() const
{
    auto filePath = comms::headerPathRoot(ClassName, m_generator);
    m_generator.logger().info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.createDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ = 
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#^#CLASS#$#\n"
    ;

    util::ReplacementMap repl = {
        {"GENERATED", SwigGenerator::fileGeneratedComment()},
        {"CLASS", swigClassDeclInternal()},
    };

    stream << util::processTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

std::string SwigMsgHandler::swigClassDeclInternal() const
{
    auto* iFace = m_generator.swigMainInterface();
    assert(iFace != nullptr);

    auto allMessages = m_generator.getAllMessagesIdSorted();
    util::StringsList handleFuncs;
    handleFuncs.reserve(allMessages.size());

    for (auto* m : allMessages) {
        if (!m->isReferenced()) {
            continue;
        }
                
        static const std::string Templ = 
            "virtual void handle_#^#MESSAGE#$#(#^#MESSAGE#$#& msg);\n";

        util::ReplacementMap repl = {
            {"MESSAGE", m_generator.swigClassName(*m)}
        };

        handleFuncs.push_back(util::processTemplate(Templ, repl));
    }

    static const std::string Templ = 
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "public:\n"
        "     virtual ~#^#CLASS_NAME#$#();\n\n"
        "     #^#HANDLE_FUNCS#$#\n"
        "     virtual void handle_#^#INTERFACE#$#(#^#INTERFACE#$#& msg);\n"
        "};\n";

    util::ReplacementMap repl = {
        {"CLASS_NAME", swigClassName(m_generator)},
        {"INTERFACE", m_generator.swigClassName(*iFace)},
        {"HANDLE_FUNCS", util::strListToString(handleFuncs, "", "")},
        {"SIZE_T", m_generator.swigConvertCppType("std::size_t")},
    };

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2swig