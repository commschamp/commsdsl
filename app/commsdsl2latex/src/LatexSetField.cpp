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

#include "LatexSetField.h"

#include "LatexGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace util = commsdsl::gen::util;
namespace strings = commsdsl::gen::strings;

namespace commsdsl2latex
{

LatexSetField::LatexSetField(LatexGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    LatexBase(static_cast<GenBase&>(*this))
{
}

bool LatexSetField::genWriteImpl() const
{
    return latexWrite();
}

std::string LatexSetField::latexInfoDetailsImpl() const
{
    GenStringsList list;
    auto parseObj = genSetFieldParseObj();

    list.push_back(latexEndianInfo(parseObj.parseEndian()));
    return util::genStrListToString(list, "\\\\\\hline\n", "\\\\");
}

std::string LatexSetField::latexExtraDetailsImpl() const
{
    GenStringsList lines;
    auto parseObj = genSetFieldParseObj();
    auto& revBits = parseObj.parseRevBits();
    auto hexWidth = parseObj.parseMinLength() * 2U;

    auto maskStr =
        [hexWidth](std::uintmax_t val)
        {
            std::stringstream stream;
            stream << "0x" << std::hex << std::setfill('0') << std::setw(static_cast<int>(hexWidth)) << val;
            return stream.str();
        };

    auto idxToMaskStr =
        [maskStr](unsigned idx)
        {
            return maskStr(1ULL << idx);
        };

    unsigned gapIdx = 0;
    for (auto& b : revBits) {
        while (gapIdx < b.first) {
            auto l = std::to_string(gapIdx) + " & " + idxToMaskStr(gapIdx) + " & & Reserved. Expected to be " + std::to_string(parseObj.parseReservedBitValue()) + ".";
            lines.push_back(std::move(l));
            ++gapIdx;
            continue;
        }

        gapIdx = b.first + 1U;
        auto& bits = parseObj.parseBits();
        auto iter = bits.find(b.second);
        assert(iter != bits.end());

        auto& info = iter->second;
        auto l = std::to_string(b.first) + " & " + idxToMaskStr(b.first) + " & " +
            LatexGenerator::latexEscDisplayName(info.m_displayName, b.second) + " & " +
            info.m_description;

        auto addVersionInfo =
            [&info, &l]()
            {
                if (info.m_sinceVersion != 0) {
                    l.append("Introduced in version " + LatexGenerator::latexIntegralToStr(info.m_sinceVersion) + ". ");
                }

                if (info.m_deprecatedSince != commsdsl::parse::ParseProtocol::parseNotYetDeprecated()) {
                    l.append("Deprecated in version " + LatexGenerator::latexIntegralToStr(info.m_deprecatedSince) + ". ");
                }
            };

        if (!info.m_description.empty()) {
            if (info.m_description.back() != '.') {
                l.push_back('.');
            }

            l.push_back(' ');
        }

        if (!info.m_reserved) {
            addVersionInfo();
            lines.push_back(std::move(l));
            continue;
        }

        l.append("Expected to be " + std::to_string(info.m_reservedValue) + ". ");
        addVersionInfo();
        lines.push_back(std::move(l));
    }

    auto bitLimit = parseObj.parseMinLength() * 8U;
    if (parseObj.parseBitLength() != 0) {
        bitLimit = std::min(bitLimit, parseObj.parseBitLength());
    }

    if (gapIdx < bitLimit) {
        std::uintmax_t mask = 0U;
        for (auto i = gapIdx ; i < bitLimit; ++i) {
            mask |= (1ULL << i);
        }

        auto l = std::to_string(gapIdx);
        if (gapIdx < (bitLimit - 1)) {
            l += " - " + std::to_string(bitLimit - 1);
        }

        l += " & " + maskStr(mask) + " & & Reserved. Expected to be " + std::to_string(parseObj.parseReservedBitValue()) + ".";
        lines.push_back(std::move(l));
    }

    static const std::string Templ =
        "\\subsubparagraph{Bit Values}\n"
        "\\label{#^#LABEL#$#}\n\n"
        "\\fbox{%\n"
        "\\begin{tabular}{c|c|c|p{7cm}}\n"
        "\\textbf{Bit(s) Index} & \\textbf{Bit(s) Mask} & \\textbf{Name} & \\textbf{Description}\\\\\n"
        "\\hline\n"
        "\\hline\n"
        "#^#LINES#$#\n"
        "\\end{tabular}\n"
        "}\n"
        "\\smallskip\n"
        "\n"
        ;

    util::GenReplacementMap repl = {
        {"LABEL", LatexGenerator::latexLabelId(*this) + "_bits"},
        {"LINES", util::genStrListToString(lines, " \\\\\\hline\n", " \\\\")},
    };

    return util::genProcessTemplate(Templ, repl);
}

} // namespace commsdsl2latex
