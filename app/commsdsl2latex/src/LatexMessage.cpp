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

#include "LatexMessage.h"

#include "LatexGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include "commsdsl/parse/ParseProtocol.h"

#include <cassert>
#include <fstream>
#include <iomanip>
#include <limits>
#include <type_traits>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2latex
{

namespace
{

const std::string& latexSenderStr(LatexMessage::ParseMessage::ParseSender sender)
{
    static const std::string Map[] = {
        /* Both */ "Client + Server",
        /* Client */ "Client Only",
        /* Server */ "Server Only",
    };
    static const std::size_t MapSize = std::extent_v<decltype(Map)>;
    static_assert(MapSize == static_cast<unsigned>(LatexMessage::ParseMessage::ParseSender::NumOfValues));

    auto idx = static_cast<unsigned>(sender);
    assert(idx < MapSize);
    return Map[idx];
}

} // namespace

LatexMessage::LatexMessage(LatexGenerator& generator, ParseMessage parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

LatexMessage::~LatexMessage() = default;

std::string LatexMessage::latexRelFilePath() const
{
    if (!genIsReferenced()) {
        return strings::genEmptyString();
    }

    auto& latexGenerator = LatexGenerator::latexCast(genGenerator());
    return latexGenerator.latexRelPathFor(*this) + strings::genLatexSuffixStr();
}

std::string LatexMessage::latexTitle() const
{
    auto name = LatexGenerator::latexEscDisplayName(genParseObj().parseDisplayName(), genParseObj().parseName());
    return "Message \"" + name + "\"";
}

bool LatexMessage::genPrepareImpl()
{
    m_latexFields = LatexField::latexTransformFieldsList(genFields());
    return true;
}

bool LatexMessage::genWriteImpl() const
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
            "#^#DESCRIPTION#$#\n"
            "#^#PREPEND#$#\n"
            "#^#INFO#$#\n"
            "#^#FIELDS_SUMMARY#$#\n"
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
            {"INFO", latexInfoDetails()},
            {"FIELDS_SUMMARY", latexFields()},
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

std::string LatexMessage::latexSection() const
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

std::string LatexMessage::latexInfoDetails() const
{
    static const std::string Templ =
        "\\subsubparagraph{Details}\n"
        "\\label{#^#LABEL#$#}\n\n"
        "\\fbox{%\n"
        "\\begin{tabular}{l|c}\n"
        "#^#LINES#$#\n"
        "\\end{tabular}\n"
        "}\n"
        "\\smallskip\n"
        "\n"
        ;

    util::GenStringsList lines;

    auto parseObj = genParseObj();
    do{
        lines.push_back("\\textbf{ID} & " + LatexGenerator::latexIntegralToStr(parseObj.parseId(), 2));
    } while (false);

    do{
        lines.push_back("\\textbf{Sent By} & " + latexSenderStr(parseObj.parseSender()));
    } while (false);

    do {
        auto minLength = parseObj.parseMinLength();
        auto maxLength = parseObj.parseMaxLength();
        if (minLength == maxLength) {
            lines.push_back("\\textbf{Fixed Length} & " + std::to_string(minLength) + " Bytes");
            break;
        }

        if (maxLength != std::numeric_limits<std::size_t>::max()) {
            lines.push_back("\\textbf{Variable Length} & " + std::to_string(minLength) + " - " + std::to_string(maxLength) + " Bytes");
            break;
        }

        lines.push_back("\\textbf{Variable Length} & " + std::to_string(minLength) + "+ Bytes");
    } while (false);

    do {
        auto sinceVersion = parseObj.parseSinceVersion();
        if (sinceVersion == 0) {
            break;
        }

        lines.push_back("\\textbf{Introduced In Version} &" + LatexGenerator::latexIntegralToStr(sinceVersion));
    } while (false);

    do {
        auto deprecatedSince = parseObj.parseDeprecatedSince();
        if (deprecatedSince == commsdsl::parse::ParseProtocol::parseNotYetDeprecated()) {
            break;
        }

        lines.push_back("\\textbf{Deprecated In Version} &" + LatexGenerator::latexIntegralToStr(deprecatedSince));
    } while (false);

    util::GenReplacementMap repl = {
        {"LABEL", LatexGenerator::latexLabelId(*this) + "_details"},
        {"LINES", util::genStrListToString(lines, " \\\\\\hline\n", " \\\\\n")},
    };

    return util::genProcessTemplate(Templ, repl);
}

std::string LatexMessage::latexFields() const
{
    if (m_latexFields.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "\\subsubparagraph{Member Fields}\n"
        "\\label{#^#LABEL#$#}\n\n"
        "#^#DETAILS#$#\n"
        "\n"
        ;

    util::GenReplacementMap repl = {
        {"LABEL", LatexGenerator::latexLabelId(*this) + "_fields"},
        {"DETAILS", LatexField::latexMembersDetails(m_latexFields)},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2latex
