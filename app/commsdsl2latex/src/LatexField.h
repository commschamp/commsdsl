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

    static LatexFieldsList latexTransformFieldsList(const GenFieldsList& fields);
    static std::string latexMembersDetails(const LatexFieldsList& latexFields);
    static LatexField* latexCast(GenField* genField);
    static const LatexField* latexCast(const GenField* genField);
    const LatexField* latexFindSibling(const std::string& name) const;

    std::string latexRelFilePath() const;
    std::string latexTitle() const;
    std::string latexDoc() const;
    std::string latexRefLabelId() const;
    std::string latexInfoDetails() const;
    std::string latexExtraDetails() const;
    std::string latexDescription() const;

    bool latexWrite() const;

    const GenField& latexGenField() const
    {
        return m_genField;
    }    

    const std::string& latexFieldKind() const
    {
        return latexFieldKindImpl();
    }

    bool latexIsOptional() const;

protected:
    virtual std::string latexDocImpl() const;
    virtual std::string latexDescriptionImpl() const;
    virtual std::string latexRefLabelIdImpl() const;
    virtual std::string latexInfoDetailsImpl() const;
    virtual std::string latexExtraDetailsImpl() const;
    virtual const std::string& latexFieldKindImpl() const;
    virtual bool latexIsOptionalImpl() const;

    static std::string latexSignedInfo(ParseIntField::ParseType value);
    static std::string latexEndianInfo(commsdsl::parse::ParseEndian value);
    static std::string latexUnitsInfo(commsdsl::parse::ParseUnits value);

private:
    std::string latexSection() const;

    const GenField& m_genField;
};

} // namespace commsdsl2latex
