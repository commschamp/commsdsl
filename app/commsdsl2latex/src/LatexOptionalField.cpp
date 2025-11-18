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

#include "LatexOptionalField.h"

#include "LatexGenerator.h"

#include "commsdsl/gen/strings.h"

namespace strings = commsdsl::gen::strings;

#include <cassert>

namespace commsdsl2latex
{

LatexOptionalField::LatexOptionalField(LatexGenerator& generator, ParseField parseObj, GenElem* parent) :
    GenBase(generator, parseObj, parent),
    LatexBase(static_cast<GenBase&>(*this))
{
}

bool LatexOptionalField::latexIsPassThroughToMember() const
{
    auto* memField = genMemberField();
    if (memField == nullptr) {
        return false;
    }

    auto& desc = memField->genParseObj().parseDescription();
    if (!desc.empty()) {
        return false;
    }

    return true;
}

bool LatexOptionalField::genWriteImpl() const
{
    return latexWrite();
}

std::string LatexOptionalField::latexDocImpl() const
{
    if (latexIsPassThroughToMember()) {
        return LatexField::latexCast(genMemberField())->latexDoc();
    }

    return LatexBase::latexDocImpl();
}

std::string LatexOptionalField::latexDescriptionImpl() const
{
    if (latexIsPassThroughToMember()) {
        return LatexGenerator::latexEscString(genMemberField()->genParseObj().parseDescription());
    }

    return LatexBase::latexDescriptionImpl();
}

std::string LatexOptionalField::latexRefLabelIdImpl() const
{
    if (latexIsPassThroughToMember()) {
        return LatexField::latexCast(genMemberField())->latexRefLabelId();
    }

    return LatexBase::latexRefLabelIdImpl();
}

std::string LatexOptionalField::latexInfoDetailsImpl() const
{
    if (latexIsPassThroughToMember()) {
        assert(false); // Should not be called
        return strings::genEmptyString();
    }

    auto* actField = genMemberField();
    if (actField == nullptr) {
        actField = genExternalField();
    }

    assert(actField != nullptr);
    return ("\\textbf{Same As} & \\nameref{" + LatexField::latexCast(actField)->latexRefLabelId() + "}");
}

std::string LatexOptionalField::latexExtraDetailsImpl() const
{
    if (latexIsPassThroughToMember()) {
        assert(false); // Should not be called
        return strings::genEmptyString();
    }

    auto* memField = genMemberField();
    if (memField != nullptr) {
        return LatexField::latexCast(memField)->latexDoc();
    }

    return strings::genEmptyString();
}

const std::string& LatexOptionalField::latexFieldKindImpl() const
{
    auto* actField = genMemberField();
    if (actField == nullptr) {
        actField = genExternalField();
    }

    assert(actField != nullptr);
    return LatexField::latexCast(actField)->latexFieldKind();
}

bool LatexOptionalField::latexIsOptionalImpl() const
{
    return true;
}

} // namespace commsdsl2latex
