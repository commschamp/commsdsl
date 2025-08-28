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

#include "LatexRefField.h"

#include "LatexGenerator.h"

#include "commsdsl/gen/strings.h"
#include "commsdsl/gen/util.h"

namespace strings = commsdsl::gen::strings;
namespace util = commsdsl::gen::util;

#include <cassert>

namespace commsdsl2latex
{

LatexRefField::LatexRefField(LatexGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    LatexBase(static_cast<GenBase&>(*this)) 
{
}   

bool LatexRefField::genPrepareImpl()
{
    if (!GenBase::genPrepareImpl()) {
        return false;
    }

    m_latexField = dynamic_cast<LatexField*>(genReferencedField());
    assert(m_latexField != nullptr);

    return true;
}

bool LatexRefField::genWriteImpl() const
{
    return latexWrite();
}

std::string LatexRefField::latexDocImpl() const
{
    if (latexIsEmptyAlias()) {
        return "\n";
    }

    return LatexBase::latexDocImpl();
}

std::string LatexRefField::latexRefLabelIdImpl() const
{
    if (latexIsEmptyAlias()) {
        return m_latexField->latexRefLabelId();
    }

    return LatexBase::latexRefLabelIdImpl();
}

const std::string& LatexRefField::latexFieldKindImpl() const
{
    return m_latexField->latexFieldKind();
}

std::string LatexRefField::latexInfoDetailsImpl() const
{
    assert(m_latexField != nullptr);
    return ("\\textbf{Same As} & \\nameref{" + m_latexField->latexRefLabelId() + "}");
}

bool LatexRefField::latexIsOptionalImpl() const
{
    return m_latexField->latexIsOptional();
}

bool LatexRefField::latexIsEmptyAlias() const
{
    auto* parent = genGetParent();
    assert(parent != nullptr);

    do {
        auto type = parent->genElemType();
        if (type != commsdsl::gen::GenElem::GenType_Field) {
            break;
        }

        auto* parentField = static_cast<const commsdsl::gen::GenField*>(parent);
        auto parseKind = parentField->genParseObj().parseKind();
        if (parseKind != commsdsl::parse::ParseField::ParseKind::Bitfield) {
            break;
        }

        return false;
    } while (false);

    assert(m_latexField != nullptr);
    auto& displayName = genParseObj().parseDisplayName();
    auto& refDisplayName = m_latexField->latexGenField().genParseObj().parseDisplayName();

    if (!displayName.empty()) {
        return displayName == refDisplayName;
    }

    return genParseObj().parseName() == m_latexField->latexGenField().genParseObj().parseName();
}

} // namespace commsdsl2latex
