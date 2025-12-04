//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "LatexFrame.h"

#include "LatexGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2latex
{

namespace
{

const std::size_t InvalidLength = std::numeric_limits<std::size_t>::max();

} // namespace

LatexFrame::LatexFrame(LatexGenerator& generator, ParseFrame parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

LatexFrame::~LatexFrame() = default;

std::string LatexFrame::latexRelFilePath() const
{
    auto& latexGenerator = LatexGenerator::latexCast(genGenerator());
    return latexGenerator.latexRelPathFor(*this) + strings::genLatexSuffixStr();
}

std::string LatexFrame::latexTitle() const
{
    auto name = LatexGenerator::latexEscDisplayName(genParseObj().parseDisplayName(), genParseObj().parseName());
    return "Frame \"" + name + "\"";
}

bool LatexFrame::genWriteImpl() const
{
    auto relFilePath = latexRelFilePath();
    if (relFilePath.empty()) {
        return true;
    }

    auto& latexGenerator = LatexGenerator::latexCast(genGenerator());
    auto filePath = util::genPathAddElem(latexGenerator.genGetOutputDir(), relFilePath);

    auto dirPath = util::genPathUp(filePath);
    assert(!dirPath.empty());
    if (!latexGenerator.genCreateDirectory(dirPath)) {
        return false;
    }

    latexGenerator.genLogger().genInfo("Generating " + filePath);
    std::ofstream stream(filePath);
    if (!stream) {
        latexGenerator.genLogger().genError("Failed to open \"" + filePath + "\" for writing.");
        return false;
    }

    do {
        auto replaceFileName = relFilePath + strings::genReplaceFileSuffixStr();
        auto replaceContents = util::genReadFileContents(latexGenerator.latexInputCodePathForFile(replaceFileName));
        if (!replaceContents.empty()) {
            stream << replaceContents;
            break;
        }

        static const std::string Templ =
            "#^#GENERATED#$#\n"
            "#^#REPLACE_COMMENT#$#\n"
            "#^#SECTION#$#"
            "#^#LABEL#$#\n"
            "\n"
            "#^#DESCRIPTION#$#\n"
            "\n"
            "#^#PREPEND#$#\n"
            "#^#FIELDS#$#\n"
            "#^#APPEND#$#\n";

        auto prependFileName = relFilePath + strings::genPrependFileSuffixStr();
        auto appendFileName = relFilePath + strings::genAppendFileSuffixStr();
        util::GenReplacementMap repl = {
            {"GENERATED", LatexGenerator::latexFileGeneratedComment()},
            {"SECTION", latexSection()},
            {"LABEL", "\\label{" + LatexGenerator::latexLabelId(*this) + '}'},
            {"DESCRIPTION", util::genStrMakeMultiline(LatexGenerator::latexEscString(genParseObj().parseDescription()))},
            {"PREPEND", util::genReadFileContents(latexGenerator.latexInputCodePathForFile(prependFileName))},
            {"APPEND", util::genReadFileContents(latexGenerator.latexInputCodePathForFile(appendFileName))},
            {"FIELDS", latexLayers()},
        };

        LatexGenerator::latexEnsureNewLineBreak(repl["DESCRIPTION"]);
        if (repl["DESCRIPTION"].empty()) {
            repl["DESCRIPTION"] =
                LatexGenerator::latexSchemaCommentPrefix() +
                    "Use \"" + strings::genDescriptionStr() + "\" DSL element property to introduce description";
        }

        if (latexGenerator.latexHasCodeInjectionComments()) {
            repl["REPLACE_COMMENT"] =
                latexGenerator.latexCodeInjectCommentPrefix() + "Replace the whole file with \"" + replaceFileName + "\".";

            if (repl["PREPEND"].empty()) {
                repl["PREPEND"] = latexGenerator.latexCodeInjectCommentPrefix() + "Prepend to details with \"" + prependFileName + "\".";
            }

            if (repl["APPEND"].empty()) {
                repl["APPEND"] = latexGenerator.latexCodeInjectCommentPrefix() + "Append to file with \"" + appendFileName + "\".";
            }
        };

        stream << util::genProcessTemplate(Templ, repl, true) << std::endl;
    } while (false);

    stream.flush();
    if (!stream.good()) {
        latexGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string LatexFrame::latexSection() const
{
    static const std::string Templ =
        "#^#REPLACE_COMMENT#$#\n"
        "#^#SECTION#$#{#^#TITLE#$#}\n"
        ;

    auto& latexGenerator = LatexGenerator::latexCast(genGenerator());
    auto titleFileName = latexRelFilePath() + strings::genTitleFileSuffixStr();
    util::GenReplacementMap repl = {
        {"SECTION", LatexGenerator::latexSectionDirective(*this)},
        {"TITLE", util::genReadFileContents(latexGenerator.latexInputCodePathForFile(titleFileName))},
    };

    if (repl["TITLE"].empty()) {
        repl["TITLE"] = latexTitle();
    }

    if (latexGenerator.latexHasCodeInjectionComments()) {
        repl["REPLACE_COMMENT"] =
            latexGenerator.latexCodeInjectCommentPrefix() + "Replace the title value with contents of \"" + titleFileName + "\".";
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string LatexFrame::latexLayers() const
{
    util::GenStringsList lines;
    util::GenStringsList fields;

    for (auto& lPtr : genLayers()) {
        auto layerParseObj = lPtr->genParseObj();
        auto nameStr = LatexGenerator::latexEscDisplayName(layerParseObj.parseDisplayName(), layerParseObj.parseName());

        auto* extField = lPtr->genExternalField();
        auto* memField = lPtr->genMemberField();

        auto* layerField = extField;
        if (layerField == nullptr) {
            layerField = memField;
        }

        std::string lengthStr("<variable>");
        if (layerField != nullptr) {
            auto minLength = layerField->genParseObj().parseMinLength();
            auto maxLength = layerField->genParseObj().parseMaxLength();

            lengthStr = std::to_string(minLength);
            do {
                if (maxLength == minLength) {
                    break;
                }

                if (maxLength == InvalidLength) {
                    lengthStr += '+';
                    break;
                }

                lengthStr += " - " + std::to_string(maxLength);
            } while (false);
        }

        auto refStr = "\\hyperref[" + LatexGenerator::latexLabelId(*lPtr) + "]{" + nameStr + "}";
        lines.push_back(lengthStr + " & " + refStr);

        static const std::string LayerTempl = {
            "#^#SECTION#$#{#^#TITLE#$#}"
            "\\label{#^#LABEL#$#}\n\n"
            "#^#DESCRIPTION#$#\n"
            "#^#NOINDENT#$#\n"
            "#^#FIELD_DESCRIPTION#$#\n"
            "#^#FIELD_INFO#$#\n"
            "#^#FIELD_EXTRA#$#\n"
        };

        util::GenReplacementMap layerRepl = {
            {"SECTION", LatexGenerator::latexSectionDirective(*lPtr)},
            {"TITLE", "Frame Field \"" + nameStr + "\""},
            {"LABEL", LatexGenerator::latexLabelId(*lPtr)},
            {"DESCRIPTION", LatexGenerator::latexEscString(layerParseObj.parseDescription())},
        };

        LatexGenerator::latexEnsureNewLineBreak(layerRepl["DESCRIPTION"]);
        if (layerRepl["DESCRIPTION"].empty()) {
            layerRepl["DESCRIPTION"] =
                LatexGenerator::latexSchemaCommentPrefix() +
                    "Use \"" + strings::genDescriptionStr() + "\" DSL element property to introduce description";
        }

        do {
            if (extField != nullptr) {
                layerRepl["NOINDENT"] = "\\noindent";
                layerRepl["FIELD_DESCRIPTION"] = "Same as \\nameref{" + LatexField::latexCast(extField)->latexRefLabelId() + "}";
                LatexGenerator::latexEnsureNewLineBreak(layerRepl["FIELD_DESCRIPTION"]);
                break;
            }

            if (memField != nullptr) {
                auto* latexMemField = LatexField::latexCast(memField);
                assert(latexMemField != nullptr);

                layerRepl["FIELD_DESCRIPTION"] = latexMemField->latexDescription();
                layerRepl["FIELD_INFO"] = latexMemField->latexInfoDetails();
                layerRepl["FIELD_EXTRA"] = latexMemField->latexExtraDetails();

                if (!layerRepl["FIELD_DESCRIPTION"].empty()) {
                    layerRepl["NOINDENT"] = "\\noindent";
                }

                LatexGenerator::latexEnsureNewLineBreak(layerRepl["FIELD_DESCRIPTION"]);
                break;
            }

            if (layerRepl["DESCRIPTION"].empty()) {
                layerRepl["FIELD_DESCRIPTION"] = "Raw data sequence.";
                LatexGenerator::latexEnsureNewLineBreak(layerRepl["FIELD_DESCRIPTION"]);
            }

        } while (false);

        fields.push_back(util::genProcessTemplate(LayerTempl, layerRepl));
    }

    static const std::string Templ =
        "\\subsubparagraph{Frame Fields}\n"
        "\\label{#^#LABEL#$#}\n\n"
        "\\fbox{%\n"
        "\\begin{tabular}{c|c}\n"
        "\\textbf{Length (Bytes)}& \\textbf{Name}\\\\\n"
        "\\hline\n"
        "\\hline\n"
        "#^#LINES#$#\n"
        "\\end{tabular}\n"
        "}\n"
        "\\smallskip\n\n"
        "#^#DETAILS#$#\n"
        "\n"
        ;

    util::GenReplacementMap repl = {
        {"LABEL", LatexGenerator::latexLabelId(*this) + "_fields"},
        {"LINES", util::genStrListToString(lines, " \\\\\\hline\n", " \\\\")},
        {"DETAILS", util::genStrListToString(fields, "\n", "\n")},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2latex
