//
// Copyright 2025 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

#include "Latex.h"

#include "LatexGenerator.h"
#include "LatexSchema.h"

#include "commsdsl/gen/util.h"
#include "commsdsl/gen/strings.h"

#include <cassert>
#include <fstream>

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2latex
{

bool Latex::latexWrite(LatexGenerator& generator)
{
    Latex obj(generator);
    return obj.latexWriteInternal() && obj.latexWriteCfgInternal();
}

std::string Latex::latexDocFileBaseName(const LatexGenerator& generator)
{
    return generator.genProtocolSchema().genMainNamespace() + "_doc";
}

std::string Latex::latexDocTexFileName(const LatexGenerator& generator)
{
    return latexDocFileBaseName(generator) + strings::genLatexSuffixStr();
}

std::string Latex::latexDocCfgFileName(const LatexGenerator& generator)
{
    return latexDocFileBaseName(generator) + ".cfg";
}

bool Latex::latexWriteInternal()
{
    auto docName = latexDocTexFileName(m_latexGenerator);

    auto filePath = util::genPathAddElem(m_latexGenerator.genGetOutputDir(), docName);

    m_latexGenerator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_latexGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    do {
        auto replaceFileName = latexDocTexFileName(m_latexGenerator) + strings::genReplaceFileSuffixStr();
        bool hasReplace = false;
        auto replaceContents = m_latexGenerator.genReadCodeInjectCode(replaceFileName, "Replace the whole file", &hasReplace);
        if (hasReplace) {
            stream << replaceContents;
            break;
        }

        const std::string Templ =
            "#^#GEN_COMMENT#$#\n"
            "#^#REPLACE_COMMENT#$#\n"
            "#^#DOCUMENT#$#\n"
            "#^#PACKAGE#$#\n"
            "#^#MACRO#$#\n"
            "#^#TITLE#$#\n"
            "\\begin{document}\n\n"
            "\\maketitle\n\n"
            "\\tableofcontents\n\n"
            "#^#CONTENTS#$#\n"
            "\\end{document}\n"
            ;

        util::GenReplacementMap repl = {
            {"GEN_COMMENT", m_latexGenerator.latexFileGeneratedComment()},
            {"DOCUMENT", latexDocumentInternal()},
            {"PACKAGE", latexPackageInternal()},
            {"MACRO", latexMacroInternal()},
            {"CONTENTS", latexContentsInternal()},
            {"TITLE", latexTitleInternal()},
            {"REPLACE_COMMENT", std::move(replaceContents)},
        };

        auto str = commsdsl::gen::util::genProcessTemplate(Templ, repl, true);
        stream << str;
    } while (false);

    stream.flush();
    if (!stream.good()) {
        m_latexGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

bool Latex::latexWriteCfgInternal()
{
    auto docName = latexDocCfgFileName(m_latexGenerator);

    auto filePath = util::genPathAddElem(m_latexGenerator.genGetOutputDir(), docName);

    m_latexGenerator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        m_latexGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    do {
        auto replaceFileName = docName + strings::genReplaceFileSuffixStr();
        bool hasReplace = false;
        auto replaceContents = m_latexGenerator.genReadCodeInjectCode(replaceFileName, "Replace the whole file", &hasReplace);
        if (hasReplace) {
            stream << replaceContents;
            break;
        }

        const std::string Templ =
            "#^#GEN_COMMENT#$#\n"
            "#^#REPLACE_COMMENT#$#\n"
            "\\Preamble{xhtml}\n"
            "\\Css{table.longtable {margin-left:0; margin-right:auto;}}"
            "\\begin{document}\n"
            "\\EndPreamble\n"
            ;

        util::GenReplacementMap repl = {
            {"GEN_COMMENT", m_latexGenerator.latexFileGeneratedComment()},
            {"REPLACE_COMMENT", std::move(replaceContents)},
        };

        auto str = commsdsl::gen::util::genProcessTemplate(Templ, repl, true);
        stream << str;
    } while (false);

    stream.flush();
    if (!stream.good()) {
        m_latexGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string Latex::latexDocumentInternal() const
{
    auto replaceFileName = latexDocTexFileName(m_latexGenerator) + strings::genDocumentFileSuffixStr();
    bool hasReplace = false;
    auto replaceContents = m_latexGenerator.genReadCodeInjectCode(replaceFileName, "Replace document class definition", &hasReplace);
    if (hasReplace) {
        return replaceContents;
    }

    const std::string Templ =
        "#^#REPLACE_COMMENT#$#\n"
        "\\documentclass{article}\n"
    ;

    util::GenReplacementMap repl {
        {"REPLACE_COMMENT", std::move(replaceContents)},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string Latex::latexPackageInternal() const
{
    auto replaceFileName = latexDocTexFileName(m_latexGenerator) + strings::genPackageFileSuffixStr();
    bool hasReplace = false;
    auto replaceContents = m_latexGenerator.genReadCodeInjectCode(replaceFileName, "Replace packages definition", &hasReplace);
    if (hasReplace) {
        return replaceContents;
    }

    const std::string Templ =
        "#^#REPLACE_COMMENT#$#\n"
        "\\usepackage[T1]{fontenc}\n"
        "\\usepackage[colorlinks]{hyperref}\n"
        "\\usepackage{nameref}\n"
        "\\usepackage{array}\n"
        "\\usepackage{booktabs}\n"
        "\\usepackage{longtable}\n"
        "\n"
        "\\setlength\\LTleft{15pt}\n"
        "\\setlength\\LTright{15pt}\n"
        "\n"
        "#^#APPEND#$#\n"
    ;

    auto appendFileName = latexDocTexFileName(m_latexGenerator) + strings::genPackageAppendFileSuffixStr();
    util::GenReplacementMap repl = {
        {"REPLACE_COMMENT", std::move(replaceContents)},
        {"APPEND", m_latexGenerator.genReadCodeInjectCode(appendFileName, "Append packages definition")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string Latex::latexMacroInternal() const
{
    auto replaceFileName = latexDocTexFileName(m_latexGenerator) + strings::genMacroFileSuffixStr();
    bool hasReplace = false;
    auto replaceContents = m_latexGenerator.genReadCodeInjectCode(replaceFileName, "Replace macros definition", &hasReplace);
    if (hasReplace) {
        return replaceContents;
    }

    const std::string Templ =
        "#^#REPLACE_COMMENT#$#\n"
        "% Counter for internal anchors\n"
        "\\newcounter{dummyctr}\n\n"
        "% Fake heading\n"
        "\\newcommand{\\subsubparagraph}[1]{%\n"
        "\\par\\medskip\n"
        "\\noindent\n"
        "\\phantomsection\n"
        "\\refstepcounter{dummyctr}\n"
        "\\def\\@currentlabelname{#1}\n"
        "\\textbf{#1}\n"
        "\\par\\smallskip\n"
        "}\n"
        "#^#APPEND#$#\n"
    ;

    auto appendFileName = latexDocTexFileName(m_latexGenerator) + strings::genMacroAppendFileSuffixStr();
    util::GenReplacementMap repl = {
        {"REPLACE_COMMENT", std::move(replaceContents)},
        {"APPEND", m_latexGenerator.genReadCodeInjectCode(appendFileName, "Append macros definition")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string Latex::latexContentsInternal() const
{
    auto replaceFileName = latexDocTexFileName(m_latexGenerator) + strings::genContentFileSuffixStr();
    bool hasReplace = false;
    auto replaceContents = m_latexGenerator.genReadCodeInjectCode(replaceFileName, "Replace document content", &hasReplace);
    if (hasReplace) {
        return replaceContents;
    }

    const std::string Templ =
        "#^#REPLACE_COMMENT#$#\n"
        "#^#PREPEND#$#\n"
        "#^#INPUTS#$#\n"
        "#^#APPEND#$#\n"
    ;

    util::GenStringsList schemaInputs;
    auto& schemas = m_latexGenerator.genSchemas();
    for (auto& s : schemas) {
        assert(s);
        auto* latexSchema = LatexSchema::latexCast(s.get());
        schemaInputs.push_back(m_latexGenerator.latexWrapInput(latexSchema->latexRelFilePath()));
    }

    auto prependFileName = latexDocTexFileName(m_latexGenerator) + strings::genContentPrependFileSuffixStr();
    auto appendFileName = latexDocTexFileName(m_latexGenerator) + strings::genContentAppendFileSuffixStr();
    util::GenReplacementMap repl = {
        {"INPUTS", util::genStrListToString(schemaInputs, "\n", "")},
        {"REPLACE_COMMENT", std::move(replaceContents)},
        {"PREPEND", m_latexGenerator.genReadCodeInjectCode(prependFileName, "Prepend generated content")},
        {"APPEND", m_latexGenerator.genReadCodeInjectCode(appendFileName, "Append generated content")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string Latex::latexTitleInternal() const
{
    auto replaceFileName = latexDocTexFileName(m_latexGenerator) + strings::genTitleFileSuffixStr();
    bool hasReplace = false;
    auto replaceContents = m_latexGenerator.genReadCodeInjectCode(replaceFileName, "Replace title (whole section)", &hasReplace);
    if (hasReplace) {
        return replaceContents;
    }

    const std::string Templ =
        "#^#REPLACE_COMMENT#$#\n"
        "\\title{#^#TITLE#$#}\n"
        "\\date{\\today}\n"
        "#^#APPEND#$#\n"
    ;

    auto& protSchema = m_latexGenerator.genProtocolSchema();
    auto title = protSchema.genParseObj().parseDisplayName();
    if (title.empty()) {
        title = "Protocol \"" + LatexGenerator::latexEscString(protSchema.genMainNamespace()) + "\"";
    }

    auto appendFileName = latexDocTexFileName(m_latexGenerator) + strings::genTitleAppendFileSuffixStr();
    util::GenReplacementMap repl = {
        {"TITLE", std::move(title)},
        {"REPLACE_COMMENT", std::move(replaceContents)},
        {"APPEND", m_latexGenerator.genReadCodeInjectCode(appendFileName, "Append to title info")},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2latex

