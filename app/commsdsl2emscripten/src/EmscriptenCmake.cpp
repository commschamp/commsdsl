//
// Copyright 2022 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "EmscriptenCmake.h"

#include "EmscriptenComms.h"
#include "EmscriptenDataBuf.h"
#include "EmscriptenGenerator.h"
#include "EmscriptenSchema.h"
#include "EmscriptenVersion.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2emscripten
{

bool EmscriptenCmake::emscriptenWrite(EmscriptenGenerator& generator)
{
    EmscriptenCmake obj(generator);
    return obj.emscriptenWriteInternal();
}

bool EmscriptenCmake::emscriptenWriteInternal() const
{
    auto filePath = util::genPathAddElem(m_generator.genGetOutputDir(), strings::genCmakeListsFileStr());
    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.genCreateDirectory(dirPath)) {
        return false;
    }       

    m_generator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }     

    const std::string Templ = 
        "cmake_minimum_required (VERSION 3.12)\n"
        "project (#^#PROJ_NAME#$#_emscripten)\n\n"
        "option (OPT_MODULARIZE \"Force usage of -sMODULARIZE as linker parameter.\" OFF)\n"
        "option (OPT_FIND_COMMS \"Use find_package to find COMMS headers-only library.\" OFF)\n"
        "option (OPT_FIND_PROTOCOL \"Use find_package to find protocol definition headers-only library.\" OFF)\n"
        "option (OPT_USE_CCACHE \"Use ccache\" OFF)\n"
        "\n"
        "# Extra input parameters:\n"
        "# OPT_PROTOCOL_NAME - Optional parameter to override expected protocol definition project name. Defaults to #^#PROJ_NAME#$#.\n"
        "# OPT_INSTALL_DIR - Optional parameter to override installation sub-directory. Defaults to \"js\".\n"
        "# OPT_EXTRA_INCLUDE_DIRS - Optional parameter to provide extra include directories for the dependencies.\n"
        "# OPT_CCACHE_EXECUTABLE - Custom ccache executable\n"
        "\n"
        "######################################################################\n\n"
        "if (NOT EMSCRIPTEN)\n"
        "    message (FATAL_ERROR \"Expected to be wrapped in emcmake\")\n"
        "endif ()\n\n"        
        "if (\"${OPT_PROTOCOL_NAME}\" STREQUAL \"\")\n"
        "    set (OPT_PROTOCOL_NAME #^#PROJ_NAME#$#)\n"
        "endif ()\n\n"
        "if (\"${OPT_INSTALL_DIR}\" STREQUAL \"\")\n"
        "    set (OPT_INSTALL_DIR js)\n"
        "endif ()\n\n"   
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
        "set (extra_link_opts)\n"
        "if (OPT_MODULARIZE)\n"
        "    list (APPEND extra_link_opts -sMODULARIZE)\n"
        "endif ()\n\n"
        "add_executable(${PROJECT_NAME} ${src})\n"
        "target_link_options(${PROJECT_NAME} PRIVATE -lembind -sEXPORTED_RUNTIME_METHODS=ccall ${extra_link_opts})\n"
        "target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)\n\n"
        "if (NOT \"${OPT_EXTRA_INCLUDE_DIRS}\" STREQUAL \"\")\n"
        "    target_include_directories(${PROJECT_NAME} PRIVATE ${OPT_EXTRA_INCLUDE_DIRS})\n"
        "endif ()\n\n"
        "if (TARGET cc::${OPT_PROTOCOL_NAME})\n"
        "    target_link_libraries(${PROJECT_NAME} PRIVATE cc::${OPT_PROTOCOL_NAME})\n"
        "endif ()\n\n"
        "if (TARGET cc::comms)\n"
        "    target_link_libraries(${PROJECT_NAME} PRIVATE cc::comms)\n"
        "endif ()\n\n"        
        "install(FILES\n"
        "    \"$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.js\"\n"
        "    \"$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.wasm\"\n"
        "    DESTINATION ${CMAKE_INSTALL_PREFIX}/${OPT_INSTALL_DIR})\n" 
        "#^#APPEND#$#\n"
        ;   

    util::StringsList sources;
    EmscriptenComms::emscriptenAddSourceFiles(m_generator, sources);
    EmscriptenDataBuf::emscriptenAddSourceFiles(m_generator, sources);
    EmscriptenVersion::emscriptenAddSourceFiles(m_generator, sources);

    for (auto& sPtr : m_generator.genSchemas()) {
        auto* s = EmscriptenSchema::cast(sPtr.get());
        s->emscriptenAddSourceFiles(sources);
    }

    util::ReplacementMap repl = {
        {"PROJ_NAME", m_generator.genProtocolSchema().genMainNamespace()},
        {"APPEND", util::genReadFileContents(util::genPathAddElem(m_generator.genGetCodeDir(), strings::genCmakeListsFileStr()) + strings::genAppendFileSuffixStr())},
        {"SOURCES", util::genStrListToString(sources, "\n", "")},
        {"EXTRA_SOURCES", util::genReadFileContents(util::genPathAddElem(m_generator.genGetCodeDir(), strings::genCmakeListsFileStr()) + strings::genSourcesFileSuffixStr())},
    };

    auto str = commsdsl::gen::util::genProcessTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.genLogger().genError("Failed to write " + filePath + ".");
        return false;
    }

    return true;
}

} // namespace commsdsl2emscripten
