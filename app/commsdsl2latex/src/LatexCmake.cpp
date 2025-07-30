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

#include "LatexCmake.h"

#include "Latex.h"
#include "LatexGenerator.h"

#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <fstream>

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2latex
{

namespace 
{

const std::string& latexCodeInjectPrefix()
{
    static const std::string Str = "# [CODE_INJECT]: ";
    return Str;
}

} // namespace 
    

bool LatexCmake::latexWrite(LatexGenerator& generator)
{
    LatexCmake obj(generator);
    return obj.latexWriteInternal();
}

bool LatexCmake::latexWriteInternal()
{
    auto filePath = util::genPathAddElem(m_latexGenerator.genGetOutputDir(), strings::genCmakeListsFileStr());

    m_latexGenerator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_latexGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    const std::string Templ = 
        "cmake_minimum_required (VERSION 3.10)\n"
        "project (\"#^#PROJ_NAME#$#_latex\" NONE)\n\n"
        "set (MAIN_FILE \"${CMAKE_CURRENT_SOURCE_DIR}/#^#TEX_FILE#$#\")\n\n"
        "add_custom_target(\"all_artifacts\"\n"
        "   COMMAND ${CMAKE_COMMAND} -E echo \"Built all artifacts\")\n\n"
        "#^#SECTION_PDF#$#\n\n"
        "#^#SECTION_HTML#$#\n\n"
        "#^#APPEND#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"PROJ_NAME", m_latexGenerator.genCurrentSchema().genSchemaName()},
        {"TEX_FILE", Latex::latexDocTexFileName(m_latexGenerator)},
        {"SECTION_PDF", latexSectionPdf()},
        {"SECTION_HTML", latexSectionHtml()},
        {"APPEND", latexAppendInternal()},
    };

    auto str = commsdsl::gen::util::genProcessTemplate(Templ, repl, true);
    stream << str;

    stream.flush();
    if (!stream.good()) {
        m_latexGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }


    return true;
}

std::string LatexCmake::latexSectionPdf() const
{
    auto replaceFileName = strings::genCmakeListsFileStr() + strings::genPdfFileSuffixStr();
    auto replaceContents = util::genReadFileContents(m_latexGenerator.latexInputCodePathForFile(replaceFileName));
    if (!replaceContents.empty()) {
        return replaceContents;
    }

    const std::string Templ = 
        "#^#REPLACE_COMMENT#$#\n"
        "# PDF Generation\n"
        "find_program (PDFLATEX_EXE \"pdflatex\")\n"
        "if (PDFLATEX_EXE)\n"
        "    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/pdf)\n"
        "    # Run generation twice for table of contents\n"
        "    add_custom_target(\"pdf\"\n"
        "        COMMAND ${CMAKE_COMMAND} -E env TEXINPUTS=\"${CMAKE_CURRENT_SOURCE_DIR}:\" -- ${PDFLATEX_EXE} -file-line-error -output-directory ${CMAKE_CURRENT_BINARY_DIR}/pdf ${MAIN_FILE}\n"
        "        COMMAND ${CMAKE_COMMAND} -E env TEXINPUTS=\"${CMAKE_CURRENT_SOURCE_DIR}:\" -- ${PDFLATEX_EXE} -file-line-error -output-directory ${CMAKE_CURRENT_BINARY_DIR}/pdf ${MAIN_FILE}\n"        
        "        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/pdf\n"
        "        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/pdf/#^#FILE_BASE#$#.pdf ${CMAKE_INSTALL_PREFIX}/pdf/\n"
        "        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/pdf)\n"        
        "    add_dependencies(\"all_artifacts\" \"pdf\")\n"
        "    #^#APPEND#$#\n"
        "endif()\n";

    auto appendFileName = strings::genCmakeListsFileStr() + strings::genPdfAppendFileSuffixStr();
    util::GenReplacementMap repl = {
        {"FILE_BASE", Latex::latexDocFileBaseName(m_latexGenerator)},
        {"APPEND", util::genReadFileContents(m_latexGenerator.latexInputCodePathForFile(appendFileName))},
    };

    if (m_latexGenerator.latexHasCodeInjectionComments()) {
        repl["REPLACE_COMMENT"] = latexCodeInjectPrefix() + "Use \"" + replaceFileName + "\" file to replace default PDF generation section.";

        if (repl["APPEND"].empty()) {
            repl["APPEND"] = latexCodeInjectPrefix() + "Use \"" + appendFileName + "\" file to append to default PDF generation section.";
        }
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string LatexCmake::latexSectionHtml() const
{
    auto replaceFileName = strings::genCmakeListsFileStr() + strings::genHtmlFileSuffixStr();
    auto replaceContents = util::genReadFileContents(m_latexGenerator.latexInputCodePathForFile(replaceFileName));
    if (!replaceContents.empty()) {
        return replaceContents;
    }

    const std::string Templ = 
        "#^#REPLACE_COMMENT#$#\n"
        "# HTML Generation\n"
        "find_program (HTLATEX_EXE \"htlatex\")\n"
        "if (HTLATEX_EXE)\n"
        "    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/html)\n\n"
        "    # Run generation twice for table of contents\n"
        "    add_custom_target(\"html\"\n"
        "        COMMAND ${CMAKE_COMMAND} -E env TEXINPUTS=\"${CMAKE_CURRENT_SOURCE_DIR}:\" -- ${HTLATEX_EXE} ${MAIN_FILE}\n"
        "        COMMAND ${CMAKE_COMMAND} -E env TEXINPUTS=\"${CMAKE_CURRENT_SOURCE_DIR}:\" -- ${HTLATEX_EXE} ${MAIN_FILE}\n"
        "        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/html\n"
        "        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/html/#^#FILE_BASE#$#.html ${CMAKE_INSTALL_PREFIX}/html/\n"
        "        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/html/#^#FILE_BASE#$#.css ${CMAKE_INSTALL_PREFIX}/html/\n"
        "        #^#CMD_APPEND#$#\n"
        "        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html)\n"        
        "    add_dependencies(\"all_artifacts\" \"html\")\n"
        "    #^#APPEND#$#\n"
        "endif()\n";

    auto appendFileName = strings::genCmakeListsFileStr() + strings::genHtmlAppendFileSuffixStr();
    auto cmdAppendFileName = strings::genCmakeListsFileStr() + strings::genHtmlCmdAppendFileSuffixStr();
    util::GenReplacementMap repl = {
        {"FILE_BASE", Latex::latexDocFileBaseName(m_latexGenerator)},
        {"APPEND", util::genReadFileContents(m_latexGenerator.latexInputCodePathForFile(appendFileName))},
        {"CMD_APPEND", util::genReadFileContents(m_latexGenerator.latexInputCodePathForFile(cmdAppendFileName))},
    };

    if (m_latexGenerator.latexHasCodeInjectionComments()) {
        repl["REPLACE_COMMENT"] =latexCodeInjectPrefix() + "Use \"" + replaceFileName + "\" file to replace default HTML generation section.";

        if (repl["APPEND"].empty()) {
            repl["APPEND"] = latexCodeInjectPrefix() + "Use \"" + appendFileName + "\" file to append to default HTML generation section.";
        }

        if (repl["CMD_APPEND"].empty()) {
            repl["CMD_APPEND"] = latexCodeInjectPrefix() + "Use \"" + cmdAppendFileName + "\" file to append to default HTML generation commands.";
        }        
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string LatexCmake::latexAppendInternal() const
{
    auto replaceFileName = strings::genCmakeListsFileStr() + strings::genAppendFileSuffixStr();
    auto replaceContents = util::genReadFileContents(m_latexGenerator.latexInputCodePathForFile(replaceFileName));
    if (!replaceContents.empty()) {
        return replaceContents;
    }

    if (m_latexGenerator.latexHasCodeInjectionComments()) {
        return latexCodeInjectPrefix() + "Use \"" + replaceFileName + "\" file to append extra code to this file.";
    }

    return strings::genEmptyString();
}

} // namespace commsdsl2latex

