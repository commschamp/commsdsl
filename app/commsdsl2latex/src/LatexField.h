//
// Copyright 2025 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
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

#pragma once

#include "commsdsl/gen/GenField.h"
#include "commsdsl/gen/util.h"

#include "commsdsl/parse/ParseIntField.h"

namespace commsdsl2latex
{

class LatexField
{
public:
    using ParseIntField = commsdsl::parse::ParseIntField;
    using GenStringsList = commsdsl::gen::util::GenStringsList;
    using GenElem = commsdsl::gen::GenElem;
    using GenField = commsdsl::gen::GenField;
    using GenFieldsList = GenField::GenFieldsList;

    using LatexFieldsList = std::vector<LatexField*>;

    explicit LatexField(const GenField& field);
    virtual ~LatexField();

    [[nodiscard]] static LatexFieldsList latexTransformFieldsList(const GenFieldsList& fields);
    [[nodiscard]] static std::string latexMembersDetails(const LatexFieldsList& latexFields);
    [[nodiscard]] static LatexField* latexCast(GenField* genField);
    [[nodiscard]] static const LatexField* latexCast(const GenField* genField);
    [[nodiscard]] const LatexField* latexFindSibling(const std::string& name) const;

    [[nodiscard]] std::string latexRelFilePath() const;
    [[nodiscard]] std::string latexTitle() const;
    [[nodiscard]] std::string latexDoc() const;
    [[nodiscard]] std::string latexRefLabelId() const;
    [[nodiscard]] std::string latexInfoDetails() const;
    [[nodiscard]] std::string latexExtraDetails() const;
    [[nodiscard]] std::string latexDescription() const;

    [[nodiscard]] bool latexWrite() const;

    [[nodiscard]] const GenField& latexGenField() const
    {
        return m_genField;
    }

    [[nodiscard]] const std::string& latexFieldKind() const
    {
        return latexFieldKindImpl();
    }

    [[nodiscard]] bool latexIsOptional() const;

protected:
    [[nodiscard]] virtual std::string latexDocImpl() const;
    [[nodiscard]] virtual std::string latexDescriptionImpl() const;
    [[nodiscard]] virtual std::string latexRefLabelIdImpl() const;
    [[nodiscard]] virtual std::string latexInfoDetailsImpl() const;
    [[nodiscard]] virtual std::string latexExtraDetailsImpl() const;
    [[nodiscard]] virtual const std::string& latexFieldKindImpl() const;
    [[nodiscard]] virtual bool latexIsOptionalImpl() const;

    [[nodiscard]] static std::string latexSignedInfo(ParseIntField::ParseType value);
    [[nodiscard]] static std::string latexEndianInfo(commsdsl::parse::ParseEndian value);
    [[nodiscard]] static std::string latexUnitsInfo(commsdsl::parse::ParseUnits value);

private:
    [[nodiscard]] std::string latexSection() const;

    const GenField& m_genField;
};

} // namespace commsdsl2latex
