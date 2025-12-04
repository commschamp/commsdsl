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
#include "SwigNamespace.h"
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

SwigMsgHandler::SwigMsgHandler(SwigGenerator& generator, const SwigNamespace& parent) :
    m_swigGenerator(generator),
    m_parent(parent)
{
}

bool SwigMsgHandler::swigWrite() const
{
    if (m_written) {
        return true;
    }

    m_written = true;

    auto filePath = comms::genHeaderPathForNamespaceMember(SwigClassName, m_swigGenerator, m_parent);
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

void SwigMsgHandler::swigAddFwdCode(GenStringsList& list) const
{
    if (m_fwdAdded) {
        return;
    }

    m_fwdAdded = true;

    const std::string Templ =
        "class #^#CLASS_NAME#$#;\n";

    util::GenReplacementMap repl = {
        {"CLASS_NAME", swigClassName()}
    };

    list.push_back(util::genProcessTemplate(Templ, repl));
}

void SwigMsgHandler::swigAddClassCode(GenStringsList& list) const
{
    if (m_codeAdded) {
        return;
    }

    m_codeAdded = true;

    auto* iFace = m_parent.swigInterface();
    assert(iFace != nullptr);
    auto interfaceClassName = m_swigGenerator.swigClassName(*iFace);

    auto allMessages = swigMessagesListInternal();
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
            {"MESSAGE", m_swigGenerator.swigClassName(*m)},
            {"COMMS_MESSAGE", comms::genScopeFor(*m, m_swigGenerator)},
            {"INTERFACE", interfaceClassName},
        };

        if (SwigProtocolOptions::swigIsDefined(m_swigGenerator)) {
            repl["PROT_OPTS"] = ", " + SwigProtocolOptions::swigClassName(m_swigGenerator);
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
        {"CLASS_NAME", swigClassName()},
        {"INTERFACE", interfaceClassName},
        {"HANDLE_FUNCS", util::genStrListToString(handleFuncs, "\n", "")},
    };

    list.push_back(util::genProcessTemplate(Templ, repl));
}

void SwigMsgHandler::swigAddDef(GenStringsList& list) const
{
    if (m_defAdded) {
        return;
    }

    m_defAdded = true;

    static const std::string Templ =
        "%feature(\"director\") #^#CLASS_NAME#$#;";

    util::GenReplacementMap repl = {
        {"CLASS_NAME", swigClassName()},
    };

    list.push_back(util::genProcessTemplate(Templ, repl));

    list.push_back(SwigGenerator::swigDefInclude(comms::genRelHeaderForNamespaceMember(SwigClassName, m_swigGenerator, m_parent)));
}

std::string SwigMsgHandler::swigClassName() const
{
    return m_swigGenerator.swigScopeNameForNamespaceMember(SwigClassName, m_parent);
}

std::string SwigMsgHandler::swigClassDeclInternal() const
{
    auto* iFace = m_parent.swigInterface();
    assert(iFace != nullptr);

    auto allMessages = swigMessagesListInternal();
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
        {"CLASS_NAME", swigClassName()},
        {"INTERFACE", m_swigGenerator.swigClassName(*iFace)},
        {"HANDLE_FUNCS", util::genStrListToString(handleFuncs, "", "")},
        {"SIZE_T", m_swigGenerator.swigConvertCppType("std::size_t")},
    };

    return util::genProcessTemplate(Templ, repl);
}

SwigMsgHandler::GenMessagesAccessList SwigMsgHandler::swigMessagesListInternal() const
{
    auto allMessages = m_parent.genGetAllMessagesIdSorted();
    if (allMessages.empty() && m_parent.genName().empty()) {
        allMessages = m_swigGenerator.genCurrentSchema().genGetAllMessagesIdSorted();
    }

    return allMessages;
}

} // namespace commsdsl2swig