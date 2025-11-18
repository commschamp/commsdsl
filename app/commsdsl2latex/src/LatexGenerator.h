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
    using GenElem = commsdsl::gen::GenElem;
    using GenSchemaPtr = commsdsl::gen::GenSchemaPtr;
    using GenNamespacePtr = commsdsl::gen::GenNamespacePtr;
    using GenMessagePtr = commsdsl::gen::GenMessagePtr;
    using GenFramePtr = commsdsl::gen::GenFramePtr;
    using GenProgramOptions = commsdsl::gen::GenProgramOptions;
    using GenGenerator = commsdsl::gen::GenGenerator;
    using GenFieldPtr = commsdsl::gen::GenFieldPtr;

    static const std::string& latexFileGeneratedComment();
    static const std::string& latexCodeInjectCommentPrefix();
    static const std::string& latexSchemaCommentPrefix();
    static std::string latexWrapInput(const std::string& filePath);
    static void latexWrapInputInPlace(std::string& filePath);
    static const std::string& latexSectionDirective(const GenElem& elem);
    static std::string latexLabelId(const GenElem& elem);
    static std::string latexEscString(const std::string& str);
    static std::string latexEscDisplayName(const std::string& displayName, const std::string& name);
    static void latexEnsureNewLineBreak(std::string& str);
    static std::string latexIntegralToStr(std::intmax_t value, bool isUnsigned, std::size_t hexWidth = 0);
    static std::string latexIntegralToStr(std::uintmax_t value, std::size_t hexWidth = 0);

    static LatexGenerator& latexCast(GenGenerator& generator)
    {
        return static_cast<LatexGenerator&>(generator);
    }

    static const LatexGenerator& latexCast(const GenGenerator& generator)
    {
        return static_cast<const LatexGenerator&>(generator);
    }

    std::string latexRelPathFor(const GenElem& elem) const;

    std::string latexInputCodePathForFile(const std::string& name) const;

    bool latexHasCodeInjectionComments() const
    {
        return m_hasCodeInjectionComments;
    }

protected:
    virtual bool genWriteImpl() override;

    virtual GenSchemaPtr genCreateSchemaImpl(ParseSchema parseObj, GenElem* parent) override;
    virtual GenNamespacePtr genCreateNamespaceImpl(ParseNamespace parseObj, GenElem* parent) override;
    virtual GenMessagePtr genCreateMessageImpl(ParseMessage parseObj, GenElem* parent) override;
    virtual GenFramePtr genCreateFrameImpl(ParseFrame parseObj, GenElem* parent) override;

    virtual GenFieldPtr genCreateIntFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateEnumFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateSetFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateFloatFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateBitfieldFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateBundleFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateStringFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateDataFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateListFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateRefFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateOptionalFieldImpl(ParseField parseObj, GenElem* parent) override;
    virtual GenFieldPtr genCreateVariantFieldImpl(ParseField parseObj, GenElem* parent) override;

    virtual OptsProcessResult genProcessOptionsImpl(const GenProgramOptions& options) override;

private:
    bool latexWriteExtraFilesInternal() const;

    bool m_hasCodeInjectionComments = false;
};

} // namespace commsdsl2latex
