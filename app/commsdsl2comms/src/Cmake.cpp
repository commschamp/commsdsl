//
// Copyright 2018 - 2019 (C). Alex Robenko. All rights reserved.
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
    return 
        obj.writeMain() && 
        obj.writePlugin() &&
        obj.writeTest();
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

    auto allInterfaces = m_generator.getAllInterfaces();
    assert(!allInterfaces.empty());
    auto* firstInterface = allInterfaces.front();
    assert(!firstInterface->name().empty());

    auto allFrames = m_generator.getAllFrames();
    assert(!allFrames.empty());
    auto* firstFrame = allFrames.front();
    assert(!firstFrame->name().empty());

    std::string build_test_opt("OFF");
    if (m_generator.testsBuildEnabledByDefault()) {
        build_test_opt = "ON";
    }

    std::string build_plugin_opt("OFF");
    if (m_generator.pluginBuildEnabledByDefault()) {
        build_plugin_opt = "ON";
    }

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PROJ_NAME", m_generator.schemaName()));
    replacements.insert(std::make_pair("PROJ_NAMESPACE", m_generator.mainNamespace()));
    replacements.insert(std::make_pair("CC_TAG", m_generator.commsChampionTag()));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForFile(common::cmakeListsFileStr())));
    replacements.insert(std::make_pair("DEFAULT_INTERFACE", m_generator.scopeForInterface(firstInterface->externalRef(), true, true)));
    replacements.insert(std::make_pair("DEFAULT_FRAME", m_generator.scopeForFrame(firstFrame->externalRef(), true, true)));
    replacements.insert(std::make_pair("DEFAULT_OPTIONS", m_generator.scopeForOptions(common::defaultOptionsStr(), true, true)));
    replacements.insert(std::make_pair("DEFAULT_INPUT", m_generator.scopeForInput(common::allMessagesStr(), true, true)));
    replacements.insert(std::make_pair("BUILD_TEST_OPT", build_test_opt));
    replacements.insert(std::make_pair("BUILD_PLUGIN_OPT", build_plugin_opt));

    static const std::string Template = 
        "cmake_minimum_required (VERSION 3.1)\n"
        "project (\"#^#PROJ_NAME#$#\")\n\n"
        "option (OPT_BUILD_TEST \"Build and install test applications.\" #^#BUILD_TEST_OPT#$#)\n"
        "option (OPT_BUILD_PLUGIN \"Build and install CommsChampion plugin.\" #^#BUILD_PLUGIN_OPT#$#)\n"
        "option (OPT_NO_COMMS \"Forcefully exclude checkout and install of COMMS library. \\\n"
        "    Works only if OPT_BUILD_TEST and OPT_BUILD_PLUGIN options weren't used\" OFF)\n"
        "option (OPT_WARN_AS_ERR \"Treat warning as error\" ON)\n"
        "option (OPT_USE_CCACHE \"Use of ccache on UNIX system\" ON)\n\n"
        "# Other parameters:\n"
        "# OPT_CC_TAG - Override default tag of comms_champion project.\n"
        "# OPT_QT_DIR - Path to custom Qt5 install directory.\n"
        "# OPT_CC_MAIN_INSTALL_DIR - Path to CommsChampion external install directory\n"
        "#       (if such already built).\n"
        "# OPT_TEST_OPTIONS - Class name of the options for test applications,\n"
        "#       defaults to #^#DEFAULT_OPTIONS#$#.\n"        
        "# OPT_TEST_INTERFACE - Class name of the interface for test applications,\n"
        "#       defaults to #^#DEFAULT_INTERFACE#$#.\n"
        "# OPT_TEST_FRAME - Class name of the frame for test applications,\n"
        "#       defaults to #^#DEFAULT_FRAME#$#.\n"
        "# OPT_TEST_INPUT_MESSAGES - All input messages bundle for test applications,\n"
        "#       defaults to #^#DEFAULT_INPUT#$#.\n\n"
        "if (\"${OPT_CC_TAG}\" STREQUAL \"\")\n"
        "    set (OPT_CC_TAG \"#^#CC_TAG#$#\")\n"
        "endif()\n\n"
        "if (NOT CMAKE_CXX_STANDARD)\n"
        "    set (CMAKE_CXX_STANDARD 11)\n"
        "endif()\n\n"        
        "include(GNUInstallDirs)\n"
        "set (LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})\n"
        "set (BIN_INSTALL_DIR ${CMAKE_INSTALL_BINDIR})\n"
        "set (INC_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})\n"
        "set (CONFIG_INSTALL_DIR ${CMAKE_INSTALL_DATADIR}/CommsChampion)\n"
        "set (PLUGIN_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/CommsChampion/plugin)\n"
        "set (DOC_INSTALL_DIR ${CMAKE_INSTALL_DOCDIR})\n\n"
        "######################################################################\n\n"
        "if (OPT_BUILD_PLUGIN)\n"
        "    if (NOT \"${OPT_QT_DIR}\" STREQUAL \"\")\n"
        "        list (APPEND CMAKE_PREFIX_PATH ${OPT_QT_DIR})\n"
        "    endif ()\n\n"
        "    find_package(Qt5Core)\n"
        "    find_package(Qt5Widgets)\n"
        "endif ()\n\n"
        "######################################################################\n\n"
        "# Set compiler flags\n"
        "if ((CMAKE_COMPILER_IS_GNUCC) OR (\"${CMAKE_CXX_COMPILER_ID}\" STREQUAL \"Clang\"))\n"
        "    set (extra_flags_list\n"
        "        \"-Wall\" \"-Wextra\" \"-Wcast-align\" \"-Wcast-qual\" \"-Wctor-dtor-privacy\"\n"
        "        \"-Wmissing-include-dirs\"\n"
        "        \"-Woverloaded-virtual\" \"-Wredundant-decls\" \"-Wshadow\" \"-Wundef\" \"-Wunused\"\n"
        "        \"-Wno-unknown-pragmas\" \"-fdiagnostics-show-option\"\n"
        "    )\n\n"
        "    if (CMAKE_COMPILER_IS_GNUCC)\n"
        "        list (APPEND extra_flags_list\n"
        "            \"-Wnoexcept\" \"-Wlogical-op\" \"-Wstrict-null-sentinel\" \"-ftemplate-backtrace-limit=0\"\n"
        "        )\n"
        "    endif ()\n\n"
        "    if (\"${CMAKE_CXX_COMPILER_ID}\" STREQUAL \"Clang\")\n"
        "        list (APPEND extra_flags_list\n"
        "        \"-Wno-dangling-field\" \"-Wno-unused-command-line-argument\"\n"
        "        \"-ftemplate-depth=1024\")\n"
        "    endif ()\n\n"
        "    if (OPT_WARN_AS_ERR)\n"
        "        list (APPEND extra_flags_list \"-Werror\")\n"
        "    endif ()\n\n"
        "    string(REPLACE \";\" \" \" extra_flags \"${extra_flags_list}\")\n"
        "    set (CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} ${extra_flags}\")\n"
        "elseif (MSVC)\n"
        "    add_definitions(\"/bigobj\" \"-D_CRT_SECURE_NO_WARNINGS\")\n"
        "    if (OPT_WARN_AS_ERR)\n"
        "        add_definitions(\"/WX\")\n"
        "    endif ()\n"
        "endif ()\n\n"
        "if ((UNIX) AND (OPT_USE_CCACHE))\n"
        "    find_program(CCACHE_FOUND ccache)\n"
        "    if(CCACHE_FOUND)\n"
        "        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)\n"
        "        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)\n"
        "    endif()\n"
        "endif ()\n\n"
        "######################################################################\n\n"
        "# Use external CommsChampion project or compile it in place\n"
        "set (CC_EXTERNAL_TGT \"comms_champion_external\")\n"
        "if (OPT_BUILD_TEST OR OPT_BUILD_PLUGIN)\n"
        "    set (external_cc_needed TRUE)\n"
        "endif ()\n\n"
        "if ((external_cc_needed OR (NOT OPT_NO_COMMS)) AND (\"${OPT_CC_MAIN_INSTALL_DIR}\" STREQUAL \"\"))\n"
        "    set (cc_repo \"https://github.com/arobenko/comms_champion.git\")\n"
        "    set (cc_tag \"${OPT_CC_TAG}\")\n"
        "    set (cc_main_dir \"${CMAKE_BINARY_DIR}/comms_champion\")\n"
        "    set (cc_src_dir \"${cc_main_dir}/src\")\n"
        "    set (cc_bin_dir \"${cc_main_dir}/build\")\n"
        "    set (cc_install_dir \"${CMAKE_INSTALL_PREFIX}\")\n\n"
        "    if (NOT \"${OPT_QT_DIR}\" STREQUAL \"\")\n"
        "        set (cc_qt_dir_opt -DCC_QT_DIR=${OPT_QT_DIR})\n"
        "        set (EXT_QT_DIR ${OPT_QT_DIR})\n"
        "    endif ()\n\n"
        "    if (NOT ${OPT_BUILD_PLUGIN})\n"
        "        set (ct_lib_only_opt -DCC_COMMS_LIB_ONLY=ON)\n"
        "        set (EXT_CC_NO_COMMS_CHAMPION TRUE)\n"
        "    endif ()\n\n"
        "    find_package (Git REQUIRED)\n"
        "    set (ext_targets_inc_file \"${cc_src_dir}/cmake/DefineExternalProjectTargets.cmake\")\n"
        "    if (NOT EXISTS \"${ext_targets_inc_file}\")\n"
        "        execute_process (\n"
        "            COMMAND ${CMAKE_COMMAND} -E remove_directory \"${cc_src_dir}\"\n"
        "        )\n\n"
        "        execute_process (\n"
        "            COMMAND \n"
        "                ${GIT_EXECUTABLE} clone -b ${cc_tag} --depth 1 ${cc_repo} ${cc_src_dir}\n"
        "            RESULT_VARIABLE git_result\n"
        "        )\n\n"
        "        if (NOT \"${git_result}\" STREQUAL \"0\")\n"
        "            message (FATAL_ERROR \"git clone/checkout failed\")\n"
        "        endif ()\n\n"
        "        if (NOT EXISTS \"${ext_targets_inc_file}\")\n"
        "             message (FATAL_ERROR \"Incompatible version of ${cc_repo}, please use different tag.\")\n"
        "        endif ()\n\n"
        "    endif ()\n\n"
        "    set (EXT_CC_INSTALL_DIR ${cc_install_dir})\n"
        "    include (${ext_targets_inc_file})\n\n"
        "    include(ExternalProject)\n"
        "    ExternalProject_Add(\n"
        "        \"${CC_EXTERNAL_TGT}\"\n"
        "        PREFIX \"${cc_main_dir}\"\n"
        "        STAMP_DIR \"${cc_bin_dir}\"\n"
        "        GIT_REPOSITORY ${cc_repo}\n"
        "        GIT_TAG ${cc_tag}\n"
        "        SOURCE_DIR \"${cc_src_dir}\"\n"
        "        BINARY_DIR \"${cc_bin_dir}\"\n"
        "        INSTALL_DIR \"${cc_install_dir}\"\n"
        "        CMAKE_ARGS\n"
        "            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${cc_install_dir}\n"
        "            -DCC_NO_UNIT_TESTS=ON -DCC_NO_WARN_AS_ERR=ON -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}\n"
        "            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}\n"
        "            ${cc_qt_dir_opt} ${ct_lib_only_opt}\n"
        "    )\n\n"
        "    if (WIN32 AND (NOT \"${OPT_QT_DIR}\" STREQUAL \"\"))\n"
        "        message (STATUS \"Qt5 deployment is available by building \\\"deploy_qt\\\" target\")\n"
        "        add_custom_target (\"deploy_qt\"\n"
        "            COMMAND ${CMAKE_COMMAND} --build ${cc_bin_dir} --target deploy_qt\n"
        "            WORKING_DIRECTORY ${cc_bin_dir}\n"
        "        )\n\n"
        "        add_dependencies(\"deploy_qt\" ${CC_EXTERNAL_TGT})\n"
        "    endif ()\n"
        "elseif (external_cc_needed)\n"
        "    list (APPEND CMAKE_PREFIX_PATH \"${OPT_CC_MAIN_INSTALL_DIR}\")\n"
        "    find_package(CommsChampion NO_MODULE)\n"
        "endif ()\n\n"
        "######################################################################\n\n"
        "# Define documentation target\n"
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
        "# Define protocol library\n"
        "add_library(#^#PROJ_NAMESPACE#$# INTERFACE)\n\n"
        "target_include_directories(#^#PROJ_NAMESPACE#$# INTERFACE\n"
        "    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>\n"
        "    $<INSTALL_INTERFACE:include>\n"
        ")\n\n"
        "if (external_cc_needed AND (NOT TARGET ${CC_EXTERNAL_TGT}))\n"
        "    if (NOT TARGET cc::comms)\n"
        "        message (FATAL_ERROR \"COMMS library is not found\")\n"
        "    endif()\n\n"
        "    target_link_libraries(#^#PROJ_NAMESPACE#$# INTERFACE cc::comms)\n"
        "endif ()\n\n"
        "install(TARGETS #^#PROJ_NAMESPACE#$# EXPORT #^#PROJ_NAMESPACE#$#Config)\n"
        "install(EXPORT #^#PROJ_NAMESPACE#$#Config\n"
        "    DESTINATION ${LIB_INSTALL_DIR}/#^#PROJ_NAMESPACE#$#/cmake\n"
        ")\n\n"
        "install (\n"
        "    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/#^#PROJ_NAMESPACE#$#\n"
        "    DESTINATION ${INC_INSTALL_DIR}\n"
        ")\n\n"
        "######################################################################\n\n"
        "if (OPT_BUILD_TEST)\n"
        "    add_subdirectory(test)\n"
        "endif ()\n\n"
        "######################################################################\n\n"
        "if (OPT_BUILD_PLUGIN)\n"
        "    add_subdirectory(cc_plugin)\n"
        "endif ()\n"
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
            "if (TARGET ${CC_EXTERNAL_TGT})\n"
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
        "            set (PLUGIN_INSTALL_DIR \"${rel_plugin_install_path}\")\n"
        "        endif()\n"
        "    endif ()\n\n"
        "    if (TARGET ${CC_EXTERNAL_TGT})\n"
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

bool Cmake::writeTest() const
{
    auto dir = m_generator.testDir();
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

    auto allInterfaces = m_generator.getAllInterfaces();
    assert(!allInterfaces.empty());
    auto* firstInterface = allInterfaces.front();
    assert(!firstInterface->name().empty());

    auto allFrames = m_generator.getAllFrames();
    assert(!allFrames.empty());
    auto* firstFrame = allFrames.front();
    assert(!firstFrame->name().empty());

    std::vector<std::string> appendPath = {
        common::testStr(),
        common::cmakeListsFileStr()
    };
    

    common::ReplacementMap replacements;
    replacements.insert(std::make_pair("PROJ_NS", m_generator.mainNamespace()));
    replacements.insert(std::make_pair("APPEND", m_generator.getExtraAppendForFile(appendPath)));
    replacements.insert(std::make_pair("INTERFACE_SCOPE", m_generator.scopeForInterface(firstInterface->externalRef(), true, true)));
    replacements.insert(std::make_pair("FRAME_SCOPE", m_generator.scopeForFrame(firstFrame->externalRef(), true, true)));
    replacements.insert(std::make_pair("OPTIONS_SCOPE", m_generator.scopeForOptions(common::defaultOptionsStr(), true, true)));
    replacements.insert(std::make_pair("INPUT_SCOPE", m_generator.scopeForInput(common::allMessagesStr(), true, true)));

    static const std::string Template =
        "######################################################################\n"
        "function (define_test name)\n"
        "    set (src ${name}.cpp)\n"
        "    add_executable(${name} ${src})\n"
        "    target_link_libraries(${name} PRIVATE cc::comms #^#PROJ_NS#$#)\n\n"
        "    set (extra_defs)\n"
        "    if (NOT \"${OPT_TEST_INTERFACE}\" STREQUAL \"\")\n"
        "        list (APPEND extra_defs -DINTERFACE=${OPT_TEST_INTERFACE})\n"
        "    endif ()\n\n"
        "    if (NOT \"${OPT_TEST_INTERFACE_HEADER}\" STREQUAL \"\")\n"
        "        list (APPEND extra_defs -DINTERFACE_HEADER=${OPT_TEST_INTERFACE_HEADER})\n"
        "    endif ()\n\n"        
        "    if (NOT \"${OPT_TEST_FRAME}\" STREQUAL \"\")\n"
        "        list (APPEND extra_defs -DFRAME=${OPT_TEST_FRAME})\n"
        "    endif ()\n\n"
        "    if (NOT \"${OPT_TEST_FRAME_HEADER}\" STREQUAL \"\")\n"
        "        list (APPEND extra_defs -DFRAME_HEADER=${OPT_TEST_FRAME_HEADER})\n"
        "    endif ()\n\n"        
        "    if (NOT \"${OPT_TEST_OPTIONS}\" STREQUAL \"\")\n"
        "        list (APPEND extra_defs -DOPTIONS=${OPT_TEST_OPTIONS})\n"
        "    endif ()\n\n"
        "    if (NOT \"${OPT_TEST_OPTIONS_HEADER}\" STREQUAL \"\")\n"
        "        list (APPEND extra_defs -DOPTIONS_HEADER=${OPT_TEST_OPTIONS_HEADER})\n"
        "    endif ()\n\n"        
        "    if (NOT \"${OPT_TEST_INPUT_MESSAGES}\" STREQUAL \"\")\n"
        "        list (APPEND extra_defs -DINPUT_MESSAGES=${OPT_TEST_INPUT_MESSAGES})\n"
        "    endif ()\n\n"
        "    if (NOT \"${OPT_TEST_INPUT_MESSAGES_HEADER}\" STREQUAL \"\")\n"
        "        list (APPEND extra_defs -DINPUT_MESSAGES_HEADER=${OPT_TEST_INPUT_MESSAGES_HEADER})\n"
        "    endif ()\n\n"
        "    if (extra_defs)\n"
        "        target_compile_definitions(${name} PRIVATE ${extra_defs})\n"
        "    endif ()\n\n"
        "    install (\n"
        "        TARGETS ${name}\n"
        "        DESTINATION ${BIN_INSTALL_DIR}\n"
        "    )\n\n"
        "    if (TARGET ${CC_EXTERNAL_TGT})\n"
        "        add_dependencies(${name} ${CC_EXTERNAL_TGT})\n"
        "    endif ()\n"        
        "endfunction ()\n\n"
        "######################################################################\n\n"
        "if (\"${OPT_TEST_INTERFACE}\" STREQUAL \"\")\n"
        "    set (OPT_TEST_INTERFACE \"#^#INTERFACE_SCOPE#$#\")\n"
        "endif ()\n\n"
        "if (\"${OPT_TEST_FRAME}\" STREQUAL \"\")\n"
        "    set (OPT_TEST_FRAME \"#^#FRAME_SCOPE#$#\")\n"
        "endif ()\n\n"
        "if (\"${OPT_TEST_OPTIONS}\" STREQUAL \"\")\n"
        "    set (OPT_TEST_OPTIONS \"#^#OPTIONS_SCOPE#$#\")\n"
        "endif ()\n\n"
        "if (\"${OPT_TEST_INPUT_MESSAGES}\" STREQUAL \"\")\n"
        "    set (OPT_TEST_INPUT_MESSAGES \"#^#INPUT_SCOPE#$#\")\n"
        "endif ()\n\n"
        "string (REPLACE \"::\" \"/\" OPT_TEST_INTERFACE_HEADER \"${OPT_TEST_INTERFACE}.h\")\n"
        "string (REPLACE \"::\" \"/\" OPT_TEST_FRAME_HEADER \"${OPT_TEST_FRAME}.h\")\n"
        "string (REPLACE \"::\" \"/\" OPT_TEST_OPTIONS_HEADER \"${OPT_TEST_OPTIONS}.h\")\n"
        "string (REPLACE \"::\" \"/\" OPT_TEST_INPUT_MESSAGES_HEADER \"${OPT_TEST_INPUT_MESSAGES}.h\")\n\n"
        "if ((CMAKE_COMPILER_IS_GNUCC) OR (\"${CMAKE_CXX_COMPILER_ID}\" STREQUAL \"Clang\"))\n"
        "    set (CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} -Wno-unused-function\")\n"
        "endif ()\n\n"
        "if (\"${CMAKE_CXX_COMPILER_ID}\" STREQUAL \"Clang\")\n"
        "    set (CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} -Wno-unneeded-internal-declaration\")\n"
        "endif ()\n\n"
        "define_test(#^#PROJ_NS#$#_input_test)\n"
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
