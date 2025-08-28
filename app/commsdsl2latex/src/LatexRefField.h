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

#include "LatexField.h"

#include "commsdsl/gen/GenRefField.h"

namespace commsdsl2latex
{

class LatexGenerator;
class LatexRefField final : public commsdsl::gen::GenRefField, public LatexField
{
    using GenBase = commsdsl::gen::GenRefField;
    using LatexBase = LatexField;
public:
    using ParseField = commsdsl::parse::ParseField;
    using GenElem = commsdsl::gen::GenElem;

    LatexRefField(LatexGenerator& generator, ParseField parseObj, GenElem* parent);

protected:
    virtual bool genPrepareImpl() override;
    virtual bool genWriteImpl() const override;    

    virtual std::string latexDocImpl() const override;
    virtual std::string latexRefLabelIdImpl() const override;
    virtual const std::string& latexFieldKindImpl() const override;
    virtual std::string latexInfoDetailsImpl() const override;
    virtual bool latexIsOptionalImpl() const override;


private:
    bool latexIsEmptyAlias() const;
    
    LatexField* m_latexField = nullptr;
};

} // namespace commsdsl2latex
