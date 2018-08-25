#include "Doxygen.h"

#include <fstream>

#include <boost/filesystem.hpp>

#include "Generator.h"
#include "common.h"

namespace bf = boost::filesystem;

namespace commsdsl2comms
{

bool Doxygen::write(Generator& generator)
{
    Doxygen obj(generator);
    return
        obj.writeConf() &&
        obj.writeLayout() &&
        obj.writeNamespaces();
}

bool Doxygen::writeConf() const
{
    auto filePath = m_generator.startProtocolDocWrite("doxygen.conf");

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
        "SORT_BRIEF_DOCS        = NO\n"
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
        "HTML_TIMESTAMP         = YES\n"
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
        "MACRO_EXPANSION        = NO\n"
        "EXPAND_ONLY_PREDEF     = NO\n"
        "SEARCH_INCLUDES        = YES\n"
        "PREDEFINED             = FOR_DOXYGEN_DOC_ONLY\n"
        "SKIP_FUNCTION_MACROS   = YES\n"
        "ALLEXTERNALS           = NO\n"
        "EXTERNAL_GROUPS        = YES\n"
        "EXTERNAL_PAGES         = YES\n"
        "PERL_PATH              = /usr/bin/perl\n"
        "CLASS_DIAGRAMS         = YES\n"
        "HIDE_UNDOC_RELATIONS   = YES\n"
        "HAVE_DOT               = NO\n\n";

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PROJ_NAME", m_generator.schemaName()));

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
    auto filePath = m_generator.startProtocolDocWrite("namespaces.dox");

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
        "#^#OTHER_NS#$#\n"
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
            "/// @brief Namespace for the custom frame layers defined in @ref #^#NS#$# namespace.\n"
            ;

        common::ReplacementMap repl;
        repl.insert(std::make_pair("NS", s));
        otherNs.push_back(common::processTemplate(Templ, repl));
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("NS", m_generator.mainNamespace()));
    replacements.insert(std::make_pair("OTHER_NS", common::listToString(otherNs, "\n", common::emptyString())));

    stream << common::processTemplate(Template, replacements);

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    return true;
}

} // namespace commsdsl2comms
