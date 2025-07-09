//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
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

#include <cassert>
#include <fstream>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;


namespace commsdsl2test
{

namespace 
{

using GenReplacementMap = commsdsl::gen::util::GenReplacementMap;

} // namespace 
    

bool TestCmake::write(TestGenerator& generator)
{
    TestCmake obj(generator);
    return obj.testWriteInternal();
}

bool TestCmake::testWriteInternal() const
{
    auto filePath = 
        commsdsl::gen::util::genPathAddElem(
            m_generator.genGetOutputDir(), commsdsl::gen::strings::genCmakeListsFileStr());    

    m_generator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    auto allInterfaces = m_generator.genGetAllInterfaces();
    assert(!allInterfaces.empty());
    auto* firstInterface = allInterfaces.front();
    auto interfaceScope = commsdsl::gen::comms::genScopeFor(*firstInterface, m_generator);

    auto allFrames = m_generator.genGetAllFrames();
    assert(!allFrames.empty());
    auto* firstFrame = allFrames.front();
    assert(!firstFrame->genName().empty());

    auto* interfaceNs = firstInterface->genParentNamespace();

    auto* inputNs = firstFrame->genParentNamespace();
    while (inputNs != nullptr) {
        if (inputNs->genHasMessagesRecursive()) {
            break;
        }

        auto* parentNs = inputNs->genGetParent();
        if ((parentNs != nullptr) && (parentNs->genElemType() != commsdsl::gen::GenElem::Type_Namespace)) {
            inputNs = nullptr;
            break;
        }

        inputNs = static_cast<decltype(inputNs)>(parentNs);
    }

    if (inputNs == nullptr) {
        inputNs = interfaceNs;
    }

    GenReplacementMap repl = {
        {"PROJ_NAME", m_generator.genCurrentSchema().genSchemaName()},
        {"PROJ_NS", m_generator.genCurrentSchema().genMainNamespace()},
        {"INTERFACE_SCOPE", std::move(interfaceScope)},
        {"FRAME_SCOPE", commsdsl::gen::comms::genScopeFor(*firstFrame, m_generator)},
        {"OPTIONS_SCOPE", commsdsl::gen::comms::genScopeForOptions(commsdsl::gen::strings::genDefaultOptionsStr(), m_generator)},
        {"INPUT_SCOPE", commsdsl::gen::comms::genScopeForInput(commsdsl::gen::strings::genAllMessagesStr(), m_generator, *inputNs)},
        {"EXTRA_SOURCES", util::genReadFileContents(util::genPathAddElem(m_generator.genGetCodeDir(), strings::genCmakeListsFileStr()) + strings::genSourcesFileSuffixStr())},
    };

    static const std::string Template =
        "cmake_minimum_required (VERSION 3.10)\n"
        "project (\"#^#PROJ_NAME#$#_test\")\n\n"
        "option (OPT_WARN_AS_ERR \"Treat warning as error\" ON)\n"
        "option (OPT_USE_CCACHE \"Use of ccache\" OFF)\n"
        "option (OPT_WITH_DEFAULT_SANITIZERS \"Build with sanitizers\" OFF)\n\n"
        "# Other parameters:\n"
        "# OPT_TEST_RENAME - Rename the final test application.\n"
        "# OPT_TEST_OPTIONS - Class name of the options for test applications,\n"
        "#       defaults to #^#OPTIONS_SCOPE#$#.\n"        
        "# OPT_TEST_INTERFACE - Class name of the interface for test applications,\n"
        "#       defaults to #^#INTERFACE_SCOPE#$#.\n"
        "# OPT_TEST_FRAME - Class name of the frame for test applications,\n"
        "#       defaults to #^#FRAME_SCOPE#$#.\n"
        "# OPT_TEST_INPUT_MESSAGES - All input messages bundle for test applications,\n"
        "#       defaults to #^#INPUT_SCOPE#$#.\n"
        "# OPT_MSVC_FORCE_WARN_LEVEL - Force msvc warning level\n\n"
        "# Extra configuration variables:\n"
        "# OPT_CCACHE_EXECUTABLE - Custom ccache executable\n\n"
        "######################################################################\n\n"
        "if (\"${CMAKE_CXX_STANDARD}\" STREQUAL \"\")\n"
        "    set(CMAKE_CXX_STANDARD 11)\n"
        "endif()\n\n"
        "include(GNUInstallDirs)\n"
        "######################################################################\n"
        "function (define_test name)\n"
        "    set (src\n"
        "        ${name}.cpp\n"
        "        #^#EXTRA_SOURCES#$#\n"
        "    )\n\n"
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
        "    set (rename_param)\n"
        "    if (NOT \"${OPT_TEST_RENAME}\" STREQUAL \"\")\n"
        "        set (rename_param RENAME ${OPT_TEST_RENAME})\n"
        "    endif()\n\n"
        "    install (\n"
        "        TARGETS ${name}\n"
        "        DESTINATION ${CMAKE_INSTALL_BINDIR}\n"
        "        ${rename_param}\n"
        "    )\n\n"
        "    if (TARGET ${CC_EXTERNAL_TGT})\n"
        "        add_dependencies(${name} ${CC_EXTERNAL_TGT})\n"
        "    endif ()\n\n"        
        "    target_compile_options(${name} PRIVATE\n"
        "        $<$<CXX_COMPILER_ID:MSVC>:/wd4996 /bigobj>\n"
        "        $<$<CXX_COMPILER_ID:GNU>:-Wno-unused-function -ftemplate-depth=2048 -fconstexpr-depth=4096>\n"
        "        $<$<CXX_COMPILER_ID:Clang>:-Wno-unused-function -Wno-unneeded-internal-declaration -ftemplate-depth=2048 -fconstexpr-depth=4096>\n"
        "    )\n"
        "endfunction ()\n\n"
        "######################################################################\n\n"
        "find_package (LibComms REQUIRED)\n"
        "find_package (#^#PROJ_NS#$# REQUIRED)\n\n"
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

    auto str = util::genProcessTemplate(Template, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }
    
    return true;    
}

} // namespace commsdsl2test