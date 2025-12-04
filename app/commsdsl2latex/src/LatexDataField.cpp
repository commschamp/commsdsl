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

#include "LatexDataField.h"

#include "LatexGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2latex
{

namespace
{

std::string latexDataStr(const std::vector<std::uint8_t>& data)
{
    std::stringstream stream;
    stream << "0x" << std::hex;

    for (auto b : data) {
        stream << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b);
    }
    return stream.str();
}

} // namespace

LatexDataField::LatexDataField(LatexGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    LatexBase(static_cast<GenBase&>(*this))
{
}

bool LatexDataField::genWriteImpl() const
{
    return latexWrite();
}

std::string LatexDataField::latexInfoDetailsImpl() const
{
    util::GenStringsList list;
    auto parseObj = genDataFieldParseObj();
    do {
        auto& validValues = parseObj.parseValidValues();
        if (validValues.empty()) {
            break;
        }

        util::GenStringsList values;
        for (auto& v : validValues) {
            values.push_back(latexDataStr(v.m_value));
        }

        list.push_back("\\textbf{Valid Values} & " + util::genStrListToString(values, ", ", ""));
    } while (false);

    do {
        auto fixedLength = parseObj.parseFixedLength();
        if (fixedLength != 0U) {
            list.push_back("\\textbf{Fixed Length} & " + LatexGenerator::latexIntegralToStr(fixedLength) + " Bytes");
        }
    } while (false);

    do {
        auto* prefixField = genExternalPrefixField();
        if (prefixField == nullptr) {
            prefixField = genMemberPrefixField();
        }

        if (prefixField == nullptr) {
            break;
        }

        list.push_back("\\textbf{Length Prefix} & \\nameref{" + LatexField::latexCast(prefixField)->latexRefLabelId() + "}");
    } while (false);

    do {
        auto& detachedPrefixName = parseObj.parseDetachedPrefixFieldName();
        if (detachedPrefixName.empty()) {
            break;
        }

        auto* sibling = latexFindSibling(detachedPrefixName);
        if (sibling == nullptr) {
            break;
        }

        list.push_back("\\textbf{Detached Length Prefix} & \\nameref{" + sibling->latexRefLabelId() + "}");
    } while (false);

    if (list.empty()) {
        return strings::genEmptyString();
    }

    return util::genStrListToString(list, "\\\\\\hline\n", "\\\\");
}

std::string LatexDataField::latexExtraDetailsImpl() const
{
    auto* memField = genMemberPrefixField();
    if (memField == nullptr) {
        return strings::genEmptyString();
    }

    auto* latexField = LatexField::latexCast(memField);
    assert(latexField != nullptr);
    return latexField->latexDoc();
}

} // namespace commsdsl2latex
