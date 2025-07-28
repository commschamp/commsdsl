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

#include "commsdsl/gen/GenGenerator.h"

namespace commsdsl2latex 
{

class LatexGenerator final : public commsdsl::gen::GenGenerator
{
    using Base = commsdsl::gen::GenGenerator;

public:
    using ParseSchema = commsdsl::parse::ParseSchema;
    using ParseNamespace = commsdsl::parse::ParseNamespace;

    using GenElem = commsdsl::gen::GenElem;
    using GenSchemaPtr = commsdsl::gen::GenSchemaPtr;
    using GenNamespacePtr = commsdsl::gen::GenNamespacePtr;
    using GenProgramOptions = commsdsl::gen::GenProgramOptions;
    using GenGenerator = commsdsl::gen::GenGenerator;

    static const std::string& latexFileGeneratedComment();
    static const std::string& latexCodeInjectCommentPrefix();
    static std::string latexWrapInput(const std::string& filePath);
    static void latexWrapInputInPlace(std::string& filePath);
    static const std::string& latexSectionDirective(const GenElem& elem);

    static LatexGenerator& latexCast(GenGenerator& generator)
    {
        return static_cast<LatexGenerator&>(generator);
    }

    static const LatexGenerator& latexCast(const GenGenerator& generator)
    {
        return static_cast<const LatexGenerator&>(generator);
    }    

    std::string latexRelPathFor(const GenElem& elem);

    std::string latexInputCodePathForFile(const std::string& name) const;    

    bool latexHasCodeInjectionComments() const
    {
        return m_hasCodeInjectionComments;
    }

protected:
    virtual bool genWriteImpl() override;    

    virtual GenSchemaPtr genCreateSchemaImpl(ParseSchema parseObj, GenElem* parent) override;
    virtual GenNamespacePtr genCreateNamespaceImpl(ParseNamespace parseObj, GenElem* parent) override;

    virtual OptsProcessResult genProcessOptionsImpl(const GenProgramOptions& options) override;

private:
    bool latexWriteExtraFilesInternal() const;

    bool m_hasCodeInjectionComments = false;
};

} // namespace commsdsl2latex
