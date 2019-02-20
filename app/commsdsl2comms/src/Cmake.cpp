//
// Copyright 2018 (C). Alex Robenko. All rights reserved.
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

#include "Cmake.h"

#include <fstream>

#include <boost/filesystem.hpp>

#include "Generator.h"
#include "common.h"

namespace bf = boost::filesystem;

namespace commsdsl2comms
{

bool Cmake::write(Generator& generator)
{
    Cmake obj(generator);
    return obj.writeMain() && obj.writePlugin();
}

bool Cmake::writeMain() const
{
    auto dir = m_generator.outputDir();
    if (dir.empty()) {
        return false;
    }

    bf::path filePath(dir);
    filePath /= common::cmakeListsFileStr();

    std::string filePathStr(filePath.string());

    m_generator.logger().info("Generating " + filePathStr);
    std::ofstream stream(filePathStr);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePathStr + "\" for writing.");
        return false;
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PROJ_NAME", m_generator.schemaName()));
    replacements.insert(std::make_pair("PROJ_NAMESPACE", m_generator.mainNamespace()));
    replacements.insert(std::make_pair("CC_TAG", m_generator.commsChampionTag()));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForFile(common::cmakeListsFileStr())));

    static const std::string Template = 
        "cmake_minimum_required (VERSION 3.1)\n"
        "project (\"#^#PROJ_NAME#$#\")\n\n"
        "option (OPT_LIB_ONLY \"Install only protocol library, no other libraries/plugings are built/installed.\" OFF)\n"
        "option (OPT_THIS_AND_COMMS_LIBS_ONLY \"Install this protocol and COMMS libraries only, no other applications/plugings are built/installed.\" OFF)\n"
        "option (OPT_FULL_SOLUTION \"Build and install full solution, including CommsChampion.\" ON)\n"
        "option (OPT_NO_WARN_AS_ERR \"Do NOT treat warning as error\" OFF)\n"
        "option (OPT_NO_CCACHE \"Disable use of ccache on UNIX system\" OFF)\n\n"
        "# Other parameters:\n"
        "# OPT_QT_DIR - Path to custom Qt5 install directory.\n"
        "# OPT_CC_MAIN_INSTALL_DIR - Path to CommsChampion install directory (if such already built).\n\n"
        "if (NOT CMAKE_CXX_STANDARD)\n"
        "    set (CMAKE_CXX_STANDARD 11)\n"
        "endif()\n\n"
        "set (INSTALL_DIR ${CMAKE_INSTALL_PREFIX})\n\n"
        "include(GNUInstallDirs)\n"
        "set (LIB_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR})\n"
        "set (BIN_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_BINDIR})\n"
        "set (INC_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_INCLUDEDIR})\n"
        "set (CONFIG_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_DATADIR}/CommsChampion)\n"
        "set (PLUGIN_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/CommsChampion/plugin)\n"
        "set (DOC_INSTALL_DIR ${INSTALL_DIR}/${CMAKE_INSTALL_DOCDIR})\n\n"
        "find_package (Doxygen)\n"
        "if (DOXYGEN_FOUND)\n"
        "    set (doc_output_dir \"${DOC_INSTALL_DIR}\")\n"
        "    make_directory (${doc_output_dir})\n\n"
        "    set (match_str \"OUTPUT_DIRECTORY[^\\n]*\")\n"
        "    set (replacement_str \"OUTPUT_DIRECTORY = ${doc_output_dir}\")\n"
        "    set (config_file \"${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen.conf\")\n"
        "    set (OPT_DOXYGEN_CONFIG_FILE \"${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf\")\n\n"
        "    file (READ ${config_file} config_text)\n"
        "    string (REGEX REPLACE \"${match_str}\" \"${replacement_str}\" modified_config_text \"${config_text}\")\n"
        "    file (WRITE \"${OPT_DOXYGEN_CONFIG_FILE}\" \"${modified_config_text}\")\n"
        "    set (doc_tgt \"doc_#^#PROJ_NAMESPACE#$#\")\n"
        "    add_custom_target (\"${doc_tgt}\"\n"
        "        COMMAND ${DOXYGEN_EXECUTABLE} ${OPT_DOXYGEN_CONFIG_FILE}\n"
        "        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})\n"
        "endif ()\n\n"
        "######################################################################\n\n"
        "function (define_lib)\n"
        "    add_library(#^#PROJ_NAMESPACE#$# INTERFACE)\n\n"
        "    target_include_directories(#^#PROJ_NAMESPACE#$# INTERFACE\n"
        "        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>\n"
        "        $<INSTALL_INTERFACE:include>\n"
        "    )\n\n"
        "    if (TARGET cc::comms AND (NOT OPT_FULL_SOLUTION))\n"
        "        target_link_libraries(#^#PROJ_NAMESPACE#$# INTERFACE cc::comms)\n"
        "    endif()\n\n"
        "    install(TARGETS #^#PROJ_NAMESPACE#$# EXPORT #^#PROJ_NAMESPACE#$#Config)\n"
        "    install(EXPORT #^#PROJ_NAMESPACE#$#Config\n"
        "        DESTINATION ${LIB_INSTALL_DIR}/#^#PROJ_NAMESPACE#$#/cmake\n"
        "    )\n"
        "    install (\n"
        "        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/#^#PROJ_NAMESPACE#$#\n"
        "        DESTINATION ${INC_INSTALL_DIR}\n"
        "    )\n"
        "endfunction ()\n\n"
        "######################################################################\n\n"
        "if (OPT_LIB_ONLY)\n"
        "    define_lib()\n"
        "    return ()\n"
        "endif ()\n\n"
        "######################################################################\n\n"
        "if (NOT \"${OPT_QT_DIR}\" STREQUAL \"\")\n"
        "    list (APPEND CMAKE_PREFIX_PATH ${OPT_QT_DIR})\n"
        "endif ()\n\n"
        "set (CC_EXTERNAL_TGT \"comms_champion_external\")\n"
        "include(ExternalProject)\n"
        "macro (externals install_dir build_cc deploy_tgt)\n"
        "    set (cc_tag \"#^#CC_TAG#$#\")\n"
        "    set (cc_main_dir \"${CMAKE_BINARY_DIR}/comms_champion\")\n"
        "    set (cc_src_dir \"${cc_main_dir}/src\")\n"
        "    set (cc_bin_dir \"${cc_main_dir}/build\")\n\n"
        "    if (NOT \"${OPT_QT_DIR}\" STREQUAL \"\")\n"
        "        set (cc_qt_dir_opt -DCC_QT_DIR=${OPT_QT_DIR})\n"
        "    endif ()\n\n"
        "    if (NOT ${build_cc})\n"
        "        set (ct_lib_only_opt -DCC_COMMS_LIB_ONLY=ON)\n"
        "    endif ()\n\n"
        "    find_package (Git REQUIRED)\n"
        "    if (EXISTS \"${cc_src_dir}/.git\")\n"
        "        execute_process (\n"
        "            COMMAND ${GIT_EXECUTABLE} fetch --depth 1\n"
        "            WORKING_DIRECTORY ${cc_src_dir}\n"
        "        )\n\n"
        "        execute_process (\n"
        "            COMMAND ${GIT_EXECUTABLE} checkout ${cc_tag}\n"
        "            WORKING_DIRECTORY ${cc_src_dir}\n"
        "        )\n"
        "    else ()\n"
        "        execute_process (\n"
        "            COMMAND ${CMAKE_COMMAND} -E remove_directory \"${cc_src_dir}\"\n"
        "        )\n\n"
        "        execute_process (\n"
        "            COMMAND \n"
        "                ${GIT_EXECUTABLE} clone -b ${cc_tag} --depth 1 https://github.com/arobenko/comms_champion.git\n"
        "                    ${cc_src_dir}\n"
        "            RESULT_VARIABLE git_result\n"
        "        )\n\n"
        "        if (NOT \"${git_result}\" STREQUAL \"0\")\n"
        "            message (WARNING \"git clone/checkout failed\")\n"
        "        endif ()\n"
        "    endif ()\n\n"
        "    set (EXT_CC_INSTALL_DIR ${install_dir})\n"
        "    include (${cc_src_dir}/cmake/DefineExternalProjectTargets.cmake)\n\n"
        "    ExternalProject_Add(\n"
        "        \"${CC_EXTERNAL_TGT}\"\n"
        "        PREFIX \"${cc_main_dir}\"\n"
        "        SOURCE_DIR \"${cc_src_dir}\"\n"
        "        BINARY_DIR \"${cc_bin_dir}\"\n"
        "        INSTALL_DIR \"${install_dir}\"\n"
        "        CMAKE_ARGS\n"
        "            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${install_dir}\n"
        "            -DCC_NO_UNIT_TESTS=ON -DCC_NO_WARN_AS_ERR=ON -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}\n"
        "            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}\n"
        "            ${cc_qt_dir_opt} ${ct_lib_only_opt}\n"
        "    )\n\n"
        "    if (${deploy_tgt} AND WIN32 AND (NOT \"${OPT_QT_DIR}\" STREQUAL \"\"))\n"
        "        message (STATUS \"Qt5 deployment is available by building \\\"deploy_qt\\\" target\")\n"
        "        add_custom_target (\"deploy_qt\"\n"
        "            COMMAND ${CMAKE_COMMAND} --build ${cc_bin_dir} --target deploy_qt\n"
        "            WORKING_DIRECTORY ${cc_bin_dir}\n"
        "        )\n\n"
        "        add_dependencies(\"deploy_qt\" ${CC_EXTERNAL_TGT})\n"
        "    endif ()\n"
        "endmacro()\n\n"
        "######################################################################\n\n"
        "if (OPT_THIS_AND_COMMS_LIBS_ONLY)\n"
        "    externals(${INSTALL_DIR} FALSE FALSE)\n"
        "    define_lib ()\n"
        "    return()\n"
        "endif ()\n\n"
        "while (TRUE)\n"
        "    if (OPT_FULL_SOLUTION)\n"
        "        externals(${INSTALL_DIR} TRUE TRUE)\n"
        "        break()\n"
        "    endif ()\n\n"
        "    list (APPEND CMAKE_PREFIX_PATH \"${INSTALL_DIR}\")\n"
        "    if (NOT \"${OPT_CC_MAIN_INSTALL_DIR}\" STREQUAL \"\")\n"
        "        list (APPEND CMAKE_PREFIX_PATH \"${OPT_CC_MAIN_INSTALL_DIR}\")\n"
        "    endif ()\n\n"
        "    find_package(CommsChampion QUIET NO_MODULE)\n\n"
        "    if (NOT CC_COMMS_FOUND)\n"
        "        set (externals_install \"${CMAKE_BINARY_DIR}/ext_install\")\n"
        "        externals(${externals_install} TRUE FALSE)\n"
        "        break()\n"
        "    endif ()\n\n"
        "    find_package(CommsChampion NO_MODULE)\n"
        "    break()\n"
        "endwhile()\n\n"
        "define_lib ()\n\n"
        "find_package(Qt5Core)\n"
        "find_package(Qt5Widgets)\n\n"
        "if ((CMAKE_COMPILER_IS_GNUCC) OR (\"${CMAKE_CXX_COMPILER_ID}\" STREQUAL \"Clang\"))\n"
        "    set (extra_flags_list\n"
        "        \"-Wall\" \"-Wextra\" \"-Wcast-align\" \"-Wcast-qual\" \"-Wctor-dtor-privacy\"\n"
        "        \"-Wmissing-include-dirs\"\n"
        "        \"-Woverloaded-virtual\" \"-Wredundant-decls\" \"-Wshadow\" \"-Wundef\" \"-Wunused\"\n"
        "        \"-Wno-unknown-pragmas\" \"-fdiagnostics-show-option\"\n"
        "    )\n\n"
        "    if (CMAKE_COMPILER_IS_GNUCC)\n"
        "        list (APPEND extra_flags_list\n"
        "            \"-Wnoexcept\" \"-Wlogical-op\" \"-Wstrict-null-sentinel\"\n"
        "        )\n"
        "    endif ()\n\n"
        "    if (\"${CMAKE_CXX_COMPILER_ID}\" STREQUAL \"Clang\")\n"
        "        list (APPEND extra_flags_list\n"
        "           \"-Wno-dangling-field\" \"-Wno-unused-command-line-argument\"\n"
        "           \"-ftemplate-depth=1024\")\n"
        "    endif ()\n\n"
        "    if (NOT OPT_NO_WARN_AS_ERR)\n"
        "        list (APPEND extra_flags_list \"-Werror\")\n"
        "    endif ()\n\n"
        "    string(REPLACE \";\" \" \" extra_flags \"${extra_flags_list}\")\n"
        "    set (CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} ${extra_flags}\")\n"
        "elseif (MSVC)\n"
        "    add_definitions(\"/bigobj\")\n"
        "    if (NOT CC_NO_WARN_AS_ERR)\n"
        "        add_definitions(\"/WX\")\n"
        "    endif ()\n"
        "endif ()\n\n"
        "if ((UNIX) AND (NOT OPT_NO_CCACHE))\n"
        "    find_program(CCACHE_FOUND ccache)\n"
        "    if(CCACHE_FOUND)\n"
        "        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)\n"
        "        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)\n"
        "    endif()\n"
        "endif ()\n\n"
        "add_subdirectory(cc_plugin)\n\n"
        "#^#APPEND#$#\n";

    auto str = common::processTemplate(Template, replacements);
    stream << str;

    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePathStr + "\".");
        return false;
    }
    return true;
}

bool Cmake::writePlugin() const
{
    auto dir = m_generator.pluginDir();
    if (dir.empty()) {
        return false;
    }

    bf::path filePath(dir);
    filePath /= common::cmakeListsFileStr();

    std::string filePathStr(filePath.string());

    m_generator.logger().info("Generating " + filePathStr);
    std::ofstream stream(filePathStr);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePathStr + "\" for writing.");
        return false;
    }

    common::StringsList calls;
    auto plugins = m_generator.getPlugins();
    for (auto* p : plugins) {
        auto pName = common::nameToClassCopy(p->adjustedName());
        calls.push_back("cc_plugin (\"" + pName + "\")");
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("SOURCES", m_generator.pluginCommonSources()));
    replacements.insert(std::make_pair("PROJ_NAMESPACE", m_generator.mainNamespace()));
    replacements.insert(std::make_pair("PLUGINS", common::listToString(calls, "\n", "\n")));
    if (plugins.size() == 1U) {
        auto str =
            "if (OPT_FULL_SOLUTION)\n"
            "    install (\n"
            "        FILES plugin/" + common::nameToClassCopy(plugins.front()->adjustedName()) + ".cfg\n"
            "        DESTINATION ${CONFIG_INSTALL_DIR}\n"
            "        RENAME \"default.cfg\")\n"
            "endif()\n\n";
        replacements.insert(std::make_pair("DEFAULT_INSTALL", std::move(str)));

    }

    std::vector<std::string> appendPath = {
        common::pluginNsStr(),
        common::cmakeListsFileStr()
    };
    
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForFile(appendPath)));

    static const std::string Template =
        "set (ALL_MESSAGES_LIB \"all_messages\")\n\n"
        "######################################################################\n\n"
        "function (cc_plugin_all_messages)\n"
        "    set (name \"${ALL_MESSAGES_LIB}\")\n"
        "    set (src\n"
        "        #^#SOURCES#$#\n"
        "    )\n\n"
        "    add_library (${name} STATIC ${src})\n"
        "    target_link_libraries (${name} PUBLIC #^#PROJ_NAMESPACE#$# cc::comms cc::comms_champion Qt5::Widgets Qt5::Core)\n"
        "    target_include_directories (${name} PUBLIC ${CMAKE_SOURCE_DIR})\n"
        "    if (CC_COMMS_CHAMPION_FOUND)\n"
        "        if (CC_PLUGIN_DIR)\n"
        "            file (RELATIVE_PATH rel_plugin_install_path \"${CC_ROOT_DIR}\" \"${CC_PLUGIN_DIR}\")\n"
        "            set (PLUGIN_INSTALL_DIR \"${INSTALL_DIR}/${rel_plugin_install_path}\")\n"
        "        endif()\n"
        "    endif ()\n\n"
        "    if (OPT_FULL_SOLUTION)\n"
        "        add_dependencies(${name} ${CC_EXTERNAL_TGT})\n"
        "    endif ()\n"
        "endfunction()\n\n"
        "######################################################################\n\n"
        "function (cc_plugin protocol)\n"
        "    set (name \"cc_plugin_${protocol}\")\n\n"
        "    set (meta_file \"${CMAKE_CURRENT_SOURCE_DIR}/${protocol}.json\")\n"
        "    set (stamp_file \"${CMAKE_CURRENT_BINARY_DIR}/${protocol}_refresh_stamp.txt\")\n\n"
        "    if ((NOT EXISTS ${stamp_file}) OR (${meta_file} IS_NEWER_THAN ${stamp_file}))\n"
        "        execute_process(\n"
        "            COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_SOURCE_DIR}/plugin/${protocol}Plugin.h)\n\n"
        "        execute_process(\n"
        "            COMMAND ${CMAKE_COMMAND} -E touch ${stamp_file})\n"
        "    endif ()\n\n"
        "    set (src\n"
        "        plugin/${protocol}Protocol.cpp\n"
        "        plugin/${protocol}Plugin.cpp\n"
        "    )\n\n"
        "    set (hdr\n"
        "        plugin/${protocol}Plugin.h\n"
        "    )\n\n"
        "    qt5_wrap_cpp(moc ${hdr})\n\n"
        "    set(extra_link_opts)\n"
        "    if (CMAKE_COMPILER_IS_GNUCC)\n"
        "        set(extra_link_opts \"-Wl,--no-undefined\")\n"
        "    endif ()\n\n"
        "    add_library (${name} MODULE ${src} ${moc})\n"
        "    target_link_libraries (${name} ${ALL_MESSAGES_LIB} cc::comms_champion Qt5::Widgets Qt5::Core ${extra_link_opts})\n"
        "    if (CMAKE_COMPILER_IS_GNUCC)\n"
        "        target_compile_options(${name} PRIVATE \"-ftemplate-backtrace-limit=0\")\n"
        "    endif ()\n\n"
        "    install (\n"
        "        TARGETS ${name}\n"
        "        DESTINATION ${PLUGIN_INSTALL_DIR})\n\n"
        "    install (\n"
        "        FILES plugin/${protocol}.cfg\n"
        "        DESTINATION ${CONFIG_INSTALL_DIR})\n\n"
        "endfunction()\n\n"
        "######################################################################\n\n"
        "if (NOT Qt5Core_FOUND)\n"
        "    message (WARNING \"Can NOT compile protocol plugin due to missing QT5 Core library\")\n"
        "    return ()\n"
        "endif ()\n\n"
        "if (NOT Qt5Widgets_FOUND)\n"
        "    message (WARNING \"Can NOT compile protocol plugin due to missing QT5 Widgets library\")\n"
        "    return ()\n"
        "endif ()\n\n"        
        "if (CMAKE_COMPILER_IS_GNUCC)\n"
        "    set (CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0\")\n"
        "endif ()\n\n"
        "if (CC_COMMS_CHAMPION_FOUND)\n"
        "    include_directories (${CC_INCLUDE_DIRS})\n"
        "endif ()\n\n"
        "cc_plugin_all_messages()\n\n"
        "#^#PLUGINS#$#\n"
        "#^#DEFAULT_INSTALL#$#\n"
        "#^#APPEND#$#\n";

    auto str = common::processTemplate(Template, replacements);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePathStr + "\".");
        return false;
    }
    return true;
}

} // namespace commsdsl2comms
