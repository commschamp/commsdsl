//
// Copyright 2021 - 2025 (C). Alex Robenko. All rights reserved.
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

#include "LatexField.h"
#include "LatexFrame.h"
#include "LatexMessage.h"

#include "commsdsl/gen/GenNamespace.h"

namespace commsdsl2latex
{

class LatexGenerator;
class LatexNamespace final: public commsdsl::gen::GenNamespace
{
    using GenBase = commsdsl::gen::GenNamespace;

public:
    using ParseNamespace = commsdsl::parse::ParseNamespace;

    using GenElem = commsdsl::gen::GenElem;
    using GenNamespace = commsdsl::gen::GenNamespace;

    using LatexFieldsList = LatexField::LatexFieldsList;
    using LatexMessagesList = std::vector<const LatexMessage*>;
    using LatexFramesList = std::vector<const LatexFrame*>;

    LatexNamespace(LatexGenerator& generator, ParseNamespace parseObj, GenElem* parent);
    virtual ~LatexNamespace();

    static const LatexNamespace* latexCast(const GenNamespace* ptr)
    {
        return static_cast<const LatexNamespace*>(ptr);
    }

    static const LatexNamespace& latexCast(const GenNamespace& ref)
    {
        return static_cast<const LatexNamespace&>(ref);
    }

    std::string latexRelDirPath() const;
    std::string latexRelFilePath() const;
    std::string latexTitle() const;

protected:
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;

private:
    bool latexHasDocElements() const;
    std::string latexInputs() const;   
    std::string latexSection(const std::string& title) const;
    
    LatexFieldsList m_latexFields;
    LatexMessagesList m_latexMessages;
    LatexFramesList m_latexFrames;
};

} // namespace commsdsl2latex
