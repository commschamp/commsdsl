//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "CMsgHandler.h"

#include "CGenerator.h"
#include "CInterface.h"
#include "CMessage.h"
#include "CNamespace.h"
#include "CProtocolOptions.h"
#include "CSchema.h"

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

namespace commsdsl2c
{

namespace
{

const std::string CHandlerName("MsgHandler");

} // namespace

CMsgHandler::CMsgHandler(CGenerator& generator, const CNamespace& parent) :
    m_cGenerator(generator),
    m_parent(parent)
{
}

std::string CMsgHandler::cRelHeader() const
{
    return m_cGenerator.cRelHeaderForNamespaceMember(CHandlerName, m_parent);
}

std::string CMsgHandler::cRelCommsHeader() const
{
    return m_cGenerator.cRelCommsHeaderForNamespaceMember(CHandlerName, m_parent);
}

std::string CMsgHandler::cRelSource() const
{
    return m_cGenerator.cRelSourceForNamespaceMember(CHandlerName, m_parent);
}

std::string CMsgHandler::cName() const
{
    return m_parent.cPrefixName() + '_' + CHandlerName;
}

std::string CMsgHandler::cCommsTypeName() const
{
    return cName() + strings::genCommsNameSuffixStr();
}

void CMsgHandler::cAddSourceFiles(GenStringsList& sources) const
{
    sources.push_back(cRelSource());
}

bool CMsgHandler::cWrite() const
{
    return
        cWriteHeaderInternal() &&
        cWriteSourceInternal() &&
        cWriteCommsHeaderInternal();
}

bool CMsgHandler::cWriteHeaderInternal() const
{
    auto filePath = m_cGenerator.cAbsHeaderForNamespaceMember(CHandlerName, m_parent);

    m_cGenerator.genLogger().genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_cGenerator.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_cGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ =
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#^#CPP_GUARD_BEGIN#$#\n\n"
        "// Forward declaration of the messages.\n"
        "#^#FWD#$#\n"
        "/// @brief Handler for messages dispatching.\n"
        "typedef struct\n"
        "{\n"
        "    #^#FUNCS#$#\n"
        "} #^#NAME#$#;\n\n"
        "#^#CPP_GUARD_END#$#\n";
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"FUNCS", cHeaderFuncsInternal()},
        {"FWD", cHeaderFwdInternal()},
        {"CPP_GUARD_BEGIN", CGenerator::cCppGuardBegin()},
        {"CPP_GUARD_END", CGenerator::cCppGuardEnd()},
        {"NAME", cName()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_cGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool CMsgHandler::cWriteSourceInternal() const
{
    auto filePath = m_cGenerator.cAbsSourceForNamespaceMember(CHandlerName, m_parent);

    m_cGenerator.genLogger().genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_cGenerator.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_cGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ =
        "#^#GENERATED#$#\n"
        "#include \"#^#COMMS_HEADER#$#\"\n\n"
        "#^#INCLUDES#$#\n"
        "#^#FUNCS#$#\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"COMMS_HEADER", cRelCommsHeader()},
        {"INCLUDES", cCommsSourceIncludesInternal()},
        {"FUNCS", cCommsSourceFuncsInternal()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_cGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool CMsgHandler::cWriteCommsHeaderInternal() const
{
    auto filePath = m_cGenerator.cAbsCommsHeaderForNamespaceMember(CHandlerName, m_parent);

    m_cGenerator.genLogger().genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_cGenerator.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_cGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ =
        "#^#GENERATED#$#\n"
        "#pragma once\n\n"
        "#^#INCLUDES#$#\n"
        "class #^#COMMS_NAME#$#\n"
        "{\n"
        "public:\n"
        "    explicit #^#COMMS_NAME#$#(#^#NAME#$#& handler, void* userData) : m_handler(handler), m_userData(userData) {}\n"
        "\n"
        "    #^#FUNCS#$#\n"
        "private:\n"
        "    #^#NAME#$#& m_handler;\n"
        "    void* m_userData = nullptr;\n"
        "};\n"
    ;

    util::GenReplacementMap repl = {
        {"GENERATED", CGenerator::cFileGeneratedComment()},
        {"INCLUDES", cCommsHeaderIncludesInternal()},
        {"NAME", cName()},
        {"COMMS_NAME", cCommsTypeName()},
        {"FUNCS", cCommsHeaderFuncsInternal()},
    };

    stream << util::genProcessTemplate(Templ, repl, true);
    stream.flush();
    if (!stream.good()) {
        m_cGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string CMsgHandler::cHeaderFuncsInternal() const
{
    auto allMessages = cMessagesListInternal();
    util::GenStringsList funcs;
    funcs.reserve(allMessages.size() + 1U);
    for (auto* m : allMessages) {
        static const std::string Templ =
            "/// @brief Handle the @ref #^#NAME#$# message.\n"
            "void (*handle_#^#NAME#$#)(struct #^#NAME#$#_* msg, void* userData);\n"
        ;
        auto* cMsg = CMessage::cCast(m);
        util::GenReplacementMap repl = {
            {"NAME", cMsg->cName()},
        };

        funcs.push_back(util::genProcessTemplate(Templ, repl));
    }

    auto* cInterface = m_parent.cInterface();
    assert(cInterface != nullptr);

    static const std::string Templ =
        "/// @brief Handle all other messages if their appropriate handling function is not assigned.\n"
        "void (*handle_#^#NAME#$#)(struct #^#NAME#$#_* msg, void* userData);"
    ;

    util::GenReplacementMap repl = {
        {"NAME", cInterface->cName()},
    };

    funcs.push_back(util::genProcessTemplate(Templ, repl));

    return util::genStrListToString(funcs, "\n", "");
}

std::string CMsgHandler::cHeaderFwdInternal() const
{
    static const std::string Templ =
        "struct #^#NAME#$#_;\n"
    ;

    auto allMessages = cMessagesListInternal();
    util::GenStringsList fwd;
    fwd.reserve(allMessages.size() + 1U);
    for (auto* m : allMessages) {

        auto* cMsg = CMessage::cCast(m);
        util::GenReplacementMap repl = {
            {"NAME", cMsg->cName()},
        };

        fwd.push_back(util::genProcessTemplate(Templ, repl));
    }

    auto* cInterface = m_parent.cInterface();
    assert(cInterface != nullptr);

    util::GenReplacementMap repl = {
        {"NAME", cInterface->cName()},
    };

    fwd.push_back(util::genProcessTemplate(Templ, repl));
    return util::genStrListToString(fwd, "", "\n");
}

std::string CMsgHandler::cCommsHeaderIncludesInternal() const
{
    auto* cInterface = m_parent.cInterface();
    assert(cInterface != nullptr);    

    util::GenStringsList includes {
        cRelHeader(),
        cInterface->cRelCommsHeader(),
        CProtocolOptions::cRelHeader(m_cGenerator),
    };

    auto allMessages = cMessagesListInternal();
    for (auto* m : allMessages) {
        includes.push_back(comms::genRelHeaderPathFor(*m, m_cGenerator));
    }    

    comms::genPrepareIncludeStatement(includes);
    return util::genStrListToString(includes, "\n", "\n");
}

std::string CMsgHandler::cCommsHeaderFuncsInternal() const
{
    auto allMessages = cMessagesListInternal();
    util::GenStringsList funcs;


    auto* cInterface = m_parent.cInterface();
    assert(cInterface != nullptr);    

    funcs.reserve(funcs.size() + 1U);
    for (auto* m : allMessages) {

        static const std::string Templ =
            "void handle(::#^#COMMS_SCOPE#$#<#^#INTERFACE_COMMS_NAME#$#, #^#OPTS#$#>& msg);"
            ;        
        util::GenReplacementMap repl = {
            {"INTERFACE_COMMS_NAME", cInterface->cCommsTypeName()},
            {"COMMS_SCOPE", ::comms::genScopeFor(*m, m_cGenerator)},
            {"OPTS", CProtocolOptions::cName(m_cGenerator)},
        };

        funcs.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "void handle(#^#COMMS_NAME#$#& msg);"
        ;

    util::GenReplacementMap repl = {
        {"COMMS_NAME", cInterface->cCommsTypeName()},
    };

    funcs.push_back(util::genProcessTemplate(Templ, repl));
    return util::genStrListToString(funcs, "\n", "\n");
}

std::string CMsgHandler::cCommsSourceIncludesInternal() const
{
    auto allMessages = cMessagesListInternal();
    util::GenStringsList includes {
        cRelHeader()
    };

    includes.reserve(includes.size() + allMessages.size() + 1U);
    for (auto* m : allMessages) {
        auto* cMsg = CMessage::cCast(m);
        includes.push_back(cMsg->cRelCommsHeader());
    }

    auto* cInterface = m_parent.cInterface();
    assert(cInterface != nullptr);
    includes.push_back(cInterface->cRelCommsHeader());
    comms::genPrepareIncludeStatement(includes);

    return util::genStrListToString(includes, "\n", "\n");
}

std::string CMsgHandler::cCommsSourceFuncsInternal() const
{
    auto allMessages = cMessagesListInternal();
    util::GenStringsList funcs;

    auto* cInterface = m_parent.cInterface();
    assert(cInterface != nullptr);

    funcs.reserve(funcs.size() + 1U);
    for (auto* m : allMessages) {
        static const std::string Templ =
            "void #^#HANDLER#$#::handle(::#^#COMMS_SCOPE#$#<#^#INTERFACE_COMMS_NAME#$#, #^#OPTS#$#>& msg)\n"
            "{\n"
            ""
            "    if (m_handler.handle_#^#NAME#$# != nullptr) {\n"
            "        m_handler.handle_#^#NAME#$#(toMessageHandle(&msg), m_userData);\n"
            "        return;\n"
            "    }\n\n"
            "    handle(static_cast<#^#INTERFACE_COMMS_NAME#$#&>(msg));\n"
            "}\n"
            ;

        auto* cMsg = CMessage::cCast(m);
        util::GenReplacementMap repl = {
            //{"COMMS_NAME", cMsg->cCommsTypeName()},
            {"NAME", cMsg->cName()},
            {"INTERFACE_COMMS_NAME", cInterface->cCommsTypeName()},
            {"HANDLER", cCommsTypeName()},
            {"COMMS_SCOPE", ::comms::genScopeFor(*m, m_cGenerator)},
            {"OPTS", CProtocolOptions::cName(m_cGenerator)},
        };

        funcs.push_back(util::genProcessTemplate(Templ, repl));
    }

    static const std::string Templ =
        "void #^#HANDLER#$#::handle(#^#COMMS_NAME#$#& msg)\n"
        "{\n"
        "    if (m_handler.handle_#^#NAME#$# != nullptr) {\n"
        "        m_handler.handle_#^#NAME#$#(toInterfaceHandle(&msg), m_userData);\n"
        "    }\n"
        "}\n"
        ;

    util::GenReplacementMap repl = {
        {"COMMS_NAME", cInterface->cCommsTypeName()},
        {"NAME", cInterface->cName()},
        {"HANDLER", cCommsTypeName()},
    };

    funcs.push_back(util::genProcessTemplate(Templ, repl));
    return util::genStrListToString(funcs, "\n", "");
}

CMsgHandler::GenMessagesAccessList CMsgHandler::cMessagesListInternal() const
{
    auto allMessages = m_parent.genGetAllMessages();
    if (allMessages.empty() && m_parent.genName().empty()) {
        allMessages = m_cGenerator.genCurrentSchema().genGetAllMessages();
    }

    return allMessages;
}

} // namespace commsdsl2c