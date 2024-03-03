//
// Copyright 2019 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "CommsDoxygen.h"

#include "CommsGenerator.h"
#include "CommsSchema.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

namespace 
{

const std::string ServerInputPrefixStr = "ServerInput";
const std::string ClientInputPrefixStr = "ClientInput";
const std::string ServerInputMessagesStr = ServerInputPrefixStr + "Messages";
const std::string ClientInputMessagesStr = ClientInputPrefixStr + "Messages";    

using ReplacementMap = util::ReplacementMap;

bool writeFileInternal(const std::string& name, const std::string& str, CommsGenerator& generator)
{
    auto filePath = comms::pathForDoc(name, generator);
    generator.logger().info("Generating " + filePath);

    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!generator.createDirectory(dirPath)) {
        return false;
    }      

    std::ofstream stream(filePath);
    if (!stream) {
        generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }    

    stream << str;
    stream.flush();

    if (!stream.good()) {
        generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    return true;    
}

} // namespace 
    

bool CommsDoxygen::write(CommsGenerator& generator)
{
    CommsDoxygen obj(generator);
    return obj.commsWriteInternal();
}

bool CommsDoxygen::commsWriteInternal() const
{
    return 
        commsWriteConfInternal() &&
        commsWriteLayoutInternal() &&
        commsWriteNamespacesInternal() &&
        commsWriteMainpageInternal();
}

bool CommsDoxygen::commsWriteConfInternal() const
{
    const std::string FileName = "doxygen.conf";
    const std::string Templ = 
        "DOXYFILE_ENCODING      = UTF-8\n"
        "PROJECT_NAME           = \"#^#PROJ_NAME#$#\"\n"
        "PROJECT_BRIEF          = \"Documentation for generated code of \\\"#^#PROJ_NAME#$#\\\" protocol.\"\n"
        "OUTPUT_DIRECTORY       = \n"
        "CREATE_SUBDIRS         = NO\n"
        "ALLOW_UNICODE_NAMES    = NO\n"
        "OUTPUT_LANGUAGE        = English\n"
        "OUTPUT_TEXT_DIRECTION  = None\n"
        "BRIEF_MEMBER_DESC      = YES\n"
        "REPEAT_BRIEF           = YES\n"
        "ALWAYS_DETAILED_SEC    = NO\n"
        "INLINE_INHERITED_MEMB  = YES\n"
        "FULL_PATH_NAMES        = YES\n"
        "SHORT_NAMES            = NO\n"
        "INHERIT_DOCS           = YES\n"
        "SEPARATE_MEMBER_PAGES  = NO\n"
        "TAB_SIZE               = 4\n"
        "OPTIMIZE_OUTPUT_FOR_C  = NO\n"
        "OPTIMIZE_OUTPUT_JAVA   = NO\n"
        "OPTIMIZE_FOR_FORTRAN   = NO\n"
        "OPTIMIZE_OUTPUT_VHDL   = NO\n"
        "MARKDOWN_SUPPORT       = YES\n"
        "AUTOLINK_SUPPORT       = YES\n"
        "BUILTIN_STL_SUPPORT    = YES\n"
        "CPP_CLI_SUPPORT        = NO\n"
        "SIP_SUPPORT            = NO\n"
        "IDL_PROPERTY_SUPPORT   = YES\n"
        "DISTRIBUTE_GROUP_DOC   = NO\n"
        "GROUP_NESTED_COMPOUNDS = NO\n"
        "SUBGROUPING            = YES\n"
        "INLINE_GROUPED_CLASSES = NO\n"
        "INLINE_SIMPLE_STRUCTS  = NO\n"
        "TYPEDEF_HIDES_STRUCT   = NO\n"
        "LOOKUP_CACHE_SIZE      = 0\n"
        "NUM_PROC_THREADS       = 0\n"
        "EXTRACT_ALL            = NO\n"
        "EXTRACT_PRIVATE        = NO\n"
        "EXTRACT_PRIV_VIRTUAL   = NO\n"
        "EXTRACT_PACKAGE        = NO\n"
        "EXTRACT_STATIC         = NO\n"
        "EXTRACT_LOCAL_CLASSES  = YES\n"
        "EXTRACT_LOCAL_METHODS  = NO\n"
        "EXTRACT_ANON_NSPACES   = NO\n"
        "RESOLVE_UNNAMED_PARAMS = YES\n"
        "HIDE_UNDOC_MEMBERS     = YES\n"
        "HIDE_UNDOC_CLASSES     = YES\n"
        "HIDE_FRIEND_COMPOUNDS  = NO\n"
        "HIDE_IN_BODY_DOCS      = NO\n"
        "INTERNAL_DOCS          = NO\n"
        "CASE_SENSE_NAMES       = YES\n"
        "HIDE_SCOPE_NAMES       = NO\n"
        "HIDE_COMPOUND_REFERENCE= NO\n"
        "SHOW_INCLUDE_FILES     = YES\n"
        "SHOW_GROUPED_MEMB_INC  = NO\n"
        "FORCE_LOCAL_INCLUDES   = YES\n"
        "INLINE_INFO            = NO\n"
        "SORT_MEMBER_DOCS       = YES\n"
        "SORT_BRIEF_DOCS        = YES\n"
        "SORT_MEMBERS_CTORS_1ST = YES\n"
        "SORT_GROUP_NAMES       = NO\n"
        "SORT_BY_SCOPE_NAME     = YES\n"
        "STRICT_PROTO_MATCHING  = NO\n"
        "GENERATE_TODOLIST      = YES\n"
        "GENERATE_TESTLIST      = YES\n"
        "GENERATE_BUGLIST       = YES\n"
        "GENERATE_DEPRECATEDLIST= YES\n"
        "MAX_INITIALIZER_LINES  = 30\n"
        "SHOW_USED_FILES        = YES\n"
        "SHOW_FILES             = YES\n"
        "SHOW_NAMESPACES        = YES\n"
        "LAYOUT_FILE            = doc/layout.xml\n"
        "QUIET                  = YES\n"
        "WARNINGS               = YES\n"
        "WARN_IF_UNDOCUMENTED   = NO\n"
        "WARN_IF_DOC_ERROR      = YES\n"
        "WARN_NO_PARAMDOC       = YES\n"
        "WARN_AS_ERROR          = YES\n"
        "WARN_FORMAT            = \"$file:$line: $text\"\n"
        "INPUT_ENCODING         = UTF-8\n"
        "RECURSIVE              = YES\n"
        "EXCLUDE                = \n"
        "EXCLUDE_SYMLINKS       = NO\n"
        "EXCLUDE_SYMBOLS        = *details\n"
        "EXAMPLE_RECURSIVE      = NO\n"
        "FILTER_SOURCE_FILES    = NO\n"
        "SOURCE_BROWSER         = NO\n"
        "INLINE_SOURCES         = NO\n"
        "STRIP_CODE_COMMENTS    = YES\n"
        "REFERENCED_BY_RELATION = NO\n"
        "REFERENCES_RELATION    = NO\n"
        "REFERENCES_LINK_SOURCE = YES\n"
        "SOURCE_TOOLTIPS        = YES\n"
        "USE_HTAGS              = NO\n"
        "VERBATIM_HEADERS       = YES\n"
        "CLANG_ASSISTED_PARSING = NO\n"
        "CLANG_ADD_INC_PATHS    = YES\n"
        "CLANG_OPTIONS          =\n"
        "ALPHABETICAL_INDEX     = YES\n"
        "GENERATE_HTML          = YES\n"
        "HTML_OUTPUT            = html\n"
        "HTML_FILE_EXTENSION    = .html\n"
        "HTML_COLORSTYLE_HUE    = 220\n"
        "HTML_COLORSTYLE_SAT    = 100\n"
        "HTML_COLORSTYLE_GAMMA  = 80\n"
        "HTML_TIMESTAMP         = NO\n"
        "HTML_DYNAMIC_SECTIONS  = NO\n"
        "HTML_INDEX_NUM_ENTRIES = 100\n"
        "GENERATE_DOCSET        = NO\n"
        "GENERATE_HTMLHELP      = NO\n"
        "GENERATE_QHP           = NO\n"
        "GENERATE_ECLIPSEHELP   = NO\n"
        "DISABLE_INDEX          = NO\n"
        "GENERATE_TREEVIEW      = NO\n"
        "ENUM_VALUES_PER_LINE   = 4\n"
        "TREEVIEW_WIDTH         = 250\n"
        "EXT_LINKS_IN_WINDOW    = NO\n"
        "FORMULA_FONTSIZE       = 10\n"
        "FORMULA_TRANSPARENT    = YES\n"
        "USE_MATHJAX            = NO\n"
        "SEARCHENGINE           = NO\n"
        "SERVER_BASED_SEARCH    = NO\n"
        "EXTERNAL_SEARCH        = NO\n"
        "SEARCHDATA_FILE        = searchdata.xml\n"
        "GENERATE_LATEX         = NO\n"
        "GENERATE_RTF           = NO\n"
        "GENERATE_MAN           = NO\n"
        "GENERATE_XML           = NO\n"
        "GENERATE_DOCBOOK       = NO\n"
        "GENERATE_AUTOGEN_DEF   = NO\n"
        "GENERATE_PERLMOD       = NO\n"
        "ENABLE_PREPROCESSING   = YES\n"
        "MACRO_EXPANSION        = YES\n"
        "EXPAND_ONLY_PREDEF     = YES\n"
        "SEARCH_INCLUDES        = YES\n"
        "PREDEFINED             = FOR_DOXYGEN_DOC_ONLY COMMS_MSVC_WARNING_PUSH= COMMS_MSVC_WARNING_POP= COMMS_MSVC_WARNING_DISABLE(x)=\n"
        "SKIP_FUNCTION_MACROS   = YES\n"
        "ALLEXTERNALS           = NO\n"
        "EXTERNAL_GROUPS        = YES\n"
        "EXTERNAL_PAGES         = YES\n"
        "CLASS_DIAGRAMS         = YES\n"
        "HIDE_UNDOC_RELATIONS   = YES\n"
        "HAVE_DOT               = NO\n"
        "#^#APPEND#$#\n"
        "\n";

    util::ReplacementMap repl = {
        {"PROJ_NAME", m_generator.currentSchema().schemaName()},
        {"APPEND", util::readFileContents(comms::inputCodePathForDoc(FileName, m_generator) + strings::appendFileSuffixStr())}
    };

    return writeFileInternal(FileName, util::processTemplate(Templ, repl), m_generator);
}

bool CommsDoxygen::commsWriteLayoutInternal() const
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

    return writeFileInternal(FileName, Templ, m_generator);
}

bool CommsDoxygen::commsWriteNamespacesInternal() const
{
    const std::string FileName = "namespaces.dox";

    util::StringsList elems;
    for (auto& s : m_generator.schemas()) {
        auto* commsSchema = static_cast<const CommsSchema*>(s.get());
        if ((s.get() != &m_generator.protocolSchema()) && (!commsSchema->commsHasAnyGeneratedCode())) {
            continue;
        }

        auto nsList = s->getAllNamespaces();

        util::StringsList nsElems;
        for (auto* ns : nsList) {
            static const std::string Templ =
                "/// @namespace #^#NS#$#\n"
                "/// @brief Protocol specific namespace.\n\n"
                "/// @namespace #^#NS#$#::message\n"
                "/// @brief Namespace for all the messages in #^#NS#$# namespace.\n\n"
                "/// @namespace #^#NS#$#::field\n"
                "/// @brief Namespace for all the stand alone fields defined in #^#NS#$# namespace.\n\n"
                "/// @namespace #^#NS#$#::frame\n"
                "/// @brief Namespace for all the frames defined in #^#NS#$# namespace.\n\n"
                "/// @namespace #^#NS#$#::frame::layer\n"
                "/// @brief Namespace for the custom frame layers defined in #^#NS#$# namespace.\n\n"
                "/// @namespace #^#NS#$#::frame::checksum\n"
                "/// @brief Namespace for the custom frame layers defined in #^#NS#$# namespace.\n\n";

            util::ReplacementMap repl = {
                {"NS", comms::scopeFor(*ns, m_generator)},
            };

            nsElems.push_back(util::processTemplate(Templ, repl));
        }

        static const std::string Templ = 
            "#^#MAIN#$#\n"
            "#^#NS_LIST#$#\n"
            "/// @namespace #^#NS#$#::options\n"
            "/// @brief Main namespace for the various protocol options.\n\n"
            "/// @namespace #^#NS#$#::input\n"
            "/// @brief Main namespace for hold input messages bundles.\n\n"
            "/// @namespace #^#NS#$#::dispatch\n"
            "/// @brief Main namespace for the various message dispatch functions.\n\n"        
        ;

        util::ReplacementMap repl = {
            {"NS_LIST", util::strListToString(nsElems, "", "")},
            {"NS", s->mainNamespace()},
        };

        bool hasDefaultNamespace = 
            std::any_of(
                nsList.begin(), nsList.end(),
                [](auto* ns)
                {
                    return ns->name().empty();
                });

        if (!hasDefaultNamespace) {
            repl["MAIN"] = 
                "/// @namespace " + s->mainNamespace() + "\n"
                "/// @brief Main namespace for all classes / functions of this protocol library.\n";
        }

        elems.push_back(util::processTemplate(Templ, repl));
    }

    const std::string Templ = {
        "#^#SCHEMAS#$#\n"
        "#^#APPEND#$#\n"
    };

    util::ReplacementMap repl = {
        {"SCHEMAS", util::strListToString(elems, "", "")},
        {"APPEND", util::readFileContents(comms::inputCodePathForDoc(FileName, m_generator) + strings::appendFileSuffixStr())}
    };

    return writeFileInternal(FileName, util::processTemplate(Templ, repl), m_generator);
}

bool CommsDoxygen::commsWriteMainpageInternal() const
{
    const std::string FileName = "main.dox";
    const std::string Templ =
        "/// @mainpage \"#^#PROJ_NAME#$#\" Binary Protocol Library\n"
        "/// @tableofcontents\n"
        "/// This generated code implements \"#^#PROJ_NAME#$#\" binary protocol using various\n"
        "/// classes from\n"
        "/// <a href=\"https://github.com/commschamp/comms\">COMMS Library</a>.@n\n"
        "/// Below is a short summary of generated classes.\n"
        "/// Please refer to <b>\"How to Use Defined Custom Protocol\"</b> page of its documentation\n"
        "/// for detailed explanation on how to use them.\n"
        "///\n"
        "/// @b NOTE, that the generated protocol code is mostly declarative statements\n"
        "/// of classes and types definitions. It is self explanatory and easy to read.\n"
        "/// In many cases it is easier to read and understand the generated code, than the\n"
        "/// doxygen generated documentation. Please spend some time on browsing around\n"
        "/// and looking at the generated code in addition to reading this documentation.\n"
        "///\n"
        "#^#MESSAGES_DOC#$#\n"
        "#^#FIELDS_DOC#$#\n"
        "#^#INTERFACE_DOC#$#\n"
        "#^#FRAME_DOC#$#\n"
        "#^#DISPATCH_DOC#$#\n"
        "#^#CUSTOMIZE_DOC#$#\n"
        "#^#VERSION_DOC#$#\n"
        "#^#APPEND#$#\n"
        "\n";

    util::ReplacementMap repl = {
        {"PROJ_NAME", m_generator.currentSchema().schemaName()},
        {"MESSAGES_DOC", commsMessagesDocInternal()},
        {"FIELDS_DOC", commsFieldsDocInternal()},
        {"INTERFACE_DOC", commsInterfaceDocInternal()},
        {"FRAME_DOC", commsFrameDocInternal()},
        {"DISPATCH_DOC", commsDispatchDocInternal()},
        {"CUSTOMIZE_DOC", commsCustomizeDocInternal()},
        {"VERSION_DOC", commsVersionDocInternal()},
        {"APPEND", util::readFileContents(comms::inputCodePathForDoc(FileName, m_generator) + strings::appendFileSuffixStr())}
    };

    return writeFileInternal(FileName, util::processTemplate(Templ, repl), m_generator);
}

std::string CommsDoxygen::commsMessagesDocInternal() const
{
    auto nsList = m_generator.getAllNamespaces();
    util::StringsList elems;
    for (auto* n : nsList) {
        static const std::string Templ = 
            "/// @li @ref #^#SCOPE#$#::#^#SUFFIX#$# (defined in @b #^#PATH#$#/#^#SUFFIX#$#  directory)";

        util::ReplacementMap repl = {
            {"SCOPE", comms::scopeFor(*n, m_generator)},
            {"PATH", util::strReplace(comms::scopeFor(*n, m_generator), "::", "/")},
            {"SUFFIX", strings::messageNamespaceStr()},
        };

        elems.push_back(util::processTemplate(Templ, repl));
    }

    const std::string Templ = 
        "/// @section main_messages Available Message Classes\n"
        "/// The following namespaces contain all the classes describing available messages:\n"
        "#^#LIST#$#\n"
        "///"
        ;    

    util::ReplacementMap repl = {
        {"LIST", util::strListToString(elems, "\n", "")}
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsDoxygen::commsFieldsDocInternal() const
{
    auto nsList = m_generator.getAllNamespaces();
    util::StringsList elems;
    for (auto* n : nsList) {
        static const std::string Templ = 
            "/// @li @ref #^#SCOPE#$#::#^#SUFFIX#$# (defined in @b #^#PATH#$#/#^#SUFFIX#$#  directory)";

        util::ReplacementMap repl = {
            {"SCOPE", comms::scopeFor(*n, m_generator)},
            {"PATH", util::strReplace(comms::scopeFor(*n, m_generator), "::", "/")},
            {"SUFFIX", strings::fieldNamespaceStr()},
        };

        elems.push_back(util::processTemplate(Templ, repl));
    }

    const std::string Templ = 
        "/// @section main_fields Available Common Fields\n"
        "/// The following namespaces contain all definition of all the fields,\n"
        "/// which can be references by multiple messages:\n"
        "#^#LIST#$#\n"
        "///"
        ;    

    util::ReplacementMap repl = {
        {"LIST", util::strListToString(elems, "\n", "")}
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsDoxygen::commsInterfaceDocInternal() const
{
    auto interfaces = m_generator.getAllInterfaces();
    assert(!interfaces.empty());

    util::StringsList list;
    for (auto* i : interfaces) {
        list.push_back(
            "/// @li @ref " + comms::scopeFor(*i, m_generator) +
            " (defined in @b " + comms::relHeaderPathFor(*i, m_generator) + " header file).");
    }

    static const std::string Templ = 
        "/// @section main_interfaces Common Interface Classes\n"
        "/// The available common interface classes are:\n"
        "#^#LIST#$#\n"
        "///";    

    util::ReplacementMap repl = {
        {"LIST", util::strListToString(list, "\n", "")}
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsDoxygen::commsFrameDocInternal() const
{
    const std::string Templ = 
        "/// @section main_frames Frame (Protocol Stack) Classes\n"
        "/// The available frame (protocol stack) classes are:\n"
        "#^#LIST#$#\n"
        "///\n"
        "/// Every frame class/type definition receives (as a template parameter) a list of\n"
        "/// @b input message types it is expected to recognize. Default defintion\n"
        "/// uses @ref #^#ALL_MESSAGES#$# (defined in @b #^#ALL_MESSAGES_HEADER#$#).\n"
        "/// @n If protocol defines any uni-directional message, then it is recommended to use\n"
        "/// either @ref #^#SERVER_MESSAGES#$# (from @b #^#SERVER_MESSAGES_HEADER#$#)\n"
        "/// or @ref #^#CLIENT_MESSAGES#$#  (from @b #^#CLIENT_MESSAGES_HEADER#$#)\n"
        "#^#PLATFORMS#$#\n"
        "/// @b NOTE, that the frame definition does not exactly follow the recommended\n"
        "/// instructions from <b>Protocol Stack Definition Tutorial</b> page of @b COMMS\n"
        "/// library documentation. The extra customization (see @ref main_customization below)\n"
        "/// is performed by passing options to the layers themselves.\n"
        "///\n"
        "/// The available bundles of input messages are:\n"
        "#^#MESSAGES_LIST#$#\n"
        "///";

    auto frames = m_generator.getAllFrames();
    assert(!frames.empty());

    util::StringsList list;
    for (auto* f : frames) {
        list.push_back(
            "/// @li @ref " + comms::scopeFor(*f, m_generator) +
            " (from @b " + comms::relHeaderPathFor(*f, m_generator) + " header file).");
    }

    util::StringsList messagesList;
    auto addToMessagesListFunc =
        [this, &messagesList](const std::string& name)
        {
            auto scope = comms::scopeForInput(name, m_generator);
            auto file = comms::relHeaderForInput(name, m_generator);
            auto str = "/// @li @ref " + scope + " (from @b " + file + " header file).";
            messagesList.push_back(std::move(str));
        };

    addToMessagesListFunc(strings::allMessagesStr());
    addToMessagesListFunc(ServerInputMessagesStr);
    addToMessagesListFunc(ClientInputMessagesStr);

    auto& platforms = m_generator.currentSchema().platformNames();
    for (auto& p : platforms) {
        addToMessagesListFunc(p + "Messages");
        addToMessagesListFunc(p + ServerInputMessagesStr);
        addToMessagesListFunc(p + ClientInputMessagesStr);
    };

    util::ReplacementMap repl = {
        {"LIST", util::strListToString(list, "\n", "")},
        {"MESSAGES_LIST", util::strListToString(messagesList, "\n", "")},
        {"PROT_NAMESPACE", m_generator.currentSchema().mainNamespace()},
        {"PLATFORMS", commsPlatformsDocInternal()},
        {"ALL_MESSAGES", comms::scopeForInput(strings::allMessagesStr(), m_generator)},
        {"ALL_MESSAGES_HEADER", comms::relHeaderForInput(strings::allMessagesStr(), m_generator)},
        {"SERVER_MESSAGES", comms::scopeForInput(ServerInputMessagesStr, m_generator)},
        {"SERVER_MESSAGES_HEADER", comms::relHeaderForInput(ServerInputMessagesStr, m_generator)},
        {"CLIENT_MESSAGES", comms::scopeForInput(ClientInputMessagesStr, m_generator)},
        {"CLIENT_MESSAGES_HEADER", comms::relHeaderForInput(ClientInputMessagesStr, m_generator)},
    };

    return util::processTemplate(Templ, repl);     
}

std::string CommsDoxygen::commsDispatchDocInternal() const
{
    const std::string Templ =
        "/// @section main_dispatch Dispatching Message Objects\n"
        "/// While the @b COMMS library provides various built-in ways of \n"
        "/// dispatching message objects into their respective handlers\n"
        "/// (see <b>Advanced Guide to Message Dispatching</b> page of the\n"
        "/// @b COMMS library documentation), the generated code also provides\n"
        "/// additional auxiliary dispatch functions which are @b switch statement\n"
        "/// based.\n"
        "///\n"
        "/// The available functions are:\n"
        "#^#LIST#$#\n"
        "///\n"
        "/// There are also some dispatcher objects to be used with\n"
        "/// @b comms::processAllWithDispatchViaDispatcher() function or similar.\n"
        "#^#DISPATCHERS_LIST#$#\n"
        "///";

    util::StringsList list;
    util::StringsList dispatcherList;
    auto addToListFunc =
        [this, &list, &dispatcherList](const std::string& name)
        {
            static const std::string Prefix("Dispatch");
            static const std::string Suffix("Message");

            auto accessName = comms::accessName(Prefix + name + Suffix);
            auto accessScope = comms::scopeForDispatch(accessName, m_generator);
            auto fileName = comms::className(accessName);
            auto file = comms::relHeaderForDispatch(fileName, m_generator);
            auto str = "/// @li @ref " + accessScope + 
                "()\n/// (defined in @b " + file + " header file).";
            list.push_back(std::move(str));
            auto defaultOptStr = "/// @li @ref " + accessScope + 
                strings::defaultOptionsStr() + "()\n/// (defined in @b " + file + " header file).";
            list.push_back(std::move(defaultOptStr));

            static const std::string DispatcherSuffix("MsgDispatcher");
            auto dispatcherName = comms::className(name) + DispatcherSuffix;
            auto dispatcherScope = comms::scopeForDispatch(dispatcherName, m_generator);
            auto dispatcherDefaultScope = dispatcherScope + strings::defaultOptionsStr();
            auto dispatcherStr =
                "/// @li @ref " + dispatcherScope + "\n/// (defined in @b " + file + " header file).";
            dispatcherList.push_back(std::move(dispatcherStr));
            auto dispatcherDefaultOptionsStr =
                "/// @li @ref " + dispatcherDefaultScope + "\n/// (defined in @b " + file + " header file).";
            dispatcherList.push_back(std::move(dispatcherDefaultOptionsStr));
        };

    auto addPlatformFunc =
        [&addToListFunc](const std::string& platform)
        {
            addToListFunc(platform);
            addToListFunc(platform + ServerInputPrefixStr);
            addToListFunc(platform + ClientInputPrefixStr);
        };

    addPlatformFunc(strings::emptyString());
    for (auto& p : m_generator.currentSchema().platformNames()) {
        addPlatformFunc(comms::className(p));
    }

    util::ReplacementMap repl = {
        {"LIST", util::strListToString(list, "\n", "")},
        {"DISPATCHERS_LIST", util::strListToString(dispatcherList, "\n", "")},
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsDoxygen::commsCustomizeDocInternal() const
{
    const std::string Templ = 
        "/// @section main_customization Customization\n"
        "/// Depending on the value of @b customization option passed to the @b commsdsl2comms\n"
        "/// code generator, the latter generates @ref #^#OPTIONS#$#\n"
        "/// struct (defined in @b #^#OPTIONS_HDR#$# file),\n"
        "/// which is used by default throughout the protocol definition classes.\n"
        "/// The struct contains all the available type definition, which can be used to\n"
        "/// customize default data structures and/or behaviour of various classes.\n"
        "/// If any additional customization is required, just recreate similar struct with\n"
        "/// relevant types overriden with new definition. The easiest way is to extend\n"
        "/// the #^#OPTIONS#$#. For example:\n"
        "/// @code\n"
        "/// struct MyOptions : public #^#OPTIONS#$#\n"
        "/// {\n"
        "///     struct field : public #^#OPTIONS#$#::field\n"
        "///     {\n"
        "///         // use comms::util::StaticString as storage type\n"
        "///         using SomeStringField = comms::option::app::FixedSizeStorage<32>;\n"
        "///     };\n"
        "/// };\n"
        "/// @endcode\n"
        "/// @b NOTE, that inner scope of structs in the #^#OPTIONS#$#\n"
        "/// resembles scope of namespaces used in protocol definition.\n" 
        "///\n"
        "/// The @b COMMS library also provides a flexible way to configure polymorphic\n"
        "/// interface for the message classes. If the defined protocol has multiple\n"
        "/// uni-directional messages (marked using @b sender property in the schema),\n"
        "/// then it is recommended to define two separate interfaces: one for input,\n"
        "/// other for output:\n"
        "/// @code\n"
        "/// using MyInputMsg =\n"
        "///    #^#INTERFACE#$#<\n"
        "///        comms::option::app::ReadIterator<const std::uint8_t*>, // for polymorphic read\n"
        "///        comms::option::app::Handler<MyHandler> // for polymorphic dispatch\n"
        "///    >;\n"
        "///\n"
        "/// using MyOutputMsg =\n"
        "///    #^#INTERFACE#$#<\n"
        "///        comms::option::app::WriteIterator<std::uint8_t*>, // for polymorphic write\n"
        "///        comms::option::app::LengthInfoInterface, // for polymorphic serialization length retrieval\n"
        "///        comms::option::app::IdInfoInterface // for polymorphic message ID retrieval\n"
        "///    >;\n"
        "/// @endcode\n"
        "/// In case there are only few uni-directional messages, it may make sence to define\n"
        "/// single interface class:\n"
        "/// @code\n"
        "/// using MyMsg =\n"
        "///    #^#INTERFACE#$#<\n"
        "///        comms::option::app::ReadIterator<const std::uint8_t*>, // for polymorphic read\n"
        "///        comms::option::app::Handler<MyHandler> // for polymorphic dispatch\n"
        "///        comms::option::app::WriteIterator<std::uint8_t*>, // for polymorphic write\n"
        "///        comms::option::app::LengthInfoInterface, // for polymorphic serialization length retrieval\n"
        "///        comms::option::app::IdInfoInterface // for polymorphic message ID retrieval\n"
        "///    >;\n"
        "/// @endcode\n"
        "/// In this case the code generator may also define @b #^#SERVER_OPTIONS#$#\n"
        "/// (defined in @b #^#SERVER_OPTIONS_HDR#$# file) and\n"
        "/// @b #^#CLIENT_OPTIONS#$# (defined in @b #^#CLIENT_OPTIONS_HDR#$# file).\n"
        "/// These structs suppress generation of unnecessary virtual functions which are not\n"
        "/// going to be used. Consider using this structs as options instead of default\n"
        "/// #^#OPTIONS#$#.\n"
        "///\n"
        "/// Also there is @ref #^#BARE_METAL_OPTIONS#$#\n"
        "/// (defined in @b #^#BARE_METAL_OPTIONS_HDR#$# file) which can help in defining\n"
        "/// options for bare-metal applications. It exclude all usage of dynamic memory allocation.\n"
        "///\n"
        "/// In case non-custom &lt;id&gt; layer has been used in schema (files), custom,\n"
        "/// application-specific allocation options to it may include\n"
        "/// @b comms::option::app::InPlaceAllocation and/or @b comms::option::app::SupportGenericMessage.\n"
        "/// Please see the documentation of the @b COMMS library itself for more details.\n"
        "///\n"
        "/// In many cases the input buffer is sequential (not circular), where the end of message payload\n"
        "/// never precedes its beginning and the processing is implemented in a way where message objects\n"
        "/// never outlive the input buffer. In such scenarios it could be a waste of memory / CPU cycles to\n"
        "/// copy bytes from the input buffer to internal storage of the fields like &lt;string&gt;\n"
        "/// (@b comms::field::String) and/or &lt;data&gt; (@b comms::field::ArrayList of raw bytes).\n"
        "/// The generated code also provides #^#DATA_VIEW_OPTIONS#$# (defined in\n"
        "/// @b #^#DATA_VIEW_OPTIONS_HDR#$# file) where relevant fields apply @b comms::option::app::OrigDataView\n"
        "/// option.\n"
        "///\n"
        "/// Also note that the specified extension options are implemented as the following template classes\n"
        "/// which receive other options as their base class and apply relevant changes on top.\n"
        "/// @li @ref #^#CLIENT_OPTIONS#$#T\n"
        "/// @li @ref #^#SERVER_OPTIONS#$#T\n"
        "/// @li @ref #^#BARE_METAL_OPTIONS#$#T\n"
        "/// @li @ref #^#DATA_VIEW_OPTIONS#$#T\n"
        "///\n"
        "/// As the result it is possible to combine them. For example:\n"
        "/// @code\n"
        "/// using MyOptions=\n"
        "///     #^#DATA_VIEW_OPTIONS#$#T<\n"
        "///         #^#CLIENT_OPTIONS#$#\n"
        "///     >;\n"
        "/// @endcode"
        ;

    auto allInterfaces = m_generator.getAllInterfaces();
    assert(!allInterfaces.empty());

    util::ReplacementMap repl = {
        {"INTERFACE", comms::scopeFor(*allInterfaces.front(), m_generator)},
        {"PROT_NAMESPACE", m_generator.currentSchema().mainNamespace()},
        {"OPTIONS", comms::scopeForOptions(strings::defaultOptionsStr(), m_generator)},
        {"CLIENT_OPTIONS", comms::scopeForOptions("Client" + strings::defaultOptionsStr(), m_generator)},
        {"SERVER_OPTIONS", comms::scopeForOptions("Server" + strings::defaultOptionsStr(), m_generator)},
        {"OPTIONS_HDR", comms::relHeaderForOptions(strings::defaultOptionsStr(), m_generator)},
        {"CLIENT_OPTIONS_HDR", comms::relHeaderForOptions("Client" + strings::defaultOptionsStr(), m_generator)},
        {"SERVER_OPTIONS_HDR", comms::relHeaderForOptions("Server" + strings::defaultOptionsStr(), m_generator)},
        {"BARE_METAL_OPTIONS", comms::scopeForOptions(strings::bareMetalStr() + strings::defaultOptionsStr(), m_generator)},
        {"BARE_METAL_OPTIONS_HDR", comms::relHeaderForOptions(strings::bareMetalStr() + strings::defaultOptionsStr(), m_generator)},
        {"DATA_VIEW_OPTIONS", comms::scopeForOptions(strings::dataViewStr() + strings::defaultOptionsStr(), m_generator)},
        {"DATA_VIEW_OPTIONS_HDR", comms::relHeaderForOptions(strings::dataViewStr() + strings::defaultOptionsStr(), m_generator)},
    };

    return util::processTemplate(Templ, repl);
}

std::string CommsDoxygen::commsVersionDocInternal() const
{
    if (!m_generator.currentSchema().versionDependentCode()) {
        return strings::emptyString();
    }

    const std::string Templ = 
        "///\n"
        "/// @section main_version Version Dependent Code\n"
        "/// The generated code is version dependent. The version information is stored in\n"
        "/// one of the fields held by a common interface class (see @ref main_interfaces).\n"
        "/// When presence of the field depends on the protocol version, it is defined as\n"
        "/// @b comms::field::Optional field which wraps a proper field definition. Please\n"
        "/// remember to use @b field() member function to access the wrapped field, before\n"
        "/// using @b value() member function to access the actual value.\n"
        "/// @code\n"
        "/// void handle(SomeMessage& msg)\n"
        "/// {\n"
        "///     auto& versionDependentField = msg.field_someVersionDependentField();\n"
        "///     auto& wrappedField = versionDependentField.field();\n"
        "///     auto actualValue = wrappedField.value();\n"
        "///     ...\n"
        "/// }\n"
        "/// @endcode\n"
        "/// Every default constructed message will have a version of the schema with\n"
        "/// all relevant fields being marked present/missing based on this version information.\n"
        "/// If there is a need to send a message with different protocol version information,\n"
        "/// the message needs to be brought into a consistent state by calling its @b doRefresh()\n"
        "/// member function (or @b refresh() in case of proper polymorphic behavior has been enabled)\n"
        "/// after version information has been updated.\n"
        "/// @code\n"
        "///     #^#PROT_NAMESPACE#$#::message::SomeMsg<MyOutputMsg> msg;\n"
        "///     msg.version() = 4U;\n"
        "///     msg.doRefresh(); // will update exists/missing state of every dependent field\n"
        "/// @endcode";

    util::ReplacementMap repl = {
        {"PROT_NAMESPACE", m_generator.currentSchema().mainNamespace()},
    };
    return util::processTemplate(Templ, repl);
}

std::string CommsDoxygen::commsPlatformsDocInternal() const
{
    auto platforms = m_generator.currentSchema().platformNames();
    if (platforms.empty()) {
        return strings::emptyString();
    }

    const std::string Templ = 
        "///\n"
        "/// There are also platform specific definitions:\n"
        "#^#LIST#$#\n"
        "///";

    util::StringsList list;
    for (auto& p : platforms) {
        auto addToListFunc = 
            [this, &p, &list](const std::string& type)
            {
                auto name = comms::className(p) + type + "InputMessages";
                auto scope = comms::scopeForInput(name, m_generator);
                auto file = comms::relHeaderForInput(name, m_generator);
                auto str = "/// @li @ref " + scope + " (from @b " + file + ").";
                list.push_back(std::move(str));
            };

        addToListFunc("Server");
        addToListFunc("Client");
    }

    util::ReplacementMap repl = {
        {"LIST", util::strListToString(list, "\n", "")}
    };
    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2comms
