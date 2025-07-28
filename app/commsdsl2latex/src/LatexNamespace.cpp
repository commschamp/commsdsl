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

#include <cassert>
#include <fstream>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2latex
{

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
    return latexRelDirPath() + "/namespace" + strings::genLatexSuffixStr();
}

std::string LatexNamespace::latexTitle() const
{
    if (!genParseObj().parseValid()) {
        return strings::genEmptyString();
    }

    auto& displayName = genParseObj().parseDisplayName();
    if (!displayName.empty()) {
        return displayName;
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

    return "Namespace \"" + *name + "\"";
}

bool LatexNamespace::genWriteImpl() const
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

    static const std::string Templ = 
        "#^#SECTION#$\n"
        "#^#DESCRIPTION#$#\n"
        "TODO: NS INPUTS\n";

    util::GenReplacementMap repl;

    if (genParseObj().parseValid()) {
        repl["DESCRIPTION"] = util::genStrMakeMultiline(genParseObj().parseDescription());
    }

    auto title = latexTitle();
    if (!title.empty()) {
        repl["SECTION"] = LatexGenerator::latexSectionDirective(*this) + '{' + title + '}';
    }    

    stream << util::genProcessTemplate(Templ, repl) << std::endl;
    
    stream.flush();
    if (!stream.good()) {
        latexGenerator.genLogger().genError("Failed to write \"" + filePath + "\".");
        return false;
    }

    return true;
}

} // namespace commsdsl2latex
