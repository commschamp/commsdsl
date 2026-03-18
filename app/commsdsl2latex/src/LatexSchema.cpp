//
// Copyright 2021 - 2026 (C). Alex Robenko. All rights reserved.
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

#include "LatexSchema.h"

#include "LatexGenerator.h"
#include "LatexNamespace.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2latex
{

LatexSchema::LatexSchema(LatexGenerator& generator, ParseSchema parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

LatexSchema::~LatexSchema() = default;

std::string LatexSchema::latexRelDirPath() const
{
    return genMainNamespace();
}

std::string LatexSchema::latexRelFilePath() const
{
    return latexRelDirPath() + "/schema" + strings::genLatexSuffixStr();
}

std::string LatexSchema::latexTitle() const
{
    auto& latexGenerator = LatexGenerator::latexCast(genGenerator());
    auto& displayName = genParseObj().parseDisplayName();
    if (!displayName.empty()) {
        return LatexGenerator::latexEscString(displayName);
    }

    auto& schemas = latexGenerator.genSchemas();
    if (schemas.size() == 1U) {
        return std::string();
    }

    auto& name = genParseObj().parseName();
    if (&(latexGenerator.genProtocolSchema()) == this) {
        return "Protocol \"" + LatexGenerator::latexEscString(name) + "\"";
    }

    return "Schema \"" + LatexGenerator::latexEscString(name) + "\"";
}

bool LatexSchema::genWriteImpl()
{
    auto& latexGenerator = LatexGenerator::latexCast(genGenerator());
    auto filePath = util::genPathAddElem(latexGenerator.genGetOutputDir(), latexRelFilePath());

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
        auto replaceFileName = latexRelFilePath() + strings::genReplaceFileSuffixStr();
        bool hasReplace = false;
        auto replaceContents = latexGenerator.genReadCodeInjectCode(replaceFileName, "Replace the whole file", &hasReplace);
        if (hasReplace) {
            stream << replaceContents;
            break;
        }

        static const std::string Templ =
            "#^#GENERATED#$#\n"
            "#^#REPLACE_COMMENT#$#\n"
            "#^#SECTION#$#\n"
            "#^#LABEL#$#\n"
            "\n"
            "#^#DESCRIPTION#$#\n"
            "\n"
            "#^#PREPEND#$#\n"
            "#^#INPUTS#$#\n"
            "#^#APPEND#$#\n"
            ;

        auto prependFileName = latexRelFilePath() + strings::genPrependFileSuffixStr();
        auto appendFileName = latexRelFilePath() + strings::genAppendFileSuffixStr();
        util::GenReplacementMap repl = {
            {"GENERATED", LatexGenerator::latexFileGeneratedComment()},
            {"INPUTS", latexNamespaceInputs()},
            {"REPLACE_COMMENT", std::move(replaceContents)},
            {"PREPEND", latexGenerator.genReadCodeInjectCode(prependFileName, "Prepend to details")},
            {"APPEND", latexGenerator.genReadCodeInjectCode(appendFileName, "Append to file")},
        };

        auto title = latexTitle();
        if (!title.empty()) {
            repl["SECTION"] = LatexGenerator::latexSectionDirective(*this) + '{' + title + '}';
            repl["LABEL"] = "\\label{" + LatexGenerator::latexLabelId(*this) + '}';
            repl["DESCRIPTION"] = util::genStrMakeMultiline(LatexGenerator::latexEscString(genParseObj().parseDescription()));

            LatexGenerator::latexEnsureNewLineBreak(repl["DESCRIPTION"]);

            if (repl["DESCRIPTION"].empty()) {
                repl["DESCRIPTION"] =
                    LatexGenerator::latexSchemaCommentPrefix() +
                        "Use \"" + strings::genDescriptionStr() + "\" DSL element property to introduce description";
            }
        }

        stream << util::genProcessTemplate(Templ, repl, true) << std::endl;
    } while (false);

    stream.flush();
    if (!stream.good()) {
        latexGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

std::string LatexSchema::latexNamespaceInputs() const
{
    util::GenStringsList inputs;
    for (auto& ns : genNamespaces()) {
        auto& latexNs = LatexNamespace::latexCast(*ns);
        auto nsInput = latexNs.latexRelFilePath();
        if (nsInput.empty()) {
            continue;
        }

        inputs.push_back("\\input{" + nsInput + '}');
    }

    if (inputs.empty()) {
        return strings::genEmptyString();
    }

    return util::genStrListToString(inputs, "\n", "\n");
}

} // namespace commsdsl2latex
