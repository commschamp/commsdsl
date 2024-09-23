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

#include "ToolsQtCmake.h"

#include "ToolsQtGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2tools_qt
{

namespace 
{

using ReplacementMap = commsdsl::gen::util::ReplacementMap;

} // namespace 
    

bool ToolsQtCmake::write(ToolsQtGenerator& generator)
{
    ToolsQtCmake obj(generator);
    return obj.testWriteInternal();
}

bool ToolsQtCmake::testWriteInternal() const
{
    auto filePath = 
        util::pathAddElem(
            m_generator.getOutputDir(), strings::cmakeListsFileStr());    

    m_generator.logger().info("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    static const std::string Template =
        "cmake_minimum_required (VERSION 3.10)\n"
        "project (\"#^#MAIN_NS#$#_cc_tools_qt_plugin\")\n\n"
        "# Build options:\n"
        "option (OPT_WARN_AS_ERR \"Treat warnings as errors\" ON)\n"
        "option (OPT_USE_CCACHE \"Use ccache\" OFF)\n"
        "option (OPT_WITH_DEFAULT_SANITIZERS \"Build with sanitizers\" OFF)\n"
        "option (OPT_INSTALL_DEFAULT_CONFIG \"Install default plugin configuration\" ON)\n\n"
        "# Configuration variables:\n"
        "# OPT_QT_MAJOR_VERSION - The major Qt version, defaults to 5\n"
        "# OPT_CCACHE_EXECUTABLE - Custom ccache executable\n\n"
        "######################################################################\n\n"
        "if (\"${OPT_QT_MAJOR_VERSION}\" STREQUAL \"\")\n"
        "    set(OPT_QT_MAJOR_VERSION 5)\n"
        "endif()\n\n"
        "if (\"${CMAKE_CXX_STANDARD}\" STREQUAL \"\")\n"
        "    set(CMAKE_CXX_STANDARD 17)\n"
        "endif()\n\n"
        "if (\"${CMAKE_CXX_STANDARD}\" VERSION_LESS \"17\")\n"
        "    message (FATAL_ERROR \"Use C++17 or later (instead of C++${CMAKE_CXX_STANDARD}) to compile this project.\")\n"
        "endif()\n\n"
        "find_package(LibComms REQUIRED)\n"
        "find_package(#^#MAIN_NS#$# REQUIRED)\n"
        "find_package(cc_tools_qt REQUIRED)\n"
        "find_package(Qt${OPT_QT_MAJOR_VERSION} REQUIRED COMPONENTS Widgets Core)\n\n"
        "set (CMAKE_AUTOMOC ON)\n"
        "set (CMAKE_AUTOUIC ON)\n"
        "set (CMAKE_AUTORCC ON)\n\n"
        "set (extra_opts)\n"
        "if (OPT_WARN_AS_ERR)\n"
        "    list(APPEND extra_opts WARN_AS_ERR)\n"
        "endif()\n\n"
        "if (OPT_USE_CCACHE)\n"
        "    list(APPEND extra_opts USE_CCACHE)\n"
        "    if (NOT \"${OPT_CCACHE_EXECUTABLE}\" STREQUAL \"\")\n"
        "        list(APPEND extra_opts CCACHE_EXECUTABLE \"${OPT_CCACHE_EXECUTABLE}\")\n"
        "    endif()\n"
        "endif()\n\n"   
        "if (OPT_WITH_DEFAULT_SANITIZERS)\n"
        "    list(APPEND extra_opts DEFAULT_SANITIZERS)\n"
        "endif()\n\n"          
        "include(${LibComms_DIR}/CC_Compile.cmake)\n"
        "cc_compile(${extra_opts})\n"
        "cc_msvc_force_warn_opt(/W4)\n\n"
        "include(GNUInstallDirs)\n\n"
        "set (CORE_LIB_NAME \"cc_tools_qt_plugin_#^#MAIN_NS#$#_protocol_core\")\n\n"
        "######################################################################\n\n"
        "function (cc_plugin_core)\n"
        "    set (name ${CORE_LIB_NAME})\n"
        "    set (src\n"
        "        #^#CORE_FILES#$#\n"
        "    )\n\n"
        "    add_library (${name} STATIC ${src})\n"
        "    target_link_libraries (${name} PUBLIC cc::#^#MAIN_NS#$# cc::comms cc::cc_tools_qt Qt${OPT_QT_MAJOR_VERSION}::Widgets Qt${OPT_QT_MAJOR_VERSION}::Core)\n"
        "    target_include_directories (${name} PUBLIC ${PROJECT_SOURCE_DIR})\n"
        "    target_compile_options(${name} PRIVATE\n"
        "        $<$<CXX_COMPILER_ID:MSVC>:/bigobj /wd4127 /wd5054>\n"
        "        $<$<CXX_COMPILER_ID:GNU>:-ftemplate-depth=2048 -fconstexpr-depth=4096 -Wno-unused-local-typedefs>\n"
        "        $<$<CXX_COMPILER_ID:Clang>:-ftemplate-depth=2048 -fconstexpr-depth=4096 -Wno-unused-local-typedefs>\n"
        "    )\n\n"
        "endfunction()\n\n"
        "######################################################################\n\n"
        "function (cc_plugin protocol has_config_widget)\n"
        "    string(TOLOWER \"cc_tools_plugin_${protocol}\" name)\n\n"
        "    if (NOT \"${name}\" MATCHES \".*_protocol$\")\n"
        "        string(APPEND name \"_protocol\")\n"
        "    endif ()\n\n"
        "    set (meta_file \"${CMAKE_CURRENT_SOURCE_DIR}/#^#TOP_NS#$#/#^#MAIN_NS#$#/plugin/Plugin_${protocol}.json\")\n"
        "    set (stamp_file \"${CMAKE_CURRENT_BINARY_DIR}/${protocol}_refresh_stamp.txt\")\n\n"
        "    if ((NOT EXISTS ${stamp_file}) OR (${meta_file} IS_NEWER_THAN ${stamp_file}))\n"
        "        execute_process(\n"
        "            COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_SOURCE_DIR}/#^#TOP_NS#$#/#^#MAIN_NS#$#/plugin/Plugin_${protocol}.h)\n\n"
        "        execute_process(\n"
        "            COMMAND ${CMAKE_COMMAND} -E touch ${stamp_file})\n"
        "    endif ()\n\n"
        "    set (src\n"
        "        #^#TOP_NS#$#/#^#MAIN_NS#$#/plugin/Protocol_${protocol}.cpp\n"
        "        #^#TOP_NS#$#/#^#MAIN_NS#$#/plugin/Plugin_${protocol}.cpp\n"
        "        #^#TOP_NS#$#/#^#MAIN_NS#$#/plugin/Plugin_${protocol}.h\n"
        "        #^#EXTRA_SOURCES#$#\n"        
        "    )\n\n"
        "    if (has_config_widget)\n"
        "        list (APPEND src #^#TOP_NS#$#/#^#MAIN_NS#$#/plugin/ConfigWidget_${protocol}.cpp)\n"
        "    endif ()\n\n"
        "    set(extra_link_opts)\n"
        "    if (CMAKE_COMPILER_IS_GNUCC)\n"
        "        set(extra_link_opts \"-Wl,--no-undefined\")\n"
        "    endif ()\n\n"
        "    add_library (${name} MODULE ${src} ${moc})\n"
        "    target_link_libraries (${name} ${CORE_LIB_NAME} cc::cc_tools_qt Qt${OPT_QT_MAJOR_VERSION}::Widgets Qt${OPT_QT_MAJOR_VERSION}::Core ${extra_link_opts})\n"
        "    target_compile_options(${name} PRIVATE\n"
        "        $<$<CXX_COMPILER_ID:MSVC>:/bigobj /wd4127 /wd5054>\n"
        "        $<$<CXX_COMPILER_ID:GNU>:-ftemplate-depth=2048 -fconstexpr-depth=4096>\n"
        "        $<$<CXX_COMPILER_ID:Clang>:-ftemplate-depth=2048 -fconstexpr-depth=4096>\n"
        "    )\n\n"
        "    install (\n"
        "        TARGETS ${name}\n"
        "        DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/cc_tools_qt/plugin)\n\n"
        "    if (OPT_INSTALL_DEFAULT_CONFIG)\n"
        "        install (\n"
        "            FILES #^#TOP_NS#$#/#^#MAIN_NS#$#/plugin/${protocol}.cfg\n"
        "            DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/cc_tools_qt)\n"
        "    endif()\n"
        "endfunction()\n\n"
        "######################################################################\n\n"
        "if (TARGET cc::cc_tools_qt)\n"
        "    get_target_property(cc_inc cc::cc_tools_qt INTERFACE_INCLUDE_DIRECTORIES)\n\n"
        "    if (NOT cc_inc)\n"
        "        message (FATAL_ERROR \"No include directories are specified for cc::cc_tools_qt\")\n"
        "    endif ()\n\n"
        "    # Global include is required for \"moc\"\n"
        "    include_directories (${cc_inc})\n"
        "endif ()\n\n"
        "cc_plugin_core()\n\n"
        "#^#PLUGINS_LIST#$#\n"
    ;

    auto& plugins = m_generator.toolsPlugins();
    util::StringsList pluginInvokes;
    for (auto& p : plugins) {
        assert(p);
        pluginInvokes.push_back("cc_plugin (\"" + p->toolsProtocolName() + "\" " + (p->toolsHasConfigWidget() ? "TRUE" : "FALSE") + ")");
    }

    util::ReplacementMap repl = {
        {"CORE_FILES", util::strListToString(m_generator.toolsSourceFiles(), "\n", "")},
        {"PLUGINS_LIST", util::strListToString(pluginInvokes, "\n", "")},
        {"TOP_NS", m_generator.getTopNamespace()},
        {"MAIN_NS", m_generator.protocolSchema().mainNamespace()},
        {"EXTRA_SOURCES", util::readFileContents(util::pathAddElem(m_generator.getCodeDir(), strings::cmakeListsFileStr()) + strings::sourcesFileSuffixStr())},
    };

    auto str = commsdsl::gen::util::processTemplate(Template, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

} // namespace commsdsl2tools_qt