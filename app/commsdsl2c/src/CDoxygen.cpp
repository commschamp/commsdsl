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
        {"APPEND", util::genReadFileContents(m_cGenerator.cInputAbsPathForDoc(FileName) + strings::genAppendFileSuffixStr())},
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
        {"APPEND", util::genReadFileContents(m_cGenerator.cInputAbsPathForDoc(FileName) + strings::genAppendFileSuffixStr())},
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

// std::string CDoxygen::cMessagesDocInternal() const
// {
//     auto nsList = m_cGenerator.genGetAllNamespaces();
//     util::GenStringsList elems;
//     for (auto* n : nsList) {
//         static const std::string Templ =
//             "/// @li @ref #^#SCOPE#$#::#^#SUFFIX#$# (defined in @b #^#PATH#$#/#^#SUFFIX#$#  directory)";

//         util::GenReplacementMap repl = {
//             {"SCOPE", comms::genScopeFor(*n, m_cGenerator)},
//             {"PATH", util::genScopeToRelPath(comms::genScopeFor(*n, m_cGenerator))},
//             {"SUFFIX", strings::genMessageNamespaceStr()},
//         };

//         elems.push_back(util::genProcessTemplate(Templ, repl));
//     }

//     const std::string Templ =
//         "/// @section main_messages Available Message Classes\n"
//         "/// The following namespaces contain all the classes describing available messages:\n"
//         "#^#LIST#$#\n"
//         "///"
//         ;

//     util::GenReplacementMap repl = {
//         {"LIST", util::genStrListToString(elems, "\n", "")}
//     };
//     return util::genProcessTemplate(Templ, repl);
// }

// std::string CDoxygen::cFieldsDocInternal() const
// {
//     auto nsList = m_cGenerator.genGetAllNamespaces();
//     util::GenStringsList elems;
//     for (auto* n : nsList) {
//         static const std::string Templ =
//             "/// @li @ref #^#SCOPE#$#::#^#SUFFIX#$# (defined in @b #^#PATH#$#/#^#SUFFIX#$#  directory)";

//         util::GenReplacementMap repl = {
//             {"SCOPE", comms::genScopeFor(*n, m_cGenerator)},
//             {"PATH", util::genScopeToRelPath(comms::genScopeFor(*n, m_cGenerator))},
//             {"SUFFIX", strings::genFieldNamespaceStr()},
//         };

//         elems.push_back(util::genProcessTemplate(Templ, repl));
//     }

//     const std::string Templ =
//         "/// @section main_fields Available Common Fields\n"
//         "/// The following namespaces contain all definition of all the fields,\n"
//         "/// which can be references by multiple messages:\n"
//         "#^#LIST#$#\n"
//         "///"
//         ;

//     util::GenReplacementMap repl = {
//         {"LIST", util::genStrListToString(elems, "\n", "")}
//     };
//     return util::genProcessTemplate(Templ, repl);
// }

// std::string CDoxygen::cInterfaceDocInternal() const
// {
//     auto interfaces = m_cGenerator.genGetAllInterfaces();
//     assert(!interfaces.empty());

//     util::GenStringsList list;
//     for (auto* i : interfaces) {
//         list.push_back(
//             "/// @li @ref " + comms::genScopeFor(*i, m_cGenerator) +
//             " (defined in @b " + comms::genRelHeaderPathFor(*i, m_cGenerator) + " header file).");
//     }

//     static const std::string Templ =
//         "/// @section main_interfaces Common Interface Classes\n"
//         "/// The available common interface classes are:\n"
//         "#^#LIST#$#\n"
//         "///";

//     util::GenReplacementMap repl = {
//         {"LIST", util::genStrListToString(list, "\n", "")}
//     };
//     return util::genProcessTemplate(Templ, repl);
// }

// std::string CDoxygen::cFrameDocInternal() const
// {
//     const std::string Templ =
//         "/// @section main_frames Frame Classes\n"
//         "/// The available frame classes are:\n"
//         "#^#LIST#$#\n"
//         "///\n"
//         "/// Every frame class/type definition receives (as a template parameter) a list of\n"
//         "/// @b input message types it is expected to recognize. Default defintion\n"
//         "/// uses @b All messages bundle (for example @ref #^#ALL_MESSAGES#$# defined in @b #^#ALL_MESSAGES_HEADER#$#).\n"
//         "/// @n If protocol defines any uni-directional message, then it is recommended to use\n"
//         "/// either @b Server messages bundle (for example @ref #^#SERVER_MESSAGES#$# from @b #^#SERVER_MESSAGES_HEADER#$#)\n"
//         "/// or @b Client messages bundle (for example @ref #^#CLIENT_MESSAGES#$# from @b #^#CLIENT_MESSAGES_HEADER#$#)\n"
//         "#^#PLATFORMS#$#\n"
//         "/// @b NOTE, that the frame definition does not exactly follow the recommended\n"
//         "/// instructions from <b>Protocol Stack Definition Tutorial</b> page of @b COMMS\n"
//         "/// library documentation. The extra customization (see @ref main_customization below)\n"
//         "/// is performed by passing options to the layers themselves.\n"
//         "///\n"
//         "/// The available bundles of input messages are:\n"
//         "#^#MESSAGES_LIST#$#\n"
//         "///";

//     auto frames = m_cGenerator.genGetAllFrames();
//     assert(!frames.empty());

//     util::GenStringsList list;
//     for (auto* f : frames) {
//         list.push_back(
//             "/// @li @ref " + comms::genScopeFor(*f, m_cGenerator) +
//             " (from @b " + comms::genRelHeaderPathFor(*f, m_cGenerator) + " header file).");
//     }

//     util::GenStringsList messagesList;
//     auto addToMessagesListFunc =
//         [this, &messagesList](const std::string& name, const commsdsl::gen::GenNamespace& ns)
//         {
//             auto scope = comms::genScopeForInput(name, m_cGenerator, ns);
//             auto file = comms::genRelHeaderForInput(name, m_cGenerator, ns);
//             auto str = "/// @li @ref " + scope + " (from @b " + file + " header file).";
//             messagesList.push_back(std::move(str));
//         };

//     auto allNamespaces = m_cGenerator.genGetAllNamespaces();
//     for (auto* ns : allNamespaces) {
//         if ((!ns->genHasFramesRecursive()) ||
//             (!ns->genHasMessagesRecursive())) {
//             continue;
//         }

//         addToMessagesListFunc(strings::genAllMessagesStr(), *ns);
//         addToMessagesListFunc(CServerInputMessagesStr, *ns);
//         addToMessagesListFunc(CClientInputMessagesStr, *ns);

//         auto& platforms = m_cGenerator.genCurrentSchema().platformNames();
//         for (auto& p : platforms) {
//             addToMessagesListFunc(p + "Messages", *ns);
//             addToMessagesListFunc(p + CServerInputMessagesStr, *ns);
//             addToMessagesListFunc(p + CClientInputMessagesStr, *ns);
//         };
//     }

//     auto nsIter =
//         std::find_if(
//             allNamespaces.begin(), allNamespaces.end(),
//             [](auto* ns)
//             {
//                 return !ns->genFrames().empty();
//             });

//     assert (nsIter != allNamespaces.end());
//     auto& exampleNs = **nsIter;

//     util::GenReplacementMap repl = {
//         {"LIST", util::genStrListToString(list, "\n", "")},
//         {"MESSAGES_LIST", util::genStrListToString(messagesList, "\n", "")},
//         {"PROT_NAMESPACE", m_cGenerator.genCurrentSchema().genMainNamespace()},
//         {"PLATFORMS", cPlatformsDocInternal()},
//         {"ALL_MESSAGES", comms::genScopeForInput(strings::genAllMessagesStr(), m_cGenerator, exampleNs)},
//         {"ALL_MESSAGES_HEADER", comms::genRelHeaderForInput(strings::genAllMessagesStr(), m_cGenerator, exampleNs)},
//         {"SERVER_MESSAGES", comms::genScopeForInput(CServerInputMessagesStr, m_cGenerator, exampleNs)},
//         {"SERVER_MESSAGES_HEADER", comms::genRelHeaderForInput(CServerInputMessagesStr, m_cGenerator, exampleNs)},
//         {"CLIENT_MESSAGES", comms::genScopeForInput(CClientInputMessagesStr, m_cGenerator, exampleNs)},
//         {"CLIENT_MESSAGES_HEADER", comms::genRelHeaderForInput(CClientInputMessagesStr, m_cGenerator, exampleNs)},
//     };

//     return util::genProcessTemplate(Templ, repl);
// }

// std::string CDoxygen::cDispatchDocInternal() const
// {
//     const std::string Templ =
//         "/// @section main_dispatch Dispatching Message Objects\n"
//         "/// While the @b COMMS library provides various built-in ways of \n"
//         "/// dispatching message objects into their respective handlers\n"
//         "/// (see <b>Advanced Guide to Message Dispatching</b> page of the\n"
//         "/// @b COMMS library documentation), the generated code also provides\n"
//         "/// additional auxiliary dispatch functions which are @b switch statement\n"
//         "/// based.\n"
//         "///\n"
//         "/// The available functions are:\n"
//         "#^#LIST#$#\n"
//         "///\n"
//         "/// There are also some dispatcher objects to be used with\n"
//         "/// @b comms::processAllWithDispatchViaDispatcher() function or similar.\n"
//         "#^#DISPATCHERS_LIST#$#\n"
//         "///";

//     util::GenStringsList list;
//     util::GenStringsList dispatcherList;
//     auto addToListFunc =
//         [this, &list, &dispatcherList](const std::string& name, const commsdsl::gen::GenNamespace& ns)
//         {
//             static const std::string Prefix("Dispatch");
//             static const std::string Suffix("Message");

//             auto accessName = comms::genAccessName(Prefix + name + Suffix);
//             auto accessScope = comms::genScopeForDispatch(accessName, m_cGenerator, ns);
//             auto fileName = comms::genClassName(accessName);
//             auto file = comms::genRelHeaderForDispatch(fileName, m_cGenerator, ns);
//             auto str = "/// @li @ref " + accessScope +
//                 "()\n/// (defined in @b " + file + " header file).";
//             list.push_back(std::move(str));
//             auto defaultOptStr = "/// @li @ref " + accessScope +
//                 strings::genDefaultOptionsStr() + "()\n/// (defined in @b " + file + " header file).";
//             list.push_back(std::move(defaultOptStr));

//             static const std::string DispatcherSuffix("MsgDispatcher");
//             auto dispatcherName = comms::genClassName(name) + DispatcherSuffix;
//             auto dispatcherScope = comms::genScopeForDispatch(dispatcherName, m_cGenerator, ns);
//             auto dispatcherDefaultScope = dispatcherScope + strings::genDefaultOptionsStr();
//             auto dispatcherStr =
//                 "/// @li @ref " + dispatcherScope + "\n/// (defined in @b " + file + " header file).";
//             dispatcherList.push_back(std::move(dispatcherStr));
//             auto dispatcherDefaultOptionsStr =
//                 "/// @li @ref " + dispatcherDefaultScope + "\n/// (defined in @b " + file + " header file).";
//             dispatcherList.push_back(std::move(dispatcherDefaultOptionsStr));
//         };

//     auto addPlatformFunc =
//         [&addToListFunc](const std::string& platform, const commsdsl::gen::GenNamespace& ns)
//         {
//             addToListFunc(platform, ns);
//             addToListFunc(platform + CServerInputPrefixStr, ns);
//             addToListFunc(platform + CClientInputPrefixStr, ns);
//         };

//     auto allNamespaces = m_cGenerator.genGetAllNamespaces();
//     for (auto& ns : allNamespaces) {
//         if (!ns->genHasFramesRecursive()) {
//             continue;
//         }

//         addPlatformFunc(strings::genEmptyString(), *ns);
//         for (auto& p : m_cGenerator.genCurrentSchema().platformNames()) {
//             addPlatformFunc(comms::genClassName(p), *ns);
//         }
//     }

//     util::GenReplacementMap repl = {
//         {"LIST", util::genStrListToString(list, "\n", "")},
//         {"DISPATCHERS_LIST", util::genStrListToString(dispatcherList, "\n", "")},
//     };
//     return util::genProcessTemplate(Templ, repl);
// }

// std::string CDoxygen::cCustomizeDocInternal() const
// {
//     const std::string Templ =
//         "/// @section main_customization Customization\n"
//         "/// Depending on the value of @b customization option passed to the @b commsdsl2c\n"
//         "/// code generator, the latter generates @ref #^#OPTIONS#$#\n"
//         "/// struct (defined in @b #^#OPTIONS_HDR#$# file),\n"
//         "/// which is used by default throughout the protocol definition classes.\n"
//         "/// The struct contains all the available type definition, which can be used to\n"
//         "/// customize default data structures and/or behaviour of various classes.\n"
//         "/// If any additional customization is required, just recreate similar struct with\n"
//         "/// relevant types overriden with new definition. The easiest way is to extend\n"
//         "/// the #^#OPTIONS#$#. For example:\n"
//         "/// @code\n"
//         "/// struct MyOptions : public #^#OPTIONS#$#\n"
//         "/// {\n"
//         "///     struct field : public #^#OPTIONS#$#::field\n"
//         "///     {\n"
//         "///         // use comms::util::StaticString as storage type\n"
//         "///         using SomeStringField = comms::option::app::FixedSizeStorage<32>;\n"
//         "///     };\n"
//         "/// };\n"
//         "/// @endcode\n"
//         "/// @b NOTE, that inner scope of structs in the #^#OPTIONS#$#\n"
//         "/// resembles scope of namespaces used in protocol definition.\n"
//         "///\n"
//         "/// The @b COMMS library also provides a flexible way to configure polymorphic\n"
//         "/// interface for the message classes. If the defined protocol has multiple\n"
//         "/// uni-directional messages (marked using @b sender property in the schema),\n"
//         "/// then it is recommended to define two separate interfaces: one for input,\n"
//         "/// other for output:\n"
//         "/// @code\n"
//         "/// using MyInputMsg =\n"
//         "///    #^#INTERFACE#$#<\n"
//         "///        comms::option::app::ReadIterator<const std::uint8_t*>, // for polymorphic read\n"
//         "///        comms::option::app::Handler<MyHandler> // for polymorphic dispatch\n"
//         "///    >;\n"
//         "///\n"
//         "/// using MyOutputMsg =\n"
//         "///    #^#INTERFACE#$#<\n"
//         "///        comms::option::app::WriteIterator<std::uint8_t*>, // for polymorphic write\n"
//         "///        comms::option::app::LengthInfoInterface, // for polymorphic serialization length retrieval\n"
//         "///        comms::option::app::IdInfoInterface // for polymorphic message ID retrieval\n"
//         "///    >;\n"
//         "/// @endcode\n"
//         "/// In case there are only few uni-directional messages, it may make sence to define\n"
//         "/// single interface class:\n"
//         "/// @code\n"
//         "/// using MyMsg =\n"
//         "///    #^#INTERFACE#$#<\n"
//         "///        comms::option::app::ReadIterator<const std::uint8_t*>, // for polymorphic read\n"
//         "///        comms::option::app::Handler<MyHandler> // for polymorphic dispatch\n"
//         "///        comms::option::app::WriteIterator<std::uint8_t*>, // for polymorphic write\n"
//         "///        comms::option::app::LengthInfoInterface, // for polymorphic serialization length retrieval\n"
//         "///        comms::option::app::IdInfoInterface // for polymorphic message ID retrieval\n"
//         "///    >;\n"
//         "/// @endcode\n"
//         "/// In this case the code generator may also define @b #^#SERVER_OPTIONS#$#\n"
//         "/// (defined in @b #^#SERVER_OPTIONS_HDR#$# file) and\n"
//         "/// @b #^#CLIENT_OPTIONS#$# (defined in @b #^#CLIENT_OPTIONS_HDR#$# file).\n"
//         "/// These structs suppress generation of unnecessary virtual functions which are not\n"
//         "/// going to be used. Consider using this structs as options instead of default\n"
//         "/// #^#OPTIONS#$#.\n"
//         "///\n"
//         "/// Also there is @ref #^#BARE_METAL_OPTIONS#$#\n"
//         "/// (defined in @b #^#BARE_METAL_OPTIONS_HDR#$# file) which can help in defining\n"
//         "/// options for bare-metal applications. It exclude all usage of dynamic memory allocation.\n"
//         "///\n"
//         "/// In case non-custom &lt;id&gt; layer has been used in schema (files), custom,\n"
//         "/// application-specific allocation options to it may include\n"
//         "/// @b comms::option::app::InPlaceAllocation and/or @b comms::option::app::SupportGenericMessage.\n"
//         "/// Please see the documentation of the @b COMMS library itself for more details.\n"
//         "///\n"
//         "/// In many cases the input buffer is sequential (not circular), where the end of message payload\n"
//         "/// never precedes its beginning and the processing is implemented in a way where message objects\n"
//         "/// never outlive the input buffer. In such scenarios it could be a waste of memory / CPU cycles to\n"
//         "/// copy bytes from the input buffer to internal storage of the fields like &lt;string&gt;\n"
//         "/// (@b comms::field::String) and/or &lt;data&gt; (@b comms::field::ArrayList of raw bytes).\n"
//         "/// The generated code also provides #^#DATA_VIEW_OPTIONS#$# (defined in\n"
//         "/// @b #^#DATA_VIEW_OPTIONS_HDR#$# file) where relevant fields apply @b comms::option::app::OrigDataView\n"
//         "/// option.\n"
//         "///\n"
//         "/// Also note that the specified extension options are implemented as the following template classes\n"
//         "/// which receive other options as their base class and apply relevant changes on top.\n"
//         "/// @li @ref #^#CLIENT_OPTIONS#$#T\n"
//         "/// @li @ref #^#SERVER_OPTIONS#$#T\n"
//         "/// @li @ref #^#BARE_METAL_OPTIONS#$#T\n"
//         "/// @li @ref #^#DATA_VIEW_OPTIONS#$#T\n"
//         "///\n"
//         "/// As the result it is possible to combine them. For example:\n"
//         "/// @code\n"
//         "/// using MyOptions=\n"
//         "///     #^#DATA_VIEW_OPTIONS#$#T<\n"
//         "///         #^#CLIENT_OPTIONS#$#\n"
//         "///     >;\n"
//         "/// @endcode"
//         ;

//     auto allInterfaces = m_cGenerator.genGetAllInterfaces();
//     assert(!allInterfaces.empty());

//     util::GenReplacementMap repl = {
//         {"INTERFACE", comms::genScopeFor(*allInterfaces.front(), m_cGenerator)},
//         {"PROT_NAMESPACE", m_cGenerator.genCurrentSchema().genMainNamespace()},
//         {"OPTIONS", comms::genScopeForOptions(strings::genDefaultOptionsStr(), m_cGenerator)},
//         {"CLIENT_OPTIONS", comms::genScopeForOptions("Client" + strings::genDefaultOptionsStr(), m_cGenerator)},
//         {"SERVER_OPTIONS", comms::genScopeForOptions("Server" + strings::genDefaultOptionsStr(), m_cGenerator)},
//         {"OPTIONS_HDR", comms::genRelHeaderForOptions(strings::genDefaultOptionsStr(), m_cGenerator)},
//         {"CLIENT_OPTIONS_HDR", comms::genRelHeaderForOptions("Client" + strings::genDefaultOptionsStr(), m_cGenerator)},
//         {"SERVER_OPTIONS_HDR", comms::genRelHeaderForOptions("Server" + strings::genDefaultOptionsStr(), m_cGenerator)},
//         {"BARE_METAL_OPTIONS", comms::genScopeForOptions(strings::genBareMetalStr() + strings::genDefaultOptionsStr(), m_cGenerator)},
//         {"BARE_METAL_OPTIONS_HDR", comms::genRelHeaderForOptions(strings::genBareMetalStr() + strings::genDefaultOptionsStr(), m_cGenerator)},
//         {"DATA_VIEW_OPTIONS", comms::genScopeForOptions(strings::genDataViewStr() + strings::genDefaultOptionsStr(), m_cGenerator)},
//         {"DATA_VIEW_OPTIONS_HDR", comms::genRelHeaderForOptions(strings::genDataViewStr() + strings::genDefaultOptionsStr(), m_cGenerator)},
//     };

//     return util::genProcessTemplate(Templ, repl);
// }

// std::string CDoxygen::cVersionDocInternal() const
// {
//     if (!m_cGenerator.genCurrentSchema().genVersionDependentCode()) {
//         return strings::genEmptyString();
//     }

//     const std::string Templ =
//         "///\n"
//         "/// @section main_version Version Dependent Code\n"
//         "/// The generated code is version dependent. The version information is stored in\n"
//         "/// one of the fields held by a common interface class (see @ref main_interfaces).\n"
//         "/// When presence of the field depends on the protocol version, it is defined as\n"
//         "/// @b comms::field::Optional field which wraps a proper field definition. Please\n"
//         "/// remember to use @b field() member function to access the wrapped field, before\n"
//         "/// using @b value() member function to access the actual value.\n"
//         "/// @code\n"
//         "/// void handle(SomeMessage& msg)\n"
//         "/// {\n"
//         "///     auto& versionDependentField = msg.field_someVersionDependentField();\n"
//         "///     auto& wrappedField = versionDependentField.field();\n"
//         "///     auto actualValue = wrappedField.value();\n"
//         "///     ...\n"
//         "/// }\n"
//         "/// @endcode\n"
//         "/// Every default constructed message will have a version of the schema with\n"
//         "/// all relevant fields being marked present/missing based on this version information.\n"
//         "/// If there is a need to send a message with different protocol version information,\n"
//         "/// the message needs to be brought into a consistent state by calling its @b doRefresh()\n"
//         "/// member function (or @b refresh() in case of proper polymorphic behavior has been enabled)\n"
//         "/// after version information has been updated.\n"
//         "/// @code\n"
//         "///     #^#PROT_NAMESPACE#$#::message::SomeMsg<MyOutputMsg> msg;\n"
//         "///     msg.version() = 4U;\n"
//         "///     msg.doRefresh(); // will update exists/missing state of every dependent field\n"
//         "/// @endcode";

//     util::GenReplacementMap repl = {
//         {"PROT_NAMESPACE", m_cGenerator.genCurrentSchema().genMainNamespace()},
//     };
//     return util::genProcessTemplate(Templ, repl);
// }

// std::string CDoxygen::cPlatformsDocInternal() const
// {
//     auto platforms = m_cGenerator.genCurrentSchema().platformNames();
//     if (platforms.empty()) {
//         return strings::genEmptyString();
//     }

//     const std::string Templ =
//         "///\n"
//         "/// There are also platform specific definitions:\n"
//         "#^#LIST#$#\n"
//         "///";

//     util::GenStringsList list;
//     auto allNamespaces = m_cGenerator.genGetAllNamespaces();
//     for (auto* ns : allNamespaces) {
//         if ((!ns->genHasFramesRecursive()) ||
//             (!ns->genHasMessagesRecursive())) {
//             continue;
//         }

//         for (auto& p : platforms) {
//             auto addToListFunc =
//                 [this, &p, &list, ns](const std::string& type)
//                 {
//                     auto name = comms::genClassName(p) + type + "InputMessages";
//                     auto scope = comms::genScopeForInput(name, m_cGenerator, *ns);
//                     auto file = comms::genRelHeaderForInput(name, m_cGenerator, *ns);
//                     auto str = "/// @li @ref " + scope + " (from @b " + file + ").";
//                     list.push_back(std::move(str));
//                 };

//             addToListFunc("Server");
//             addToListFunc("Client");
//         }
//     }

//     if (list.empty()) {
//         return strings::genEmptyString();
//     }

//     util::GenReplacementMap repl = {
//         {"LIST", util::genStrListToString(list, "\n", "")}
//     };
//     return util::genProcessTemplate(Templ, repl);
// }

} // namespace commsdsl2c
