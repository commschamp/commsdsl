//
// Copyright 2021 - 2024 (C). Alex Robenko. All rights reserved.
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

#include "SwigCmake.h"

#include "SwigGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2swig
{

bool SwigCmake::swigWrite(SwigGenerator& generator)
{
    SwigCmake obj(generator);
    return obj.swigWriteInternal();
}

bool SwigCmake::swigWriteInternal() const
{
    auto filePath = util::pathAddElem(m_generator.getOutputDir(), strings::cmakeListsFileStr());
    auto dirPath = util::pathUp(filePath);
    assert(!dirPath.empty());
    if (!m_generator.createDirectory(dirPath)) {
        return false;
    }       

    m_generator.logger().info("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_generator.logger().error("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }     

    const std::string Templ = 
        "cmake_minimum_required (VERSION 3.12)\n"
        "project (#^#PROJ_NAME#$#_swig)\n\n"
        "# Input parameters:\n"
        "# OPT_SWIG_LANGUAGES - Mandatory list of target languages for which bindings need to be generated\n"
        "# OPT_PROTOCOL_NAME - Optional parameter to override expected default protocol name. Defaults to #^#PROJ_NAME#$#.\n"
        "# OPT_PROTOCOL_TARGET - Optional parameter to override expected protocol definition target. Defaults to cc::${OPT_PROTOCOL_NAME}.\n\n"
        "if (\"${OPT_SWIG_LANGUAGES}\" STREQUAL \"\")\n"
        "    message (FATAL_ERROR \"List of target languages are not provided, use OPT_SWIG_LANGUAGES variable to do so.\")\n"
        "endif ()\n\n"
        "if (\"${OPT_PROTOCOL_NAME}\" STREQUAL \"\")\n"
        "    set (OPT_PROTOCOL_NAME #^#PROJ_NAME#$#)\n"
        "endif ()\n\n"        
        "if (\"${OPT_PROTOCOL_TARGET}\" STREQUAL \"\")\n"
        "    set (OPT_PROTOCOL_TARGET cc::${OPT_PROTOCOL_NAME})\n"
        "endif ()\n\n"
        "cmake_policy(SET CMP0078 NEW)\n"
        "cmake_policy(SET CMP0086 NEW)\n"
        "cmake_policy(SET CMP0122 NEW)\n\n"
        "find_package(LibComms REQUIRED)\n"
        "find_package(SWIG REQUIRED COMPONENTS ${OPT_SWIG_LANGUAGES})\n"
        "find_package(${OPT_PROTOCOL_NAME} REQUIRED) # Protocol definition needs to be built\n\n"
        "if (\"${UseSWIG_TARGET_NAME_PREFERENCE}\" STREQUAL \"\")\n"
        "    set (UseSWIG_TARGET_NAME_PREFERENCE STANDARD)\n"
        "endif ()\n\n"
        "if (\"${UseSWIG_MODULE_VERSION}\" STREQUAL \"\")\n"
        "    set (UseSWIG_MODULE_VERSION 2)\n"
        "endif ()\n\n"
        "include (UseSWIG)\n\n"
        "#^#PREPEND#$#\n"
        "set (swig_src_file ${PROJECT_SOURCE_DIR}/#^#PROJ_NAME#$#.i)\n"
        "set_source_files_properties(\n"
        "    ${swig_src_file}\n"
        "    PROPERTIES\n"
        "        CPLUSPLUS ON\n"
        ")\n\n"
        "set (src\n"
        "    ${swig_src_file}\n"
        "    #^#EXTRA_SOURCES#$#\n"
        ")\n\n"
        "foreach (lang ${OPT_SWIG_LANGUAGES})\n"
        "    #^#PREPEND_LANG#$#\n"
        "    set (lang_output_dir ${CMAKE_CURRENT_BINARY_DIR}/output_${lang})\n"
        "    swig_add_library(#^#PROJ_NAME#$#_swig_${lang} LANGUAGE ${lang} OUTPUT_DIR ${lang_output_dir} SOURCES ${src})\n"
        "    target_link_libraries(#^#PROJ_NAME#$#_swig_${lang} ${OPT_PROTOCOL_TARGET} cc::comms)\n"
        "    target_compile_options(#^#PROJ_NAME#$#_swig_${lang} PRIVATE\n"
        "        $<$<CXX_COMPILER_ID:GNU>:-ftemplate-depth=2048 -fconstexpr-depth=4096>\n"
        "        $<$<CXX_COMPILER_ID:Clang>:-ftemplate-depth=2048 -fconstexpr-depth=4096>\n"
        "    )\n"        
        "endforeach()\n\n"
        "#^#APPEND#$#"
        ;      

    util::ReplacementMap repl = {
        {"PROJ_NAME", m_generator.protocolSchema().mainNamespace()},
        {"PREPEND", swigPrependInternal()},
        {"PREPEND_LANG", swigPrependLangInternal()},
        {"APPEND", swigAppendInternal()},
        {"EXTRA_SOURCES", util::readFileContents(util::pathAddElem(m_generator.getCodeDir(), strings::cmakeListsFileStr()) + strings::sourcesFileSuffixStr())},
    };

    auto str = commsdsl::gen::util::processTemplate(Templ, repl, true);
    stream << str;
    stream.flush();
    if (!stream.good()) {
        m_generator.logger().error("Failed to write " + filePath + ".");
        return false;
    }

    return true;
}

std::string SwigCmake::swigPrependInternal() const
{
    auto& name = strings::cmakeListsFileStr();
    auto fromFile = util::readFileContents(m_generator.swigInputCodePathForFile(name + strings::prependFileSuffixStr()));
    if (!fromFile.empty()) {
        return fromFile;
    }

    const std::string Templ = 
        "# Use #^#FILE_NAME#$##^#SUFFIX#$# to add extra code here to find appropriate libraries and/or update swig global behaviour\n";

    util::ReplacementMap repl = {
        {"FILE_NAME", name},
        {"SUFFIX", strings::prependFileSuffixStr()},
    };

    return util::processTemplate(Templ, repl);
}

std::string SwigCmake::swigPrependLangInternal() const
{
    auto& name = strings::cmakeListsFileStr();
    std::string langSuffix(strings::prependFileSuffixStr() + "_lang");
    auto fromFile = util::readFileContents(m_generator.swigInputCodePathForFile(name + langSuffix));
    if (!fromFile.empty()) {
        return fromFile;
    }

    const std::string Templ = 
        "# Use #^#FILE_NAME#$##^#SUFFIX#$# to add extra code here to set swig variables to update language specific behavior\n";

    util::ReplacementMap repl = {
        {"FILE_NAME", name},
        {"SUFFIX", langSuffix}
    };

    return util::processTemplate(Templ, repl);
}

std::string SwigCmake::swigAppendInternal() const
{
    auto& name = strings::cmakeListsFileStr();
    auto& suffix = strings::appendFileSuffixStr();
    auto fromFile = util::readFileContents(m_generator.swigInputCodePathForFile(name + suffix));
    if (!fromFile.empty()) {
        return fromFile;
    }

    const std::string Templ = 
        "# Use #^#FILE_NAME#$##^#SUFFIX#$# to add extra code here to link previosly created language specific targets to language binding libraries\n";

    util::ReplacementMap repl = {
        {"FILE_NAME", name},
        {"SUFFIX", suffix},
    };

    return util::processTemplate(Templ, repl);
}

} // namespace commsdsl2swig
