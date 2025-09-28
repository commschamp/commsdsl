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

#include "LatexStringField.h"

#include "LatexGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2latex
{

LatexStringField::LatexStringField(LatexGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    LatexBase(static_cast<GenBase&>(*this))
{
}

bool LatexStringField::genWriteImpl() const
{
    return latexWrite();
}

std::string LatexStringField::latexInfoDetailsImpl() const
{
    util::GenStringsList list;
    auto parseObj = genStringFieldParseObj();
    do {
        auto& validValues = parseObj.parseValidValues();
        if (validValues.empty()) {
            break;
        }

        util::GenStringsList values;
        for (auto& v : validValues) {
            values.push_back("\"" + v.m_value + "\"");
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

    do {
        if (!parseObj.parseHasZeroTermSuffix()) {
            break;
        }
        list.push_back("\\textbf{Zero Termination Suffix} & YES");
    } while (false);

    if (list.empty()) {
        return strings::genEmptyString();
    }

    return util::genStrListToString(list, "\\\\\\hline\n", "\\\\");
}

std::string LatexStringField::latexExtraDetailsImpl() const
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
