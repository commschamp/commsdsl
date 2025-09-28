//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

const std::string SwigClassName("MsgHandler");

} // namespace

bool SwigMsgHandler::swigWrite(SwigGenerator& generator)
{
    SwigMsgHandler obj(generator);
    return obj.swigWriteInternal();
}

void SwigMsgHandler::swigAddFwdCode(const SwigGenerator& generator, GenStringsList& list)
{
    const std::string Templ =
        "class #^#CLASS_NAME#$#;\n";

    util::GenReplacementMap repl = {
        {"CLASS_NAME", swigClassName(generator)}
    };

    list.push_back(util::genProcessTemplate(Templ, repl));
}

void SwigMsgHandler::swigAddClassCode(const SwigGenerator& generator, GenStringsList& list)
{
    auto* iFace = generator.swigMainInterface();
    assert(iFace != nullptr);
    auto interfaceClassName = generator.swigClassName(*iFace);

    auto allMessages = generator.genGetAllMessagesIdSorted();
    util::GenStringsList handleFuncs;
    handleFuncs.reserve(allMessages.size());

    for (auto* m : allMessages) {
        if (!m->genIsReferenced()) {
            continue;
        }

        static const std::string Templ =
            "void handle(#^#COMMS_MESSAGE#$#<#^#INTERFACE#$##^#PROT_OPTS#$#>& msg)\n"
            "{\n"
            "    static_assert(sizeof(#^#COMMS_MESSAGE#$#<#^#INTERFACE#$##^#PROT_OPTS#$#>) == sizeof(#^#MESSAGE#$#), \"Invalid cast\");\n"
            "    handle_#^#MESSAGE#$#(static_cast<#^#MESSAGE#$#&>(msg));\n"
            "}\n\n"
            "virtual void handle_#^#MESSAGE#$#(#^#MESSAGE#$#& msg)\n"
            "{\n"
            "    handle_#^#INTERFACE#$#(static_cast<#^#INTERFACE#$#&>(msg));\n"
            "}\n";

        util::GenReplacementMap repl = {
            {"MESSAGE", generator.swigClassName(*m)},
            {"COMMS_MESSAGE", comms::genScopeFor(*m, generator)},
            {"INTERFACE", interfaceClassName},
        };

        if (SwigProtocolOptions::swigIsDefined(generator)) {
            repl["PROT_OPTS"] = ", " + SwigProtocolOptions::swigClassName(generator);
        }

        handleFuncs.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "public:\n"
        "    virtual ~#^#CLASS_NAME#$#() = default;\n\n"
        "    #^#HANDLE_FUNCS#$#\n"
        "    void handle(#^#INTERFACE#$#& msg)\n"
        "    {\n"
        "        handle_#^#INTERFACE#$#(msg);\n"
        "    }\n\n"
        "    virtual void handle_#^#INTERFACE#$#(#^#INTERFACE#$#& msg)\n"
        "    {\n"
        "        static_cast<void>(msg);\n"
        "    }\n"
        "};\n";

    util::GenReplacementMap repl = {
        {"CLASS_NAME",swigClassName(generator)},
        {"INTERFACE", interfaceClassName},
        {"HANDLE_FUNCS", util::genStrListToString(handleFuncs, "\n", "")},
    };

    list.push_back(util::genProcessTemplate(Templ, repl));
}

void SwigMsgHandler::swigAddDef(const SwigGenerator& generator, GenStringsList& list)
{
    static const std::string Templ =
        "%feature(\"director\") #^#CLASS_NAME#$#;";

    util::GenReplacementMap repl = {
        {"CLASS_NAME", swigClassName(generator)},
    };

    list.push_back(util::genProcessTemplate(Templ, repl));

    list.push_back(SwigGenerator::swigDefInclude(comms::genRelHeaderForRoot(SwigClassName, generator)));
}

std::string SwigMsgHandler::swigClassName(const SwigGenerator& generator)
{
    return generator.swigScopeNameForRoot(SwigClassName);
}

bool SwigMsgHandler::swigWriteInternal() const
{
    auto filePath = comms::genHeaderPathRoot(SwigClassName, m_swigGenerator);
    m_swigGenerator.genLogger().genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_swigGenerator.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_swigGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ =
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#^#CLASS#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", SwigGenerator::swigFileGeneratedComment()},
        {"CLASS", swigClassDeclInternal()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_swigGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string SwigMsgHandler::swigClassDeclInternal() const
{
    auto* iFace = m_swigGenerator.swigMainInterface();
    assert(iFace != nullptr);

    auto allMessages = m_swigGenerator.genGetAllMessagesIdSorted();
    util::GenStringsList handleFuncs;
    handleFuncs.reserve(allMessages.size());

    for (auto* m : allMessages) {
        if (!m->genIsReferenced()) {
            continue;
        }

        static const std::string Templ =
            "virtual void handle_#^#MESSAGE#$#(#^#MESSAGE#$#& msg);\n";

        util::GenReplacementMap repl = {
            {"MESSAGE", m_swigGenerator.swigClassName(*m)}
        };

        handleFuncs.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "class #^#CLASS_NAME#$#\n"
        "{\n"
        "public:\n"
        "     virtual ~#^#CLASS_NAME#$#();\n\n"
        "     #^#HANDLE_FUNCS#$#\n"
        "     virtual void handle_#^#INTERFACE#$#(#^#INTERFACE#$#& msg);\n"
        "};\n";

    util::GenReplacementMap repl = {
        {"CLASS_NAME", swigClassName(m_swigGenerator)},
        {"INTERFACE", m_swigGenerator.swigClassName(*iFace)},
        {"HANDLE_FUNCS", util::genStrListToString(handleFuncs, "", "")},
        {"SIZE_T", m_swigGenerator.swigConvertCppType("std::size_t")},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2swig