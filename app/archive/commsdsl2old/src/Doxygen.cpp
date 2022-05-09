//
// Copyright 2018 - 2021 (C). Alex Robenko. All rights reserved.
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

#include "Doxygen.h"

#include <fstream>
#include <vector>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "Generator.h"
#include "common.h"

namespace bf = boost::filesystem;
namespace ba = boost::algorithm;

namespace commsdsl2old
{

namespace
{

std::vector<std::string> getAppendReq(const std::string& file)
{
    std::vector<std::string> result;
    result.push_back(common::docStr());
    result.push_back(file);
    return result;
}

} // namespace 

bool Doxygen::write(Generator& generator)
{
    Doxygen obj(generator);
    return
        obj.writeConf() &&
        obj.writeLayout() &&
        obj.writeNamespaces() &&
        obj.writeMainpage();
}

bool Doxygen::writeConf() const
{
    static const std::string DocFile("doxygen.conf");
    auto filePath = m_generator.startProtocolDocWrite(DocFile);

    if (filePath.empty()) {
        return true;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Template = 
        "DOXYFILE_ENCODING      = UTF-8\n"
        "PROJECT_NAME           = \"#^#PROJ_NAME#$#\"\n"
        "PROJECT_BRIEF          = \"Documentation for generated code of \\\"#^#PROJ_NAME#$#\\\" protocol.\"\n"
        "OUTPUT_DIRECTORY       = \n"
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
        "EXTRACT_ALL            = NO\n"
        "EXTRACT_PRIVATE        = NO\n"
        "EXTRACT_PACKAGE        = NO\n"
        "EXTRACT_STATIC         = NO\n"
        "EXTRACT_LOCAL_CLASSES  = YES\n"
        "EXTRACT_LOCAL_METHODS  = NO\n"
        "EXTRACT_ANON_NSPACES   = NO\n"
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
        "EXCLUDE                = cc_plugin\n"
        "EXCLUDE_SYMLINKS       = NO\n"
        "EXCLUDE_PATTERNS       = */cc_plugin/* */install/*\n"
        "EXCLUDE_SYMBOLS        = *details *cc_plugin\n"
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
        "CLANG_OPTIONS          =\n"
        "ALPHABETICAL_INDEX     = YES\n"
        "COLS_IN_ALPHA_INDEX    = 5\n"
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

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PROJ_NAME", m_generator.schemaName()));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForFile(getAppendReq(DocFile))));

    stream << common::processTemplate(Template, replacements);

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    return true;
}

bool Doxygen::writeLayout() const
{
    auto filePath = m_generator.startProtocolDocWrite("layout.xml");

    if (filePath.empty()) {
        return true;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Str =
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

    stream << Str;

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    return true;
}

bool Doxygen::writeNamespaces() const
{
    static const std::string DocFile("namespaces.dox");
    auto filePath = m_generator.startProtocolDocWrite(DocFile);

    if (filePath.empty()) {
        return true;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Template =
        "/// @namespace #^#NS#$#\n"
        "/// @brief Main namespace for all classes / functions of this protocol library.\n\n"
        "/// @namespace #^#NS#$#::message\n"
        "/// @brief Main namespace for all the messages defined in this protocol.\n\n"
        "/// @namespace #^#NS#$#::field\n"
        "/// @brief Main namespace for all the stand alone fields defined in this protocol.\n\n"
        "/// @namespace #^#NS#$#::frame\n"
        "/// @brief Main namespace for all the frames.\n\n"
        "/// @namespace #^#NS#$#::frame::layer\n"
        "/// @brief Main namespace for the custom frame layers.\n\n"
        "/// @namespace #^#NS#$#::frame::checksum\n"
        "/// @brief Main namespace for the custom frame layers.\n\n"
        "/// @namespace #^#NS#$#::options\n"
        "/// @brief Main namespace for the various protocol options.\n\n"
        "/// @namespace #^#NS#$#::input\n"
        "/// @brief Main namespace for hold input messages bundles.\n\n"
        "/// @namespace #^#NS#$#::dispatch\n"
        "/// @brief Main namespace for the various message dispatch functions.\n\n"
        "#^#OTHER_NS#$#\n"
        "#^#APPEND#$#\n"
        ;

    common::StringsList otherNs;
    auto scopes = m_generator.getNonDefaultNamespacesScopes();
    for (auto& s : scopes) {
        static const std::string Templ =
            "/// @namespace #^#NS#$#\n"
            "/// @brief Additional protocol specific namespace.\n\n"
            "/// @namespace #^#NS#$#::message\n"
            "/// @brief Namespace for all the messages in @ref #^#NS#$# namespace.\n\n"
            "/// @namespace #^#NS#$#::field\n"
            "/// @brief Namespace for all the stand alone fields defined in @ref #^#NS#$# namespace.\n\n"
            "/// @namespace #^#NS#$#::frame\n"
            "/// @brief Namespace for all the frames defined in @ref #^#NS#$# namespace.\n\n"
            "/// @namespace #^#NS#$#::frame::layer\n"
            "/// @brief Namespace for the custom frame layers defined in @ref #^#NS#$# namespace.\n\n"
            "/// @namespace #^#NS#$#::frame::checksum\n"
            "/// @brief Namespace for the custom frame layers defined in @ref #^#NS#$# namespace.\n\n"
            "/// @namespace #^#NS#$#::options\n"
            "/// @brief Namespace for the various protocol options defined in @ref #^#NS#$# namespace.\n\n"
            "/// @namespace #^#NS#$#::input\n"
            "/// @brief Namespace for hold input messages bundles defined in @ref #^#NS#$# namespace.\n\n"
            "/// @namespace #^#NS#$#::dispatch\n"
            "/// @brief Namespace for the various message dispatch functionss defined in @ref #^#NS#$# namespace.\n\n"
            ;

        common::ReplacementMap repl;
        repl.insert(std::make_pair("NS", s));
        otherNs.push_back(common::processTemplate(Templ, repl));
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("NS", m_generator.mainNamespace()));
    replacements.insert(std::make_pair("OTHER_NS", common::listToString(otherNs, "\n", common::emptyString())));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForFile(getAppendReq(DocFile))));

    stream << common::processTemplate(Template, replacements);

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    return true;
}

bool Doxygen::writeMainpage() const
{
    static const std::string DocFile("main.dox");
    auto filePath = m_generator.startProtocolDocWrite(DocFile);

    if (filePath.empty()) {
        return true;
    }

    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Template =
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

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PROJ_NAME", m_generator.schemaName()));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForFile(getAppendReq(DocFile))));
    replacements.insert(std::make_pair("MESSAGES_DOC", getMessagesDoc()));
    replacements.insert(std::make_pair("FIELDS_DOC", getFieldsDoc()));
    replacements.insert(std::make_pair("INTERFACE_DOC", getInterfacesDoc()));
    replacements.insert(std::make_pair("FRAME_DOC", getFramesDoc()));
    replacements.insert(std::make_pair("DISPATCH_DOC", getDispatchDoc()));
    replacements.insert(std::make_pair("CUSTOMIZE_DOC", getCustomizeDoc()));
    replacements.insert(std::make_pair("VERSION_DOC", getVersionDoc()));

    stream << common::processTemplate(Template, replacements);

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    return true;
}

std::string Doxygen::getMessagesDoc() const
{
    static const std::string Templ = 
        "/// @section main_messages Available Message Classes\n"
        "/// The following namespaces contain all the classes describing available messages:\n"
        "#^#LIST#$#\n"
        "///"
        ;

    common::StringsList nsList;
    auto addToListFunc = 
        [&nsList](const std::string& str)
        {
            auto dir = ba::replace_all_copy(str, "::", "/");
            nsList.push_back(
                "/// @li @ref " + str + " (residing in @b " + dir + " directory)."
            );
        };

    addToListFunc(m_generator.mainNamespace() + "::" + common::messageStr());

    auto scopes = m_generator.getNonDefaultNamespacesScopes();
    for (auto& s : scopes) {
        addToListFunc(s + "::" + common::messageStr());
    }        

    common::ReplacementMap repl;
    repl.insert(std::make_pair("LIST", common::listToString(nsList, "\n", common::emptyString())));
    return common::processTemplate(Templ, repl);
}

std::string Doxygen::getFieldsDoc() const
{
    static const std::string Templ = 
        "/// @section main_fields Available Common Fields\n"
        "/// The following namespaces contain all definition of all the fields,\n"
        "/// which can be references by multiple messages:\n"
        "#^#LIST#$#\n"
        "///"
        ;

    common::StringsList nsList;
    auto addToListFunc = 
        [&nsList](const std::string& str)
        {
            auto dir = ba::replace_all_copy(str, "::", "/");
            nsList.push_back(
                "/// @li @ref " + str + " (residing in @b " + dir + " directory)."
            );
        };

    addToListFunc(m_generator.mainNamespace() + "::" + common::fieldStr());

    auto scopes = m_generator.getNonDefaultNamespacesScopes();
    for (auto& s : scopes) {
        addToListFunc(s + "::" + common::fieldStr());
    }        

    common::ReplacementMap repl;
    repl.insert(std::make_pair("LIST", common::listToString(nsList, "\n", common::emptyString())));
    return common::processTemplate(Templ, repl);
}

std::string Doxygen::getInterfacesDoc() const
{
    static const std::string Templ = 
        "/// @section main_interfaces Common Interface Classes\n"
        "/// The available common interface classes are:\n"
        "#^#LIST#$#\n"
        "///";

    auto interfaces = m_generator.getAllInterfaces();
    assert(!interfaces.empty());

    common::StringsList list;
    for (auto& i : interfaces) {
        auto scope = m_generator.scopeForInterface(i->externalRef(), true, true);
        auto file = ba::replace_all_copy(scope, "::", "/") + common::headerSuffix();
        list.push_back(
            "/// @li @ref " + scope +
            " from @b " + file + " header file).");
    }

    common::ReplacementMap repl;
    repl.insert(std::make_pair("LIST", common::listToString(list, "\n", common::emptyString())));
    return common::processTemplate(Templ, repl);        
}

std::string Doxygen::getFramesDoc() const
{
    static const std::string Templ = 
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

    common::StringsList list;
    for (auto& f : frames) {
        auto scope = m_generator.scopeForFrame(f->externalRef(), true, true);
        auto file = ba::replace_all_copy(scope, "::", "/") + common::headerSuffix();
        list.push_back(
            "/// @li @ref " + scope +
            " (from @b " + file + " header file).");
    }

    common::StringsList messagesList;
    auto addToMessagesListFunc =
        [this, &messagesList](const std::string& name)
        {
            auto scope = m_generator.scopeForInput(name, true, true);
            auto file = m_generator.headerfileForInput(name, false);
            auto str = "/// @li @ref " + scope + " (from @b " + file + " header file).";
            messagesList.push_back(std::move(str));
        };

    addToMessagesListFunc(common::allMessagesStr());
    addToMessagesListFunc(common::serverInputMessagesStr());
    addToMessagesListFunc(common::clientInputMessagesStr());

    auto& platforms = m_generator.platforms();
    for (auto& p : platforms) {
        addToMessagesListFunc(p + "Messages");
        addToMessagesListFunc(p + common::serverInputMessagesStr());
        addToMessagesListFunc(p + common::clientInputMessagesStr());
    };


    common::ReplacementMap repl;
    repl.insert(std::make_pair("LIST", common::listToString(list, "\n", common::emptyString())));
    repl.insert(std::make_pair("MESSAGES_LIST", common::listToString(messagesList, "\n", common::emptyString())));
    repl.insert(std::make_pair("PROT_NAMESPACE", m_generator.mainNamespace()));
    repl.insert(std::make_pair("PLATFORMS", getPlatformsDoc()));
    repl.insert(std::make_pair("ALL_MESSAGES", m_generator.scopeForInput(common::allMessagesStr(), true, true)));
    repl.insert(std::make_pair("ALL_MESSAGES_HEADER", m_generator.headerfileForInput(common::allMessagesStr(), false)));
    repl.insert(std::make_pair("SERVER_MESSAGES", m_generator.scopeForInput(common::serverInputMessagesStr(), true, true)));
    repl.insert(std::make_pair("SERVER_MESSAGES_HEADER", m_generator.headerfileForInput(common::serverInputMessagesStr(), false)));
    repl.insert(std::make_pair("CLIENT_MESSAGES", m_generator.scopeForInput(common::clientInputMessagesStr(), true, true)));
    repl.insert(std::make_pair("CLIENT_MESSAGES_HEADER", m_generator.headerfileForInput(common::clientInputMessagesStr(), false)));

    return common::processTemplate(Templ, repl);        
}

std::string Doxygen::getDispatchDoc() const
{
    static const std::string Templ =
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

    common::StringsList list;
    common::StringsList dispatcherList;
    auto addToListFunc =
        [this, &list, &dispatcherList](const std::string& name)
        {
            static const std::string Prefix("Dispatch");
            static const std::string Suffix("Message");

            auto adjustedName = common::nameToAccessCopy(Prefix + name + Suffix);
            auto fileName = common::nameToClassCopy(adjustedName);
            auto scope = m_generator.scopeForDispatch(name, true, false) + adjustedName;
            auto defaultScope = scope + common::defaultOptionsStr();
            auto file = m_generator.headerfileForDispatch(fileName, false);
            auto str = "/// @li @ref " + scope + "\n/// (defined in @b " + file + " header file).";
            list.push_back(std::move(str));
            auto defaultOptStr = "/// @li @ref " + defaultScope + "\n/// (defined in @b " + file + " header file).";
            list.push_back(std::move(defaultOptStr));

            static const std::string DispatcherSuffix("MsgDispatcher");
            auto dispatcherName = common::nameToClassCopy(name) + DispatcherSuffix;
            auto dispatcherScope = m_generator.scopeForDispatch(dispatcherName, true, true);
            auto dispatcherDefaultScope = dispatcherScope + common::defaultOptionsStr();
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
            addToListFunc(platform + common::serverInputStr());
            addToListFunc(platform + common::clientInputStr());
        };

    addPlatformFunc(common::emptyString());
    for (auto& p : m_generator.platforms()) {
        addPlatformFunc(common::nameToClassCopy(p));
    }

    common::ReplacementMap repl;
    repl.insert(std::make_pair("LIST", common::listToString(list, "\n", common::emptyString())));
    repl.insert(std::make_pair("DISPATCHERS_LIST", common::listToString(dispatcherList, "\n", common::emptyString())));
    return common::processTemplate(Templ, repl);
}

std::string Doxygen::getPlatformsDoc() const
{
    auto platforms = m_generator.platforms();
    if (platforms.empty()) {
        return common::emptyString();
    }

    static const std::string Templ = 
        "///\n"
        "/// There are also platform specific definitions:\n"
        "#^#LIST#$#\n"
        "///";

    common::StringsList list;
    for (auto& p : platforms) {
        auto addToListFunc = 
            [this, &p, &list](const std::string& type)
            {
                auto name = common::nameToClassCopy(p) + type + "InputMessages";
                auto scope = m_generator.scopeForInput(name, true, true);
                auto file = m_generator.headerfileForInput(name, false);
                auto str = "/// @li @ref " + scope + " (from @b " + file + ").";
                list.push_back(std::move(str));
            };

        addToListFunc("Server");
        addToListFunc("Client");
    }

    common::ReplacementMap repl;
    repl.insert(std::make_pair("LIST", common::listToString(list, "\n", common::emptyString())));
    return common::processTemplate(Templ, repl);        
}

std::string Doxygen::getCustomizeDoc() const
{
    static const std::string Templ = 
        "/// @section main_customization Customization\n"
        "/// Depending on the value of @b customization option passed to the @b commsdsl2old\n"
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

    common::ReplacementMap repl;
    repl.insert(std::make_pair("INTERFACE", m_generator.scopeForInterface(allInterfaces.front()->externalRef(), true, true)));
    repl.insert(std::make_pair("PROT_NAMESPACE", m_generator.mainNamespace()));
    repl.insert(std::make_pair("OPTIONS", m_generator.scopeForOptions(common::defaultOptionsStr(), true, true)));
    repl.insert(std::make_pair("CLIENT_OPTIONS", m_generator.scopeForOptions("Client" + common::defaultOptionsStr(), true, true)));
    repl.insert(std::make_pair("SERVER_OPTIONS", m_generator.scopeForOptions("Server" + common::defaultOptionsStr(), true, true)));
    repl.insert(std::make_pair("OPTIONS_HDR", m_generator.headerfileForOptions(common::defaultOptionsStr(), false)));
    repl.insert(std::make_pair("CLIENT_OPTIONS_HDR", m_generator.headerfileForOptions("Client" + common::defaultOptionsStr(), false)));
    repl.insert(std::make_pair("SERVER_OPTIONS_HDR", m_generator.headerfileForOptions("Server" + common::defaultOptionsStr(), false)));
    repl.insert(std::make_pair("BARE_METAL_OPTIONS", m_generator.scopeForOptions(common::bareMetalStr() + common::defaultOptionsStr(), true, true)));
    repl.insert(std::make_pair("BARE_METAL_OPTIONS_HDR", m_generator.headerfileForOptions(common::bareMetalStr() + common::defaultOptionsStr(), false)));
    repl.insert(std::make_pair("DATA_VIEW_OPTIONS", m_generator.scopeForOptions(common::dataViewStr() + common::defaultOptionsStr(), true, true)));
    repl.insert(std::make_pair("DATA_VIEW_OPTIONS_HDR", m_generator.headerfileForOptions(common::dataViewStr() + common::defaultOptionsStr(), false)));

    return common::processTemplate(Templ, repl);
}

std::string Doxygen::getVersionDoc() const
{
    if (!m_generator.versionDependentCode()) {
        return common::emptyString();
    }

    static const std::string Templ = 
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

    common::ReplacementMap repl;
    repl.insert(std::make_pair("PROT_NAMESPACE", m_generator.mainNamespace()));
    return common::processTemplate(Templ, repl);
}

} // namespace commsdsl2old