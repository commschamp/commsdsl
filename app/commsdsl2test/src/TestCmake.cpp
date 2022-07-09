//
// Copyright 2019 - 2022 (C). Alex Robenko. All rights reserved.
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

#include "TestCmake.h"

#include "TestGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"
#include "commsdsl/gen/comms.h"

#include <fstream>
#include <cassert>

namespace commsdsl2test
{

namespace 
{

using ReplacementMap = commsdsl::gen::util::ReplacementMap;

} // namespace 
    

bool TestCmake::write(TestGenerator& generator)
{
    TestCmake obj(generator);
    return obj.testWriteInternal();
}

bool TestCmake::testWriteInternal() const
{
    static_cast<void>(m_generator);
    auto filePath = 
        commsdsl::gen::util::pathAddElem(
            m_generator.getOutputDir(), commsdsl::gen::strings::cmakeListsFileStr());    

    m_generator.logger().info("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    std::string interfaceScope;
    auto allInterfaces = m_generator.getAllInterfaces();
    if (!allInterfaces.empty()) {
        auto* firstInterface = allInterfaces.front();
        assert(!firstInterface->name().empty());
        interfaceScope = commsdsl::gen::comms::scopeFor(*firstInterface, m_generator);
    }
    else {
        interfaceScope = commsdsl::gen::comms::scopeForInterface(commsdsl::gen::strings::messageClassStr(), m_generator);
    }

    auto allFrames = m_generator.getAllFrames();
    assert(!allFrames.empty());
    auto* firstFrame = allFrames.front();
    assert(!firstFrame->name().empty());


    ReplacementMap repl = {
        {"PROJ_NAME", m_generator.currentSchema().schemaName()},
        {"PROJ_NS", m_generator.currentSchema().mainNamespace()},
        {"INTERFACE_SCOPE", std::move(interfaceScope)},
        {"FRAME_SCOPE", commsdsl::gen::comms::scopeFor(*firstFrame, m_generator)},
        {"OPTIONS_SCOPE", commsdsl::gen::comms::scopeForOptions(commsdsl::gen::strings::defaultOptionsStr(), m_generator)},
        {"INPUT_SCOPE", commsdsl::gen::comms::scopeForInput(commsdsl::gen::strings::allMessagesStr(), m_generator)},
    };

    static const std::string Template =
        "cmake_minimum_required (VERSION 3.1)\n"
        "project (\"#^#PROJ_NAME#$#_test\")\n\n"
        "option (OPT_WARN_AS_ERR \"Treat warning as error\" ON)\n"
        "option (OPT_USE_CCACHE \"Use of ccache on UNIX system\" ON)\n"
        "# Other parameters:\n"
        "# OPT_TEST_OPTIONS - Class name of the options for test applications,\n"
        "#       defaults to #^#OPTIONS_SCOPE#$#.\n"        
        "# OPT_TEST_INTERFACE - Class name of the interface for test applications,\n"
        "#       defaults to #^#INTERFACE_SCOPE#$#.\n"
        "# OPT_TEST_FRAME - Class name of the frame for test applications,\n"
        "#       defaults to #^#FRAME_SCOPE#$#.\n"
        "# OPT_TEST_INPUT_MESSAGES - All input messages bundle for test applications,\n"
        "#       defaults to #^#INPUT_SCOPE#$#.\n"
        "# OPT_MSVC_FORCE_WARN_LEVEL - Force msvc warning level\n\n"
        "if (CMAKE_TOOLCHAIN_FILE AND EXISTS ${CMAKE_TOOLCHAIN_FILE})\n"
        "    message(STATUS \"Loading toolchain from ${CMAKE_TOOLCHAIN_FILE}\")\n"
        "endif()\n\n"
        "if (NOT CMAKE_CXX_STANDARD)\n"
        "    set (CMAKE_CXX_STANDARD 11)\n"
        "endif()\n\n"     
        "include(GNUInstallDirs)\n"
        "######################################################################\n"
        "function (define_test name)\n"
        "    set (src ${name}.cpp)\n"
        "    add_executable(${name} ${src})\n"
        "    target_link_libraries(${name} PRIVATE cc::#^#PROJ_NS#$# cc::comms)\n"
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
        "        DESTINATION ${CMAKE_INSTALL_BINDIR}\n"
        "    )\n\n"
        "    if (TARGET ${CC_EXTERNAL_TGT})\n"
        "        add_dependencies(${name} ${CC_EXTERNAL_TGT})\n"
        "    endif ()\n\n"        
        "    target_compile_options(${name} PRIVATE\n"
        "        $<$<CXX_COMPILER_ID:MSVC>:/wd4996 /bigobj>\n"
        "        $<$<CXX_COMPILER_ID:GNU>:-Wno-unused-function -ftemplate-depth=2048>\n"
        "        $<$<CXX_COMPILER_ID:Clang>:-Wno-unused-function -Wno-unneeded-internal-declaration -ftemplate-depth=2048>\n"
        "    )\n"
        "endfunction ()\n\n"
        "######################################################################\n\n"
        "find_package (LibComms REQUIRED)\n"
        "find_package (#^#PROJ_NS#$# REQUIRED)\n\n"
        "include(${LibComms_DIR}/CC_Compile.cmake)\n"
        "cc_compile(WARN_AS_ERR)\n"
        "cc_msvc_force_warn_opt(/W4)\n\n"
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
        "define_test(#^#PROJ_NS#$#_input_test)\n";

    auto str = commsdsl::gen::util::processTemplate(Template, repl);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

} // namespace commsdsl2test