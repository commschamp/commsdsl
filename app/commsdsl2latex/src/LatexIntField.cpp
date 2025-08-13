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

#include "LatexIntField.h"

#include "LatexGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <sstream>
#include <vector>

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2latex
{

LatexIntField::LatexIntField(LatexGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    LatexBase(static_cast<GenBase&>(*this)) 
{
}   

bool LatexIntField::genWriteImpl() const
{
    return latexWrite();
}

std::string LatexIntField::latexInfoDetailsImpl() const
{
    GenStringsList list;
    auto parseObj = genIntFieldParseObj();
    
    list.push_back(latexSignedInfo(parseObj.parseType()));
    list.push_back(latexEndianInfo(parseObj.parseEndian()));

    do {
        auto units = latexUnitsInfo(parseObj.parseUnits());
        if (units.empty()) {
            break;
        }

        list.push_back(std::move(units));
    } while (false);

    do {
        auto scaling = parseObj.parseScaling();
        if (scaling.first == scaling.second) {
            break;
        }

        list.push_back("\\textbf{Scaling} &" + std::to_string(scaling.first) + " / " + std::to_string(scaling.second));
    } while (false);

    do {
        auto& validRanges = parseObj.parseValidRanges();
        util::GenStringsList values;
        bool unsignedType = genIsUnsignedType();
        for (auto& r : validRanges) {
            if (!genGenerator().genDoesElementExist(r.m_sinceVersion, r.m_deprecatedSince, true)) {
                continue;
            }

            auto valToString = 
                [unsignedType](std::intmax_t val)
                {
                    if (unsignedType) {
                        return std::to_string(static_cast<std::uintmax_t>(val));
                    } 

                    return std::to_string(val);
                };

            if (r.m_min == r.m_max) {
                values.push_back(valToString(r.m_min));
                continue;
            }

            values.push_back("[" + valToString(r.m_min) + " - " + valToString(r.m_max) + "]");
        }

        if (values.empty()) {
            break;
        }

        list.push_back("\\textbf{Valid Values} &" + util::genStrListToString(values, ", ", ""));
    } while (false);

    return util::genStrListToString(list, "\\\\\\hline\n", "\\\\\n");
}

std::string LatexIntField::latexExtraDetailsImpl() const
{
    auto parseObj = genIntFieldParseObj();
    auto& specials = parseObj.parseSpecialValues();    

    struct SpecialInfo
    {
        std::intmax_t m_value = 0;
        std::string m_name;
        std::string m_description;
    };

    std::vector<SpecialInfo> infos;
    infos.reserve(specials.size());

    for (auto& s : specials) {
        if (!genGenerator().genDoesElementExist(s.second.m_sinceVersion, s.second.m_deprecatedSince, true)) {
            continue;
        }

        infos.resize(infos.size() + 1U);
        auto& elem = infos.back();
        elem.m_value = s.second.m_value;
        elem.m_name = s.second.m_displayName;
        elem.m_description = s.second.m_description;
        if (elem.m_name.empty()) {
            elem.m_name = s.first;
        }
    }

    if (infos.empty()) {
        return strings::genEmptyString();
    }

    bool unsignedType = genIsUnsignedType();
    std::sort(
        infos.begin(), infos.end(),
        [unsignedType](auto& elem1, auto& elem2)
        {
            if (elem1.m_value == elem2.m_value) {
                return elem1.m_name < elem2.m_name;
            }

            if (unsignedType) {
                return static_cast<std::uintmax_t>(elem1.m_value) < static_cast<std::uintmax_t>(elem2.m_value);
            }

            return elem1.m_value < elem2.m_value;
        });

    util::GenStringsList lines;
    for (auto& elem : infos) {
        std::stringstream stream;
        if ((unsignedType) || (0 <= elem.m_value)) {
            auto castValue = static_cast<std::uintmax_t>(elem.m_value);
            stream << static_cast<std::uintmax_t>(elem.m_value) << " (0x" << std::hex << castValue << ")";
        }
        else {
            stream << elem.m_value;
        }

        stream << " & " << elem.m_name << " & " << elem.m_description;
        lines.push_back(stream.str());
    }

    static const std::string Templ = 
        "\\subsubparagraph{Special Values}\n"
        "\\label{#^#LABEL#$#}\n\n"
        "\\fbox{%\n"
        "\\begin{tabular}{l|l|p{7cm}}\n"
        "\\textbf{Value} & \\textbf{Name}& \\textbf{Description}\\\\\n"
        "\\hline\n"
        "\\hline\n"        
        "#^#LINES#$#\n"
        "\\end{tabular}\n"
        "}\n"
        ;

    util::GenReplacementMap repl = {
        {"LABEL", LatexGenerator::latexLabelId(*this) + "_sepcials"},
        {"LINES", util::genStrListToString(lines, " \\\\\\hline\n", " \\\\")},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2latex
