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

#include "LatexEnumField.h"

#include "LatexGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cassert>
#include <vector>

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2latex
{

LatexEnumField::LatexEnumField(LatexGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    LatexBase(static_cast<GenBase&>(*this))
{
}

bool LatexEnumField::genWriteImpl() const
{
    return latexWrite();
}

std::string LatexEnumField::latexInfoDetailsImpl() const
{
    GenStringsList list;
    auto parseObj = genEnumFieldParseObj();

    list.push_back(latexSignedInfo(parseObj.parseType()));
    list.push_back(latexEndianInfo(parseObj.parseEndian()));

    return util::genStrListToString(list, "\\\\\\hline\n", "\\\\");
}

std::string LatexEnumField::latexExtraDetailsImpl() const
{
    util::GenStringsList lines;
    auto parseObj = genEnumFieldParseObj();
    auto& values = parseObj.parseValues();
    auto& revValues = parseObj.parseRevValues();
    bool unsignedType = genIsUnsignedType();
    auto hexWidth = parseObj.parseMinLength() * 2U;
    for (auto& v : revValues) {
        auto iter = values.find(v.second);
        assert(iter != values.end());
        if (iter == values.end()) {
            continue;
        }

        if (!genGenerator().genDoesElementExist(iter->second.m_sinceVersion, iter->second.m_deprecatedSince, true)) {
            continue;
        }

        auto l =
            LatexGenerator::latexIntegralToStr(iter->second.m_value, unsignedType, hexWidth) + " & " +
            LatexGenerator::latexEscDisplayName(iter->second.m_displayName, iter->first) + " & " + iter->second.m_description;
        lines.push_back(std::move(l));
    }

    if (lines.empty()) {
        return strings::genEmptyString();
    }

    static const std::string Templ =
        "\\subsubparagraph{Valid Values}\n"
        "\\label{#^#LABEL#$#}\n\n"
        "\\fbox{%\n"
        "\\begin{tabular}{l|l|p{7cm}}\n"
        "\\textbf{Value} & \\textbf{Name}& \\textbf{Description}\\\\\n"
        "\\hline\n"
        "\\hline\n"
        "#^#LINES#$#\n"
        "\\end{tabular}\n"
        "}\n"
        "\\smallskip\n"
        "\n"
        ;

    util::GenReplacementMap repl = {
        {"LABEL", LatexGenerator::latexLabelId(*this) + "_values"},
        {"LINES", util::genStrListToString(lines, " \\\\\\hline\n", " \\\\")},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2latex
