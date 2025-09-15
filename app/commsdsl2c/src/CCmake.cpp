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

#include "CCmake.h"

#include "CGenerator.h"
#include "CSchema.h"
// #include "CVersion.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2c
{

bool CCmake::cWrite(CGenerator& generator)
{
    CCmake obj(generator);
    return obj.cWriteInternal();
}

bool CCmake::cWriteInternal() const
{
    auto filePath = util::genPathAddElem(m_cGenerator.genGetOutputDir(), strings::genCmakeListsFileStr());
    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_cGenerator.genCreateDirectory(dirPath)) {
        return false;
    }       

    m_cGenerator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_cGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }     

    const std::string Templ = 
        "cmake_minimum_required (VERSION 3.12)\n"
        "project (#^#PROJ_NAME#$#_c)\n\n"
        "option (OPT_FIND_COMMS \"Use find_package to find COMMS headers-only library.\" ON)\n"
        "option (OPT_FIND_PROTOCOL \"Use find_package to find protocol definition headers-only library.\" ON)\n"
        "option (OPT_USE_CCACHE \"Use ccache\" OFF)\n"
        "\n"
        "# CMake built-in options\n"
        "option (BUILD_SHARED_LIBS \"Build as shared library\" OFF)\n"
        "\n"
        "# Extra input parameters:\n"
        "# OPT_PROTOCOL_NAME - Optional parameter to override expected protocol definition project name. Defaults to \"#^#PROJ_NAME#$#\".\n"
        "# OPT_CMAKE_NAMESPACE_NAME - Optional parameter to override CMake namespace of built target. Defaults to \"cc\".\n"
        "# OPT_EXTRA_INCLUDE_DIRS - Optional parameter to provide extra include directories for the dependencies.\n"
        "# OPT_CCACHE_EXECUTABLE - Custom ccache executable\n"
        "\n"
        "######################################################################\n\n"
        "if (\"${OPT_PROTOCOL_NAME}\" STREQUAL \"\")\n"
        "    set (OPT_PROTOCOL_NAME #^#PROJ_NAME#$#)\n"
        "endif ()\n\n"
        "if (\"${OPT_CMAKE_NAMESPACE_NAME}\" STREQUAL \"\")\n"
        "    set (OPT_CMAKE_NAMESPACE_NAME cc)\n"
        "endif ()\n"
        "if (OPT_FIND_COMMS)\n"
        "    find_package(LibComms REQUIRED)\n"
        "endif ()\n\n"
        "if (OPT_FIND_PROTOCOL)\n"
        "    find_package(${OPT_PROTOCOL_NAME} REQUIRED)\n"
        "endif ()\n\n"
        "if (OPT_USE_CCACHE)\n"
        "    if (\"${OPT_CCACHE_EXECUTABLE}\" STREQUAL \"\")\n"
        "        find_program(ccache_exe ccache REQUIRED)\n"
        "        set (OPT_CCACHE_EXECUTABLE ${ccache_exe})\n"
        "    endif ()\n\n"
        "   if (OPT_CCACHE_EXECUTABLE)\n"
        "       set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${OPT_CCACHE_EXECUTABLE})\n"
        "       set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${OPT_CCACHE_EXECUTABLE})\n"
        "   endif ()\n"
        "endif()\n\n"
        "set (src\n"
        "    #^#SOURCES#$#\n"
        "    #^#EXTRA_SOURCES#$#\n"
        ")\n\n"
        "add_library(${PROJECT_NAME} ${src})\n"
        "target_include_directories(${PROJECT_NAME}\n"
        "    PUBLIC\n"
        "        $<INSTALL_INTERFACE:include>\n"
        "        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>\n"
        "    PRIVATE\n"
        "        ${CMAKE_CURRENT_SOURCE_DIR}"
        ")\n\n"
        "if (NOT \"${OPT_EXTRA_INCLUDE_DIRS}\" STREQUAL \"\")\n"
        "    target_include_directories(${PROJECT_NAME} PRIVATE ${OPT_EXTRA_INCLUDE_DIRS})\n"
        "endif ()\n\n"
        "if (TARGET cc::${OPT_PROTOCOL_NAME})\n"
        "    target_link_libraries(${PROJECT_NAME} PRIVATE cc::${OPT_PROTOCOL_NAME})\n"
        "endif ()\n\n"
        "if (TARGET cc::comms)\n"
        "    target_link_libraries(${PROJECT_NAME} PRIVATE cc::comms)\n"
        "endif ()\n\n"   
        "include (GNUInstallDirs)\n"     
        "install(\n"
        "    TARGETS ${PROJECT_NAME}\n"
        "    DESTINATION ${CMAKE_INSTALL_LIBDIR}\n"
        "    EXPORT ${PROJECT_NAME}Config\n"
        ")\n\n"
        "install(EXPORT ${PROJECT_NAME}Config NAMESPACE ${OPT_CMAKE_NAMESPACE_NAME}::\n"
        "    DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}/cmake\n"
        ")\n"
        "\n"
        "install(\n"
        "    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/cc_c\n"
        "    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}\n"
        ")\n"
        "#^#APPEND#$#\n"
        ;   

    util::GenStringsList sources;
    // TODO:
    // CVersion::cAddSourceFiles(m_cGenerator, sources);

    for (auto& sPtr : m_cGenerator.genSchemas()) {
        auto* s = CSchema::cCast(sPtr.get());
        s->cAddSourceFiles(sources);
    }

    util::GenReplacementMap repl = {
        {"PROJ_NAME", m_cGenerator.genProtocolSchema().genMainNamespace()},
        {"APPEND", util::genReadFileContents(util::genPathAddElem(m_cGenerator.genGetCodeDir(), strings::genCmakeListsFileStr()) + strings::genAppendFileSuffixStr())},
        {"SOURCES", util::genStrListToString(sources, "\n", "")},
        {"EXTRA_SOURCES", util::genReadFileContents(util::genPathAddElem(m_cGenerator.genGetCodeDir(), strings::genCmakeListsFileStr()) + strings::genSourcesFileSuffixStr())},
    };

    auto str = commsdsl::gen::util::genProcessTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_cGenerator.genLogger().genError("Failed to write " + filePath + ".");
        return false;
    }

    return true;
}

} // namespace commsdsl2c
