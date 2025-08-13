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

#include "commsdsl/gen/GenMessage.h"

namespace commsdsl2latex
{

class LatexGenerator;
class LatexMessage final: public commsdsl::gen::GenMessage
{
    using GenBase = commsdsl::gen::GenMessage;

public:
    using ParseMessage = commsdsl::parse::ParseMessage;

    using GenElem = commsdsl::gen::GenElem;
    using GenMessage = commsdsl::gen::GenMessage;

    using LatexFieldsList = LatexField::LatexFieldsList;

    LatexMessage(LatexGenerator& generator, ParseMessage parseObj, GenElem* parent);
    virtual ~LatexMessage();

    static const LatexMessage* latexCast(const GenMessage* ptr)
    {
        return static_cast<const LatexMessage*>(ptr);
    }

    static const LatexMessage& latexCast(const GenMessage& ref)
    {
        return static_cast<const LatexMessage&>(ref);
    }

    std::string latexRelFilePath() const;
    std::string latexTitle() const;

protected:
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;

private:
    std::string latexSection() const;
    std::string latexInfoDetails() const;
    std::string latexFields() const;
    
    LatexFieldsList m_latexFields;
};

} // namespace commsdsl2latex
