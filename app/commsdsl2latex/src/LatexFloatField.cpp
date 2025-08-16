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

#include "LatexFloatField.h"

#include "LatexGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2latex
{

namespace 
{

std::string latexFloatValToString(double val, const commsdsl::parse::ParseFloatField& parseObj)
{
    if (std::isnan(val)) {
        return "NaN";
    }

    if (std::isinf(val)) {
        if (val < 0) {
            return "-Inf";
        }

        return "Inf";
    }

    if (parseObj.parseType() == commsdsl::parse::ParseFloatField::ParseType::Float) {
        auto castVal = static_cast<float>(val);

        if (castVal == std::numeric_limits<float>::lowest()) {
            return ("(min)");
        }
        
        if (castVal == std::numeric_limits<float>::max()) {
            return ("(max)");
        }        
    }

    if (parseObj.parseType() == commsdsl::parse::ParseFloatField::ParseType::Double) {
        if (val == std::numeric_limits<double>::lowest()) {
            return ("(min)");
        }
        
        if (val == std::numeric_limits<double>::max()) {
            return ("(max)");
        }        
    }  
    
    auto displayDecimals = parseObj.parseDisplayDecimals();
    if (displayDecimals == 0) {
        return std::to_string(val);
    }

    std::stringstream stream;
    stream << std::fixed << std::setprecision(displayDecimals) << val;
    return stream.str();
}  

} // namespace 
    

LatexFloatField::LatexFloatField(LatexGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    LatexBase(static_cast<GenBase&>(*this)) 
{
}   

bool LatexFloatField::genWriteImpl() const
{
    return latexWrite();
}


std::string LatexFloatField::latexInfoDetailsImpl() const
{
    GenStringsList list;
    auto parseObj = genFloatFieldParseObj();
    
    list.push_back(latexEndianInfo(parseObj.parseEndian()));

    do {
        auto units = latexUnitsInfo(parseObj.parseUnits());
        if (units.empty()) {
            break;
        }

        list.push_back(std::move(units));
    } while (false);

    do {
        auto& validRanges = parseObj.parseValidRanges();
        util::GenStringsList values;
        for (auto& r : validRanges) {
            if (!genGenerator().genDoesElementExist(r.m_sinceVersion, r.m_deprecatedSince, true)) {
                continue;
            }
                
            if ((r.m_min == r.m_max) || (std::isnan(r.m_min))) {
                values.push_back(latexFloatValToString(r.m_min, parseObj));
                continue;
            }                

            values.push_back("[" + latexFloatValToString(r.m_min, parseObj) + " - " + latexFloatValToString(r.m_max, parseObj) + "]");
        }

        if (values.empty()) {
            break;
        }

        list.push_back("\\textbf{Valid Values} &" + util::genStrListToString(values, ", ", ""));
    } while (false);

    return util::genStrListToString(list, "\\\\\\hline\n", "\\\\");
}

std::string LatexFloatField::latexExtraDetailsImpl() const
{
    auto parseObj = genFloatFieldParseObj();
    auto& specials = parseObj.parseSpecialValues();    

    struct SpecialInfo
    {
        double m_value = 0;
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
        elem.m_name = LatexGenerator::latexEscDisplayName(s.second.m_displayName, s.first);
        elem.m_description = s.second.m_description;
    }

    if (infos.empty()) {
        return strings::genEmptyString();
    }

    std::sort(
        infos.begin(), infos.end(),
        [](auto& elem1, auto& elem2)
        {
            if (std::isnan(elem1.m_value) && std::isnan(elem2.m_value)) {
                return elem1.m_name < elem2.m_name;
            }

            if (std::isnan(elem1.m_value)) {
                return false;
            }

            if (std::isnan(elem2.m_value)) {
                return true;
            }

            if (elem1.m_value == elem2.m_value) {
                return elem1.m_name < elem2.m_name;
            }

            return elem1.m_value < elem2.m_value;
        });

    util::GenStringsList lines;
    for (auto& elem : infos) {
        auto l = latexFloatValToString(elem.m_value, parseObj) + " & " + elem.m_name + " & " + elem.m_description;
        lines.push_back(std::move(l));
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
        {"LABEL", LatexGenerator::latexLabelId(*this) + "_specials"},
        {"LINES", util::genStrListToString(lines, " \\\\\\hline\n", " \\\\")},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2latex
