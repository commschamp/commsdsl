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

#include "LatexNamespace.h"

#include "LatexGenerator.h"
#include "LatexSchema.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <fstream>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2latex
{

namespace 
{

LatexNamespace::LatexMessagesList latexTransformMessagesList(const commsdsl::gen::GenNamespace::GenMessagesList& list)
{
    LatexNamespace::LatexMessagesList result;
    result.reserve(list.size());
    for (auto& mPtr : list) {
        assert(mPtr);

        auto* latexMessage = dynamic_cast<const LatexMessage*>(mPtr.get());

        assert(latexMessage != nullptr);
        result.push_back(latexMessage);
    }

    return result;
}

LatexNamespace::LatexFramesList latexTransformMessagesList(const commsdsl::gen::GenNamespace::GenFramesList& list)
{
    LatexNamespace::LatexFramesList result;
    result.reserve(list.size());
    for (auto& mPtr : list) {
        assert(mPtr);

        auto* latexMessage = dynamic_cast<const LatexFrame*>(mPtr.get());
        assert(latexMessage != nullptr);
        result.push_back(latexMessage);
    }

    return result;
}

} // namespace 
    

LatexNamespace::LatexNamespace(LatexGenerator& generator, ParseNamespace parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent)
{
}

LatexNamespace::~LatexNamespace() = default;

std::string LatexNamespace::latexRelDirPath() const
{
    auto* parent = genGetParent();
    assert(parent != nullptr);
    std::string parentDir;
    do {
        if (parent->genElemType() == Type_Schema) {
            auto& schema = LatexSchema::latexCast(commsdsl::gen::GenSchema::genCast(*parent));
            parentDir = schema.latexRelDirPath();
            break;
        }

        assert(parent->genElemType() == Type_Namespace);
        auto& parentNs = LatexNamespace::latexCast(commsdsl::gen::GenNamespace::genCast(*parent));
        parentDir = parentNs.latexRelDirPath();        
    } while (false);
    

    auto& thisName = genName();
    if (thisName.empty()) {
        return parentDir;
    }

    return parentDir + '/' + thisName;
}

std::string LatexNamespace::latexRelFilePath() const
{
    if (!latexHasDocElements()) {
        return strings::genEmptyString();
    }

    return latexRelDirPath() + "/namespace" + strings::genLatexSuffixStr();
}

std::string LatexNamespace::latexTitle() const
{
    if (!genParseObj().parseValid()) {
        return strings::genEmptyString();
    }

    auto& displayName = genParseObj().parseDisplayName();
    if (!displayName.empty()) {
        return LatexGenerator::latexEscDisplayName(displayName, std::string());
    }

    do {
        auto* parent = genGetParent();
        assert(parent != nullptr);
        if (parent->genElemType() != Type_Schema) {
            break;
        }

        auto& schema = LatexSchema::latexCast(commsdsl::gen::GenSchema::genCast(*parent));
        auto& schemaNamespaces = schema.genNamespaces();
        if (schemaNamespaces.size() != 1U) {
            break;
        }

        return std::string();
    } while (false);

    auto* name = &(genParseObj().parseName());
    if (name->empty()) {
        auto* parent = genGetParent();
        assert(parent != nullptr);
        assert(parent->genElemType() == Type_Schema);
        auto& schema = LatexSchema::latexCast(commsdsl::gen::GenSchema::genCast(*parent));
        name = &(schema.genOrigNamespace());
    }

    return "Namespace \"" + LatexGenerator::latexEscDisplayName(*name, std::string()) + "\"";
}

bool LatexNamespace::genPrepareImpl()
{
    m_latexFields = LatexField::latexTransformFieldsList(genFields());
    m_latexMessages = latexTransformMessagesList(genMessages());
    m_latexFrames = latexTransformMessagesList(genFrames());
    return true;
}

bool LatexNamespace::genWriteImpl() const
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
            "#^#SECTION#$#\n"
            "#^#LABEL#$#\n"
            "\n"
            "#^#DESCRIPTION#$#\n"
            "\n"
            "#^#PREPEND#$#\n"
            "#^#INPUTS#$#\n"
            "#^#APPEND#$#\n";

        auto prependFileName = latexRelFilePath() + strings::genPrependFileSuffixStr();
        auto appendFileName = latexRelFilePath() + strings::genAppendFileSuffixStr();
        util::GenReplacementMap repl = {
            {"GENERATED", LatexGenerator::latexFileGeneratedComment()},
            {"INPUTS", latexInputs()},
            {"PREPEND", util::genReadFileContents(latexGenerator.latexInputCodePathForFile(prependFileName))},
            {"APPEND", util::genReadFileContents(latexGenerator.latexInputCodePathForFile(appendFileName))},
        };

        auto title = latexTitle();
        if (!title.empty()) {
            repl["SECTION"] = latexSection(title);
            repl["LABEL"] = "\\label{" + LatexGenerator::latexLabelId(*this) + '}';

            if (genParseObj().parseValid()) {
                repl["DESCRIPTION"] = util::genStrMakeMultiline(genParseObj().parseDescription());
            }
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

bool LatexNamespace::latexHasDocElements() const
{
    auto hasInput = 
        [](auto& list)
        {
            return 
                std::any_of(
                    list.begin(), list.end(),
                    [](auto& ptr)
                    {
                        return !ptr->latexRelFilePath().empty();
                    });
        };

    return
        hasInput(m_latexFields) ||
        hasInput(m_latexMessages) ||
        hasInput(m_latexFrames);
}

std::string LatexNamespace::latexInputs() const
{
    util::GenStringsList inputs;
    auto addInputFor = 
        [&inputs](auto& list)
        {
            for (const auto* elem : list) {
                auto path = elem->latexRelFilePath();
                if (!path.empty()) {
                    inputs.push_back("\\input{" + path + "}");
                }
            }
        };
        
    addInputFor(m_latexFields);
    addInputFor(m_latexMessages);
    addInputFor(m_latexFrames);

    if (inputs.empty()) {
        return strings::genEmptyString();
    }

    return util::genStrListToString(inputs, "\n", "\n");
}

std::string LatexNamespace::latexSection(const std::string& title) const
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
        repl["TITLE"] = title;
    }

    if (latexGenerator.latexHasCodeInjectionComments()) {
        repl["REPLACE_COMMENT"] = 
            latexGenerator.latexCodeInjectCommentPrefix() + "Replace the title value with contents of \"" + titleFileName + "\".";
    };      

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2latex
