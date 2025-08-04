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

#include "LatexField.h"

#include "LatexGenerator.h"

#include "commsdsl/gen/comms.h"
#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <fstream>

namespace comms = commsdsl::gen::comms;
namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2latex
{

LatexField::LatexField(const commsdsl::gen::GenField& field) :
    m_genField(field)
{
}

LatexField::~LatexField() = default;

std::string LatexField::latexRelFilePath() const
{
    assert(comms::genIsGlobalField(m_genField));
    if (!m_genField.genIsReferenced()) {
        // Not referenced fields do not need to be written
        return strings::genEmptyString();
    }    
    
    auto& latexGenerator = LatexGenerator::latexCast(m_genField.genGenerator());
    return latexGenerator.latexRelPathFor(m_genField) + strings::genLatexSuffixStr();
}

std::string LatexField::latexTitle() const
{
    auto name = LatexGenerator::latexEscDisplayName(m_genField.genParseObj().parseDisplayName(), m_genField.genParseObj().parseName());
    return "Field \"" + name + "\"";
}

LatexField::LatexFieldsList LatexField::latexTransformFieldsList(const GenFieldsList& fields)
{
    LatexFieldsList result;
    result.reserve(fields.size());
    for (auto& fPtr : fields) {
        assert(fPtr);

        auto* latexField = 
            const_cast<LatexField*>(
                dynamic_cast<const LatexField*>(fPtr.get()));

        assert(latexField != nullptr);
        result.push_back(latexField);
    }

    return result;
}

bool LatexField::latexWrite() const
{
    if (!comms::genIsGlobalField(m_genField)) {
        // Skip write for non-global fields,
        // The code generation will be driven by other means        
        return true;
    }

    if (!m_genField.genIsReferenced()) {
        // Not referenced fields do not need to be written
        m_genField.genGenerator().genLogger().genDebug(
            "Skipping file generation for non-referenced field \"" + m_genField.genParseObj().parseExternalRef() + "\".");
        return true;
    }

    auto& latexGenerator = LatexGenerator::latexCast(m_genField.genGenerator());
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

    // TODO:
    stream.flush();
    if (!stream.good()) {
        latexGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    do {
        auto replaceFileName = latexRelFilePath() + strings::genReplaceFileSuffixStr();
        auto replaceContents = util::genReadFileContents(latexGenerator.latexInputCodePathForFile(replaceFileName));
        if (!replaceContents.empty()) {
            stream << replaceContents;
            break;
        }

        static const std::string Templ = 
            "#^#GENERATED#$#\n"
            "#^#REPLACE_COMMENT#$#\n"
            "#^#SECTION#$#"
            "\\label{#^#LABEL#$#}\n\n"
            "TODO\n"
            "#^#APPEND#$#\n";

        auto appendFileName = latexRelFilePath() + strings::genAppendFileSuffixStr();
        util::GenReplacementMap repl = {
            {"GENERATED", LatexGenerator::latexFileGeneratedComment()},
            {"SECTION", latexSection()},
            {"LABEL", LatexGenerator::latexLabelId(m_genField)},
            {"APPEND", util::genReadFileContents(latexGenerator.latexInputCodePathForFile(appendFileName))},
        };

        if (latexGenerator.latexHasCodeInjectionComments()) {
            repl["REPLACE_COMMENT"] = 
                latexGenerator.latexCodeInjectCommentPrefix() + "Replace the whole file with \"" + replaceFileName + "\".";

            if (repl["APPEND"].empty()) {
                repl["APPEND"] = latexGenerator.latexCodeInjectCommentPrefix() + "Append to file with \"" + appendFileName + "\".";
            }                
        };         

        stream << util::genProcessTemplate(Templ, repl, true) << std::endl;
    } while (false);

    return true;    
}

std::string LatexField::latexSection() const
{
    static const std::string Templ = 
        "#^#REPLACE_COMMENT#$#\n"
        "#^#SECTION#$#{#^#TITLE#$#}\n"
        ;

    auto& latexGenerator = LatexGenerator::latexCast(m_genField.genGenerator());
    auto titleFileName = latexRelFilePath() + strings::genTitleFileSuffixStr();
    util::GenReplacementMap repl = {
        {"SECTION", LatexGenerator::latexSectionDirective(m_genField)},
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

} // namespace commsdsl2latex
