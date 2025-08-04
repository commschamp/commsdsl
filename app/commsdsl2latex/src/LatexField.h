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


namespace commsdsl2latex
{

class LatexField
{
public:
    using GenField = commsdsl::gen::GenField;
    using GenFieldsList = GenField::GenFieldsList;

    using LatexFieldsList = std::vector<LatexField*>;

    explicit LatexField(const GenField& field);
    virtual ~LatexField();

    static LatexFieldsList latexTransformFieldsList(const GenFieldsList& fields);

    std::string latexRelFilePath() const;
    std::string latexTitle() const;

    bool latexWrite() const;

protected:
    const GenField& latexGenField() const
    {
        return m_genField;
    }    

private:
    std::string latexSection() const;

    const GenField& m_genField;
};

} // namespace commsdsl2latex
