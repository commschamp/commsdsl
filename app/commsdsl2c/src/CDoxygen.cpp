//
// Copyright 2025 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

#include "CDoxygen.h"

#include "CErrorStatus.h"
#include "CGenerator.h"
#include "CMessage.h"
#include "CSchema.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2c
{

namespace
{

// const std::string CServerInputPrefixStr = "ServerInput";
// const std::string CClientInputPrefixStr = "ClientInput";
// const std::string CServerInputMessagesStr = CServerInputPrefixStr + "Messages";
// const std::string CClientInputMessagesStr = CClientInputPrefixStr + "Messages";

bool cWriteFileInternal(const std::string& name, const std::string& str, CGenerator& generator)
{
    auto filePath = generator.cAbsPathForDoc(name);
    generator.genLogger().genInfo("Generating " + filePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!generator.genCreateDirectory(dirPath)) {
        return false;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        generator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    stream << str;
    stream.flush();

    if (!stream.good()) {
        generator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }
    return true;
}

} // namespace

bool CDoxygen::cWrite(CGenerator& generator)
{
    CDoxygen obj(generator);
    return obj.cWriteInternal();
}

CDoxygen::CDoxygen(CGenerator& generator) :
    m_cGenerator(generator)
{
    cPrepareInternal();
}

void CDoxygen::cPrepareInternal()
{
    auto frames = m_cGenerator.genGetAllFrames();
    m_frames.reserve(frames.size());
    for (auto* f : frames) {
        auto* cFrame = CFrame::cCast(f);
        if (!cFrame->cIsValidFrame()) {
            continue;
        }

        m_frames.push_back(cFrame);
    }
}

bool CDoxygen::cWriteInternal() const
{
    return
        cWriteConfInternal() &&
        cWriteLayoutInternal() &&
        cWriteMainpageInternal();
}

bool CDoxygen::cWriteConfInternal() const
{
    const std::string FileName = "doxygen.conf";
    const std::string Templ =
        "PROJECT_NAME           = \"#^#PROJ_NAME#$#_c\"\n"
        "PROJECT_BRIEF          = \"\\\"C\\\" interface for the \\\"#^#PROJ_NAME#$#\\\" protocol.\"\n"
        "OUTPUT_DIRECTORY       = \n"
        "OUTPUT_LANGUAGE        = English\n"
        "INLINE_INHERITED_MEMB  = YES\n"
        "BUILTIN_STL_SUPPORT    = YES\n"
        "NUM_PROC_THREADS       = 0\n"
        "HIDE_UNDOC_MEMBERS     = YES\n"
        "HIDE_UNDOC_CLASSES     = YES\n"
        "CASE_SENSE_NAMES       = YES\n"
        "FORCE_LOCAL_INCLUDES   = YES\n"
        "INLINE_INFO            = NO\n"
        "SORT_BRIEF_DOCS        = YES\n"
        "SORT_MEMBERS_CTORS_1ST = YES\n"
        "SORT_BY_SCOPE_NAME     = YES\n"
        "LAYOUT_FILE            = doc/layout.xml\n"
        "QUIET                  = YES\n"
        "WARN_IF_UNDOCUMENTED   = NO\n"
        "WARN_IF_DOC_ERROR      = YES\n"
        "WARN_NO_PARAMDOC       = YES\n"
        "WARN_AS_ERROR          = YES\n"
        "INPUT                  = doc include\n"
        "RECURSIVE              = YES\n"
        "EXCLUDE                = \n"
        "EXCLUDE_SYMBOLS        = *details\n"
        "GENERATE_LATEX         = NO\n"
        "PREDEFINED             = \n"
        "HAVE_DOT               = NO\n"
        "EXTRACT_ALL            = YES\n"
        "#^#APPEND#$#\n"
        "\n";

    util::GenReplacementMap repl = {
        {"PROJ_NAME", m_cGenerator.genProtocolSchema().genDisplayName()},
        {"APPEND", m_cGenerator.genReadScriptCodeInjectCode(m_cGenerator.cInputRelPathForDoc(FileName) + strings::genAppendFileSuffixStr(), "Append here")},
    };

    return cWriteFileInternal(FileName, util::genProcessTemplate(Templ, repl), m_cGenerator);
}

bool CDoxygen::cWriteLayoutInternal() const
{
    const std::string FileName = "layout.xml";
    const std::string Templ =
        "<doxygenlayout version=\"1.0\">\n"
        "<navindex>\n"
        "    <tab type=\"mainpage\" visible=\"yes\" title=\"\"/>\n"
        "    <tab type=\"pages\" visible=\"yes\" title=\"\" intro=\"\"/>\n"
        "    <tab type=\"modules\" visible=\"yes\" title=\"\" intro=\"\"/>\n"
        "    <tab type=\"namespaces\" visible=\"yes\" title=\"\">\n"
        "    <tab type=\"namespacelist\" visible=\"yes\" title=\"\" intro=\"\"/>\n"
        "    <tab type=\"namespacemembers\" visible=\"yes\" title=\"\" intro=\"\"/>\n"
        "    </tab>\n"
        "    <tab type=\"classes\" visible=\"yes\" title=\"\">\n"
        "    <tab type=\"classlist\" visible=\"yes\" title=\"\" intro=\"\"/>\n"
        "    <tab type=\"classindex\" visible=\"$ALPHABETICAL_INDEX\" title=\"\"/>\n"
        "    <tab type=\"hierarchy\" visible=\"yes\" title=\"\" intro=\"\"/>\n"
        "    <tab type=\"classmembers\" visible=\"yes\" title=\"\" intro=\"\"/>\n"
        "    </tab>\n"
        "    <tab type=\"files\" visible=\"yes\" title=\"\">\n"
        "    <tab type=\"filelist\" visible=\"yes\" title=\"\" intro=\"\"/>\n"
        "    <tab type=\"globals\" visible=\"yes\" title=\"\" intro=\"\"/>\n"
        "    </tab>\n"
        "    <tab type=\"examples\" visible=\"yes\" title=\"\" intro=\"\"/>\n"
        "</navindex>\n\n"
        "<!-- Layout definition for a class page -->\n"
        "<class>\n"
        "    <includes visible=\"$SHOW_INCLUDE_FILES\"/>\n"
        "    <briefdescription visible=\"no\"/>\n"
        "    <detaileddescription title=\"\"/>\n"
        "    <inheritancegraph visible=\"$CLASS_GRAPH\"/>\n"
        "    <collaborationgraph visible=\"$COLLABORATION_GRAPH\"/>\n"
        "    <memberdecl>\n"
        "    <nestedclasses visible=\"yes\" title=\"\"/>\n"
        "    <publictypes title=\"\"/>\n"
        "    <services title=\"\"/>\n"
        "    <interfaces title=\"\"/>\n"
        "    <publicslots title=\"\"/>\n"
        "    <signals title=\"\"/>\n"
        "    <publicmethods title=\"\"/>\n"
        "    <publicstaticmethods title=\"\"/>\n"
        "    <publicattributes title=\"\"/>\n"
        "    <publicstaticattributes title=\"\"/>\n"
        "    <protectedtypes title=\"\"/>\n"
        "    <protectedslots title=\"\"/>\n"
        "    <protectedmethods title=\"\"/>\n"
        "    <protectedstaticmethods title=\"\"/>\n"
        "    <protectedattributes title=\"\"/>\n"
        "    <protectedstaticattributes title=\"\"/>\n"
        "    <packagetypes title=\"\"/>\n"
        "    <packagemethods title=\"\"/>\n"
        "    <packagestaticmethods title=\"\"/>\n"
        "    <packageattributes title=\"\"/>\n"
        "    <packagestaticattributes title=\"\"/>\n"
        "    <properties title=\"\"/>\n"
        "    <events title=\"\"/>\n"
        "    <privatetypes title=\"\"/>\n"
        "    <privateslots title=\"\"/>\n"
        "    <privatemethods title=\"\"/>\n"
        "    <privatestaticmethods title=\"\"/>\n"
        "    <privateattributes title=\"\"/>\n"
        "    <privatestaticattributes title=\"\"/>\n"
        "    <friends title=\"\"/>\n"
        "    <related title=\"\" subtitle=\"\"/>\n"
        "    <membergroups visible=\"yes\"/>\n"
        "    </memberdecl>\n"
        "    <memberdef>\n"
        "    <inlineclasses title=\"\"/>\n"
        "    <typedefs title=\"\"/>\n"
        "    <enums title=\"\"/>\n"
        "    <services title=\"\"/>\n"
        "    <interfaces title=\"\"/>\n"
        "    <constructors title=\"\"/>\n"
        "    <functions title=\"\"/>\n"
        "    <related title=\"\"/>\n"
        "    <variables title=\"\"/>\n"
        "    <properties title=\"\"/>\n"
        "    <events title=\"\"/>\n"
        "    </memberdef>\n"
        "    <allmemberslink visible=\"yes\"/>\n"
        "    <usedfiles visible=\"$SHOW_USED_FILES\"/>\n"
        "    <authorsection visible=\"yes\"/>\n"
        "</class>\n\n"
        "<namespace>\n"
        "    <briefdescription visible=\"yes\"/>\n"
        "    <memberdecl>\n"
        "    <nestednamespaces visible=\"yes\" title=\"\"/>\n"
        "    <constantgroups visible=\"yes\" title=\"\"/>\n"
        "    <classes visible=\"yes\" title=\"\"/>\n"
        "    <typedefs title=\"\"/>\n"
        "    <enums title=\"\"/>\n"
        "    <functions title=\"\"/>\n"
        "    <variables title=\"\"/>\n"
        "    <membergroups visible=\"yes\"/>\n"
        "    </memberdecl>\n"
        "    <detaileddescription title=\"\"/>\n"
        "    <memberdef>\n"
        "    <inlineclasses title=\"\"/>\n"
        "    <typedefs title=\"\"/>\n"
        "    <enums title=\"\"/>\n"
        "    <functions title=\"\"/>\n"
        "    <variables title=\"\"/>\n"
        "    </memberdef>\n"
        "    <authorsection visible=\"yes\"/>\n"
        "</namespace>\n\n"
        "<file>\n"
        "    <briefdescription visible=\"yes\"/>\n"
        "    <includes visible=\"$SHOW_INCLUDE_FILES\"/>\n"
        "    <includegraph visible=\"$INCLUDE_GRAPH\"/>\n"
        "    <includedbygraph visible=\"$INCLUDED_BY_GRAPH\"/>\n"
        "    <sourcelink visible=\"yes\"/>\n"
        "    <memberdecl>\n"
        "    <classes visible=\"yes\" title=\"\"/>\n"
        "    <namespaces visible=\"yes\" title=\"\"/>\n"
        "    <constantgroups visible=\"yes\" title=\"\"/>\n"
        "    <defines title=\"\"/>\n"
        "    <typedefs title=\"\"/>\n"
        "    <enums title=\"\"/>\n"
        "    <functions title=\"\"/>\n"
        "    <variables title=\"\"/>\n"
        "    <membergroups visible=\"yes\"/>\n"
        "    </memberdecl>\n"
        "    <detaileddescription title=\"\"/>\n"
        "    <memberdef>\n"
        "    <inlineclasses title=\"\"/>\n"
        "    <defines title=\"\"/>\n"
        "    <typedefs title=\"\"/>\n"
        "    <enums title=\"\"/>\n"
        "    <functions title=\"\"/>\n"
        "    <variables title=\"\"/>\n"
        "    </memberdef>\n"
        "    <authorsection/>\n"
        "</file>\n\n"
        "<group>\n"
        "    <briefdescription visible=\"yes\"/>\n"
        "    <groupgraph visible=\"$GROUP_GRAPHS\"/>\n"
        "    <memberdecl>\n"
        "    <nestedgroups visible=\"yes\" title=\"\"/>\n"
        "    <dirs visible=\"yes\" title=\"\"/>\n"
        "    <files visible=\"yes\" title=\"\"/>\n"
        "    <namespaces visible=\"yes\" title=\"\"/>\n"
        "    <classes visible=\"yes\" title=\"\"/>\n"
        "    <defines title=\"\"/>\n"
        "    <typedefs title=\"\"/>\n"
        "    <enums title=\"\"/>\n"
        "    <enumvalues title=\"\"/>\n"
        "    <functions title=\"\"/>\n"
        "    <variables title=\"\"/>\n"
        "    <signals title=\"\"/>\n"
        "    <publicslots title=\"\"/>\n"
        "    <protectedslots title=\"\"/>\n"
        "    <privateslots title=\"\"/>\n"
        "    <events title=\"\"/>\n"
        "    <properties title=\"\"/>\n"
        "    <friends title=\"\"/>\n"
        "    <membergroups visible=\"yes\"/>\n"
        "    </memberdecl>\n"
        "    <detaileddescription title=\"\"/>\n"
        "    <memberdef>\n"
        "    <pagedocs/>\n"
        "    <inlineclasses title=\"\"/>\n"
        "    <defines title=\"\"/>\n"
        "    <typedefs title=\"\"/>\n"
        "    <enums title=\"\"/>\n"
        "    <enumvalues title=\"\"/>\n"
        "    <functions title=\"\"/>\n"
        "    <variables title=\"\"/>\n"
        "    <signals title=\"\"/>\n"
        "    <publicslots title=\"\"/>\n"
        "    <protectedslots title=\"\"/>\n"
        "    <privateslots title=\"\"/>\n"
        "    <events title=\"\"/>\n"
        "    <properties title=\"\"/>\n"
        "    <friends title=\"\"/>\n"
        "    </memberdef>\n"
        "    <authorsection visible=\"yes\"/>\n"
        "</group>\n\n"
        "<directory>\n"
        "    <briefdescription visible=\"yes\"/>\n"
        "    <directorygraph visible=\"yes\"/>\n"
        "    <memberdecl>\n"
        "    <dirs visible=\"yes\"/>\n"
        "    <files visible=\"yes\"/>\n"
        "    </memberdecl>\n"
        "    <detaileddescription title=\"\"/>\n"
        "</directory>\n"
        "</doxygenlayout>\n";

    return cWriteFileInternal(FileName, Templ, m_cGenerator);
}

bool CDoxygen::cWriteMainpageInternal() const
{
    const std::string FileName = "main.dox";
    const std::string Templ =
        "/// @mainpage #^#TITLE#$#\n"
        "/// @tableofcontents\n"
        "/// #^#DESC#$#\n"
        "/// Below is a summary of generated types and functions.\n"
        "#^#INPUT#$#\n"
        "#^#OUTPUT#$#\n"
        "#^#APPEND#$#\n"
        "\n";

    auto schemaObj = m_cGenerator.genProtocolSchema().genParseObj();
    auto dispName = util::genDisplayName(schemaObj.parseDisplayName(), schemaObj.parseName());

    util::GenReplacementMap repl = {
        {"INPUT", cInputDocInternal()},
        {"OUTPUT", cOutputDocInternal()},
        {"APPEND", m_cGenerator.genReadCodeInjectCode(m_cGenerator.cInputRelPathForDoc(FileName) + strings::genAppendFileSuffixStr(), "Append here")},
        {"TITLE", "\"C\" Interface for the \"" + dispName + "\" Protocol Definition."},
        {"DESC", schemaObj.parseDescription()},
    };

    if (!repl["DESC"].empty()) {
        repl["DESC"].append("@n");
    }

    return cWriteFileInternal(FileName, util::genProcessTemplate(Templ, repl), m_cGenerator);
}

std::string CDoxygen::cInputDocInternal() const
{
    util::GenStringsList frameItems;
    for (auto* f : m_frames) {
        static const std::string Templ =
            "/// @li @ref #^#NAME#$# defined in @b #^#PATH#$#."
            ;

        util::GenReplacementMap repl = {
            {"NAME", f->cName()},
            {"PATH", f->cRelHeader()},
        };

        frameItems.push_back(util::genProcessTemplate(Templ, repl));
    }

    if (frameItems.empty()) {
        return strings::genEmptyString();
    }

    const std::string Templ =
        "/// @section main_process_input Processing Input\n"
        "/// To be able to process input there is a need to allocate a relevant frame processing object.\n"
        "/// @code\n"
        "/// #^#FRAME#$#* frame = #^#FRAME#$#_alloc();\n"
        "/// @endcode\n"
        "/// When the frame object is no longer needed there is a need to de-allocate it.\n"
        "/// @code\n"
        "/// #^#FRAME#$#_alloc(frame);\n"
        "/// @endcode\n"
        "/// There are two functions processing input and dispatching the allocated message to appropriate handler.\n"
        "/// Note that the caller is responsible to maintain input buffer, the functions return amount of processed bytes\n"
        "/// which need to be removed from the buffer.\n"
        "///\n"
        "/// @li @ref #^#FRAME#$#_processInputData()\n"
        "/// @li @ref #^#FRAME#$#_processInputDataSingleMsg()\n"
        "///\n"
        "/// Note that the handler object needs to be prepared to be able to dispatch messages to their respective handling functions.\n"
        "/// When there is a need to process the framing values use @ref #^#FRAME#$#_processInputDataSingleMsg().\n"
        "///\n"
        "/// Available Frames are:\n"
        "#^#ITEMS#$#\n"
        "///"
        ;

    auto* firstFrame = m_frames.front();
    util::GenReplacementMap repl = {
        {"FRAME", firstFrame->cName()},
        {"ITEMS", util::genStrListToString(frameItems, "\n", "")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string CDoxygen::cOutputDocInternal() const
{
    auto messages = m_cGenerator.genGetAllMessages();

    if ((messages.empty()) || (m_frames.empty())) {
        return strings::genEmptyString();
    }

    util::GenStringsList msgItems;
    for (auto* m : messages) {
        if (!m->genIsReferenced()) {
            continue;
        }

        static const std::string Templ =
            "/// @li @ref #^#NAME#$# defined in @b #^#PATH#$#."
            ;

        auto* cMsg = CMessage::cCast(m);
        util::GenReplacementMap repl = {
            {"NAME", cMsg->cName()},
            {"PATH", cMsg->cRelHeader()},
        };

        msgItems.push_back(util::genProcessTemplate(Templ, repl));
    }

    const std::string Templ =
        "/// @section main_send_msgs Sending Messages\n"
        "/// The message object needs to be allocated.\n"
        "/// @code\n"
        "/// #^#MSG#$#* msg = #^#MSG#$#_alloc();\n"
        "/// @endcode\n"
        "/// After the work with the message object is complete, it needs to be de-allocated.\n"
        "/// @code\n"
        "/// #^#MSG#$#_free(msg);\n"
        "/// @endcode\n"
        "/// Use @b #^#MSG#$#_field_&lt;field_name&gt;() functions to acquire access to message fields to be able to update their values.\n"
        "///\n"
        "/// Then use previously allocated frame object to send the message:\n"
        "/// @code\n"
        "/// #^#ERROR_STATUS#$# ec = #^#FRAME#$#_writeMessage(frame, #^#MSG#$#_toInterface(msg), buf, &bufSize);\n"
        "/// @endcode\n"
        "/// Note that @ref #^#FRAME#$#_writeMessage() function receives a message interface handle rather than the message itself.\n"
        "/// Also note that the last parameter of buffer size is both input and output parameter. On input it provides an available size of the buffer,\n"
        "/// while on output it provides amount of bytes actually written to the buffer.\n"
        "///\n"
        "/// In order to acquire requires size of the output buffer use:\n"
        "/// @code\n"
        "/// size_t bufSize = #^#FRAME#$#_messageLength(frame, #^#MSG#$#_toInterface(msg));\n"
        "/// @endcode\n"
        "/// It is also possible to allocate message using message factory residing in the frame object.\n"
        "/// @code\n"
        "/// iface = #^#FRAME#$#_createMsg(frame, msgId, 0);\n"
        "/// msg = #^#MSG#$#_fromInterface(iface);\n"
        "/// // Update message contents here\n"
        "/// ec = #^#FRAME#$#_writeMessage(frame, iface, buf, &bufSize);\n"
        "/// #^#FRAME#$#_deleteMsg(iface); // The allocated message needs to be deleted\n"
        "/// @endcode\n"
        "/// Note that message allocated with @ref #^#FRAME#$#_createMsg() is expected to be deleted with @ref #^#FRAME#$#_deleteMsg()\n"
        "///\n"
        "/// Available messages:\n"
        "#^#ITEMS#$#\n"
        "///"
        ;

    auto* firstMsg = CMessage::cCast(messages.front());
    auto* firstFrame = m_frames.front();
    util::GenReplacementMap repl = {
        {"MSG", firstMsg->cName()},
        {"FRAME", firstFrame->cName()},
        {"ERROR_STATUS", CErrorStatus::cName(m_cGenerator)},
        {"ITEMS", util::genStrListToString(msgItems, "\n", "")}
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2c
