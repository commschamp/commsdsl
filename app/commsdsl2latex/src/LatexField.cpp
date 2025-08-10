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

namespace 
{

const std::size_t InvalidLength = std::numeric_limits<std::size_t>::max();    

} // namespace 
    

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
    if (comms::genIsGlobalField(m_genField)) {
        return "Field \"" + name + "\"";
    }

    return "Member Field \"" + name + "\"";
}

std::string LatexField::latexDoc() const
{
    static const std::string Templ = 
            "#^#SECTION#$#"
            "\\label{#^#LABEL#$#}\n\n"
            "#^#DESCRIPTION#$#\n"
            "#^#PREPEND#$#\n"
            "#^#DETAILS#$#\n"
            "#^#APPEND#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"SECTION", latexSection()},
        {"LABEL", LatexGenerator::latexLabelId(m_genField)},
        {"DESCRIPTION", util::genStrMakeMultiline(m_genField.genParseObj().parseDescription())},
        {"DETAILS", latexDetails()},
    };

    LatexGenerator::latexEnsureNewLineBreak(repl["DESCRIPTION"]);    

    do {
        if (!comms::genIsGlobalField(m_genField)) {
            break;
        }

        auto& latexGenerator = LatexGenerator::latexCast(m_genField.genGenerator());
        auto relFilePath = latexRelFilePath();
        auto prependFileName = relFilePath + strings::genPrependFileSuffixStr();
        auto appendFileName = relFilePath + strings::genAppendFileSuffixStr();


        repl["PREPEND"] = util::genReadFileContents(latexGenerator.latexInputCodePathForFile(appendFileName));
        repl["APPEND"] = util::genReadFileContents(latexGenerator.latexInputCodePathForFile(prependFileName));

        if (!latexGenerator.latexHasCodeInjectionComments()) {
            break;
        };           

        if (repl["PREPEND"].empty()) {
            repl["PREPEND"] = latexGenerator.latexCodeInjectCommentPrefix() + "Prepend to details with \"" + prependFileName + "\".";
        } 
                        
        if (repl["APPEND"].empty()) {
            repl["APPEND"] = latexGenerator.latexCodeInjectCommentPrefix() + "Append to file with \"" + appendFileName + "\".";
        }                

    } while (false);

    
    return util::genProcessTemplate(Templ, repl);
}

std::string LatexField::latexRefLabelId() const
{
    return latexRefLabelIdImpl();
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

std::string LatexField::latexMembersDetails(const LatexFieldsList& latexFields)
{
    util::GenStringsList lines;
    util::GenStringsList fields;    
    std::size_t offset = 0;
    for (auto* f : latexFields) {
        auto& genField = f->latexGenField();
        auto parseObj = genField.genParseObj();
        auto details = f->latexDoc();
        std::string nameStr = LatexGenerator::latexEscDisplayName(parseObj.parseDisplayName(), parseObj.parseName());
        if (!details.empty()) {
            nameStr = "\\hyperref[" + f->latexRefLabelId() + "]{" + nameStr + "}";
            fields.push_back(details);
        }

        auto minLength = parseObj.parseMinLength();
        auto maxLength = parseObj.parseMaxLength();

        auto lengthStr = std::to_string(minLength);
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

        std::string offsetStr;
        do {
            if (offset == InvalidLength) {
                offsetStr = "<variable>";
                break;
            }

            offsetStr = std::to_string(offset);
            if (minLength == maxLength) {
                offset += minLength;
                break;
            }

            offset = InvalidLength;
        } while (false);

        lines.push_back(offsetStr + " & " + lengthStr + " & " + nameStr);
    }

    static const std::string Templ = 
        "\\fbox{%\n"
        "\\begin{tabular}{c|c|c}\n"
        "\\textbf{Offset (Bytes)} & \\textbf{Length (Bytes)}& \\textbf{Name}\\\\\n"
        "\\hline\n"
        "\\hline\n"
        "#^#LINES#$#\n"
        "\\end{tabular}\n"
        "}\n"
        "\\smallskip\n"
        "#^#DETAILS#$#\n"
        "\n"
        ;    

    util::GenReplacementMap repl = {
        {"LINES", util::genStrListToString(lines, " \\\\\\hline\n", " \\\\\n")},
        {"DETAILS", util::genStrListToString(fields, "\n", "\n")},
    };

    return util::genProcessTemplate(Templ, repl);      
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
            "#^#DOC#$#\n"
            ;
        util::GenReplacementMap repl = {
            {"GENERATED", LatexGenerator::latexFileGeneratedComment()},
            {"DOC", latexDoc()},
        };

        if (latexGenerator.latexHasCodeInjectionComments()) {
            repl["REPLACE_COMMENT"] = 
                latexGenerator.latexCodeInjectCommentPrefix() + "Replace the whole file with \"" + replaceFileName + "\".";
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

std::string LatexField::latexSection() const
{
    static const std::string Templ = 
        "#^#REPLACE_COMMENT#$#\n"
        "#^#SECTION#$#{#^#TITLE#$#}\n"
        ;

    auto& latexGenerator = LatexGenerator::latexCast(m_genField.genGenerator());
    util::GenReplacementMap repl = {
        {"SECTION", LatexGenerator::latexSectionDirective(m_genField)},
    };

    if (comms::genIsGlobalField(m_genField)) {
        auto titleFileName = latexRelFilePath() + strings::genTitleFileSuffixStr();
        repl["TITLE"] = util::genReadFileContents(latexGenerator.latexInputCodePathForFile(titleFileName));
        
        if (latexGenerator.latexHasCodeInjectionComments()) {
            repl["REPLACE_COMMENT"] = 
                latexGenerator.latexCodeInjectCommentPrefix() + "Replace the title value with contents of \"" + titleFileName + "\".";
        };      

    }

    if (repl["TITLE"].empty()) {
        repl["TITLE"] = latexTitle();
    }

    return util::genProcessTemplate(Templ, repl);
}

std::string LatexField::latexRefLabelIdImpl() const
{
    return LatexGenerator::latexLabelId(m_genField);
}

std::string LatexField::latexDetails() const
{
    return "TODO: Field details";
}

} // namespace commsdsl2latex
