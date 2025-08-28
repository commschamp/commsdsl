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

#include "LatexBitfieldField.h"

#include "LatexGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2latex
{

LatexBitfieldField::LatexBitfieldField(LatexGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    LatexBase(static_cast<GenBase&>(*this)) 
{
}   

bool LatexBitfieldField::genPrepareImpl() 
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    m_latexFields = LatexField::latexTransformFieldsList(genMembers());
    return true;
}

bool LatexBitfieldField::genWriteImpl() const
{
    return latexWrite();
}

std::string LatexBitfieldField::latexInfoDetailsImpl() const
{
    GenStringsList list;
    auto parseObj = genBitfieldFieldParseObj();
    
    list.push_back(latexEndianInfo(parseObj.parseEndian()));
    return util::genStrListToString(list, "\\\\\\hline\n", "\\\\");
}

std::string LatexBitfieldField::latexExtraDetailsImpl() const
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
        {"LABEL", LatexGenerator::latexLabelId(*this) + "_members"},
        {"DETAILS", LatexField::latexMembersDetails(m_latexFields)},
    };

    return util::genProcessTemplate(Templ, repl);      
}

} // namespace commsdsl2latex
