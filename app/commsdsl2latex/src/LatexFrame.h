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

#include "commsdsl/gen/GenFrame.h"

namespace commsdsl2latex
{

class LatexGenerator;
class LatexFrame final: public commsdsl::gen::GenFrame
{
    using GenBase = commsdsl::gen::GenFrame;

public:
    using ParseFrame = commsdsl::parse::ParseFrame;

    using GenElem = commsdsl::gen::GenElem;
    using GenFrame = commsdsl::gen::GenFrame;

    using LatexFieldsList = LatexField::LatexFieldsList;

    LatexFrame(LatexGenerator& generator, ParseFrame parseObj, GenElem* parent);
    virtual ~LatexFrame();

    static const LatexFrame* latexCast(const GenFrame* ptr)
    {
        return static_cast<const LatexFrame*>(ptr);
    }

    static const LatexFrame& latexCast(const GenFrame& ref)
    {
        return static_cast<const LatexFrame&>(ref);
    }

    std::string latexRelFilePath() const;
    std::string latexTitle() const;

protected:
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;

private:
    std::string latexSection() const;
};

} // namespace commsdsl2latex
