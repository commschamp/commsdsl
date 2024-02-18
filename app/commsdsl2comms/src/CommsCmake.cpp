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

#include "CommsCmake.h"

#include "CommsGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <fstream>
#include <cassert>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2comms
{

namespace 
{

using ReplacementMap = util::ReplacementMap;

} // namespace 
    

bool CommsCmake::write(CommsGenerator& generator)
{
    CommsCmake obj(generator);
    return obj.commsWriteInternal();
}

bool CommsCmake::commsWriteInternal() const
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

    const std::string Templ = 
        "cmake_minimum_required (VERSION 3.10)\n"
        "project (\"#^#NAME#$#\")\n\n"
        "option (OPT_REQUIRE_COMMS_LIB \"Require COMMS library, find it and set as dependency to the protocol library\" ON)\n\n"
        "# Other parameters:\n"
        "# OPT_CMAKE_EXPORT_NAMESPACE - Set namespace for a protocol library\n"
        "#     exported via generated *Config.cmake file. Defaults to \"cc\".\n"
        "# OPT_CMAKE_EXPORT_CONFIG_NAME - Override default name \"#^#NAME#$#\" of the cmake generated config file export\n"
        "#     (#^#NAME#$#Config) with provided new name.\n\n"
        "if (CMAKE_TOOLCHAIN_FILE AND EXISTS ${CMAKE_TOOLCHAIN_FILE})\n"
        "    message(STATUS \"Loading toolchain from ${CMAKE_TOOLCHAIN_FILE}\")\n"
        "endif()\n\n"
        "include(GNUInstallDirs)\n\n"
        "######################################################################\n"
        "# Define documentation target\n"
        "find_package (Doxygen)\n"
        "if (DOXYGEN_FOUND)\n"
        "    set (doc_output_dir \"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DOCDIR}\")\n"
        "    set (match_str \"OUTPUT_DIRECTORY[^\\n]*\")\n"
        "    set (replacement_str \"OUTPUT_DIRECTORY = ${doc_output_dir}\")\n"
        "    set (config_file \"${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen.conf\")\n"
        "    set (updated_config_file \"${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf\")\n\n"
        "    file (READ ${config_file} config_text)\n"
        "    string (REGEX REPLACE \"${match_str}\" \"${replacement_str}\" modified_config_text \"${config_text}\")\n\n"
        "    if (\"${PROJECT_BINARY_DIR}\" MATCHES \"^${PROJECT_SOURCE_DIR}\")\n"
        "        get_filename_component(build_name ${PROJECT_BINARY_DIR} NAME)\n"
        "        string(APPEND modified_config_text \"EXCLUDE_PATTERNS       =  */${build_name}/*\\n\")\n"
        "    endif ()\n\n"
        "    file (WRITE \"${updated_config_file}\" \"${modified_config_text}\")\n"
        "    set (doc_tgt \"doc_#^#NAME#$#\")\n"
        "    add_custom_target (\"${doc_tgt}\"\n"
        "        COMMAND ${CMAKE_COMMAND} -E make_directory ${doc_output_dir}\n"
        "        COMMAND ${DOXYGEN_EXECUTABLE} ${updated_config_file}\n"
        "        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})\n"
        "endif ()\n\n"
        "######################################################################\n\n"
        "# Define protocol library\n"
        "add_library(#^#NAME#$# INTERFACE)\n\n"
        "target_include_directories(#^#NAME#$# INTERFACE\n"
        "    $<INSTALL_INTERFACE:include>\n"
        "    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>\n"
        ")\n\n"
        "if (OPT_REQUIRE_COMMS_LIB)\n"
        "    find_package(LibComms REQUIRED)\n"
        "    target_link_libraries(#^#NAME#$# INTERFACE cc::comms)\n"
        "endif ()\n\n"
        "if (\"${OPT_CMAKE_EXPORT_NAMESPACE}\" STREQUAL \"\")\n"
        "    set (OPT_CMAKE_EXPORT_NAMESPACE \"cc\")\n"
        "endif ()\n\n"
        "if (\"${OPT_CMAKE_EXPORT_CONFIG_NAME}\" STREQUAL \"\")\n"
        "    set (OPT_CMAKE_EXPORT_CONFIG_NAME \"#^#NAME#$#\")\n"
        "endif ()\n\n"
        "install(TARGETS #^#NAME#$# EXPORT ${OPT_CMAKE_EXPORT_CONFIG_NAME}Config)\n"
        "install(EXPORT ${OPT_CMAKE_EXPORT_CONFIG_NAME}Config\n"
        "    DESTINATION ${CMAKE_INSTALL_LIBDIR}/${OPT_CMAKE_EXPORT_CONFIG_NAME}/cmake\n"
        "    NAMESPACE ${OPT_CMAKE_EXPORT_NAMESPACE}::\n"
        ")\n\n"
        "install (\n"
        "    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/\n"
        "    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}\n"
        ")\n\n"
        "file (READ \"${PROJECT_SOURCE_DIR}/include/#^#NAME#$#/Version.h\" version_file)\n"
        "string (REGEX MATCH \"#^#CAP_NAME#$#_MAJOR_VERSION[^0-9]*([0-9]*)U*\" _ ${version_file})\n"
        "set (major_ver ${CMAKE_MATCH_1})\n"
        "string (REGEX MATCH \"#^#CAP_NAME#$#_MINOR_VERSION[^0-9]*([0-9]*)U*\" _ ${version_file})\n"
        "set (minor_ver ${CMAKE_MATCH_1})\n"
        "string (REGEX MATCH \"#^#CAP_NAME#$#_PATCH_VERSION[^0-9]*([0-9]*)U*\" _ ${version_file})\n"
        "set (patch_ver ${CMAKE_MATCH_1})\n"
        "if ((NOT \"${major_ver}\" STREQUAL \"\") AND\n"
        "    (NOT \"${minor_ver}\" STREQUAL \"\") AND\n"
        "    (NOT \"${patch_ver}\" STREQUAL \"\"))\n"
        "    set (#^#CAP_NAME#$#_VERSION \"${major_ver}.${minor_ver}.${patch_ver}\")\n\n"
        "    message (STATUS \"Detected version ${#^#CAP_NAME#$#_VERSION} of the protocol library.\")\n"
        "    include(CMakePackageConfigHelpers)\n"
        "    write_basic_package_version_file(\n"
        "        ${CMAKE_BINARY_DIR}/${OPT_CMAKE_EXPORT_CONFIG_NAME}ConfigVersion.cmake\n"
        "        VERSION ${#^#CAP_NAME#$#_VERSION}\n"
        "        COMPATIBILITY AnyNewerVersion)\n\n"
        "    install (\n"
        "        FILES ${CMAKE_BINARY_DIR}/${OPT_CMAKE_EXPORT_CONFIG_NAME}ConfigVersion.cmake\n"
        "        DESTINATION ${CMAKE_INSTALL_LIBDIR}/${OPT_CMAKE_EXPORT_CONFIG_NAME}/cmake/)\n"
        "endif ()\n"
    ;
    
    util::ReplacementMap repl = {
        {"NAME", m_generator.protocolSchema().mainNamespace()},
        {"CAP_NAME", util::strToUpper(m_generator.protocolSchema().mainNamespace())},
    };

    stream << util::processTemplate(Templ, repl);
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

} // namespace commsdsl2comms