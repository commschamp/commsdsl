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
    return genOrigNamespace();
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
        return displayName;
    }

    auto& schemas = latexGenerator.genSchemas();
    if (schemas.size() == 1U) {
        return std::string();
    }

    auto& name = genParseObj().parseName();
    if (&(latexGenerator.genProtocolSchema()) == this) {
        return "Protocol \"" + name + "\"";
    }

    return "Schema \"" + name + "\"";
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

    static const std::string Templ = 
        "#^#SECTION#$#\n"
        "#^#DESCRIPTION#$#\n"
        "#^#INPUTS#$#\n"
        ;

    util::GenReplacementMap repl = {
        {"DESCRIPTION", util::genStrMakeMultiline(genParseObj().parseDescription())},
        {"INPUTS", latexNamespaceInputs()},
    };

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
