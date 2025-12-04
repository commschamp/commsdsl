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

#include "LatexListField.h"

#include "LatexGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

#include <cassert>

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

namespace commsdsl2latex
{

LatexListField::LatexListField(LatexGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    LatexBase(static_cast<GenBase&>(*this))
{
}

bool LatexListField::genWriteImpl() const
{
    return latexWrite();
}

std::string LatexListField::latexInfoDetailsImpl() const
{
    util::GenStringsList list;
    auto parseObj = genListFieldParseObj();

    do {
        auto fixedCount = parseObj.parseFixedCount();
        if (fixedCount != 0U) {
            list.push_back("\\textbf{Fixed Elements Count} & " + LatexGenerator::latexIntegralToStr(fixedCount));
        }
    } while (false);

    using GenField = commsdsl::gen::GenField;
    struct MemFieldInfo
    {
        const GenField* m_extField = nullptr;
        const GenField* m_memField = nullptr;
        std::string m_doc;
    };

    const MemFieldInfo memFields[] = {
        MemFieldInfo{genExternalElementField(), genMemberElementField(), "Element Field"},
        MemFieldInfo{genExternalCountPrefixField(), genMemberCountPrefixField(), "Count Prefix"},
        MemFieldInfo{genExternalLengthPrefixField(), genMemberLengthPrefixField(), "Length Prefix"},
        MemFieldInfo{genExternalElemLengthPrefixField(), genMemberElemLengthPrefixField(), "Element Length Prefix"},
        MemFieldInfo{genExternalTermSuffixField(), genMemberTermSuffixField(), "Terminating Suffix"},
    };

    for (auto& i : memFields) {
        auto* f = i.m_extField;
        if (f == nullptr) {
            f = i.m_memField;
        }

        if (f == nullptr) {
            continue;
        }

        list.push_back("\\textbf{" + i.m_doc + "} & \\nameref{" + LatexField::latexCast(f)->latexRefLabelId() + "}");
    }

    struct DetachedPrefixInfo
    {
        std::string m_name;
        std::string m_doc;
    };

    const DetachedPrefixInfo detachedInfos[] = {
        DetachedPrefixInfo{parseObj.parseDetachedCountPrefixFieldName(), "Detached Count Prefix"},
        DetachedPrefixInfo{parseObj.parseDetachedLengthPrefixFieldName(), "Detached Length Prefix"},
        DetachedPrefixInfo{parseObj.parseDetachedElemLengthPrefixFieldName(), "Detached Element Length Prefix"},
        DetachedPrefixInfo{parseObj.parseDetachedTermSuffixFieldName(), "Detached Termination Suffix"},
    };

    for (auto& i : detachedInfos) {
        if (i.m_name.empty()) {
            continue;
        }

        auto* sibling = latexFindSibling(i.m_name);
        if (sibling == nullptr) {
            break;
        }

        list.push_back("\\textbf{" + i.m_doc + "} & \\nameref{" + sibling->latexRefLabelId() + "}");
    }

    if (list.empty()) {
        return strings::genEmptyString();
    }

    return util::genStrListToString(list, "\\\\\\hline\n", "\\\\");
}

std::string LatexListField::latexExtraDetailsImpl() const
{
    using GenField = commsdsl::gen::GenField;
    const GenField* memFields[] = {
        genMemberElementField(),
        genMemberCountPrefixField(),
        genMemberLengthPrefixField(),
        genMemberElemLengthPrefixField(),
        genMemberTermSuffixField(),
    };

    util::GenStringsList elems;
    for (auto* f : memFields) {
        if (f == nullptr) {
            continue;
        }

        elems.push_back(LatexField::latexCast(f)->latexDoc());
    }

    if (elems.empty()) {
        return strings::genEmptyString();
    }

    return util::genStrListToString(elems, "\n", "\n");
}

} // namespace commsdsl2latex
