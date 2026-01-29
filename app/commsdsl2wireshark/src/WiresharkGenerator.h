//
// Copyright 2026 - 2026 (C). Alex Robenko. All rights reserved.
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

namespace commsdsl2wireshark
{

class WiresharkGenerator final : public commsdsl::gen::GenGenerator
{
    using GenBase = commsdsl::gen::GenGenerator;

public:
    using ParseNamespace = commsdsl::parse::ParseNamespace;
    using ParseSchema = commsdsl::parse::ParseSchema;
    using ParseFrame = commsdsl::parse::ParseFrame;

    using GenElem = commsdsl::gen::GenElem;
    using GenSchemaPtr = commsdsl::gen::GenSchemaPtr;
    using GenNamespacePtr = commsdsl::gen::GenNamespacePtr;
    using GenMessagePtr = commsdsl::gen::GenMessagePtr;
    using GenFramePtr = commsdsl::gen::GenFramePtr;
    using GenProgramOptions = commsdsl::gen::GenProgramOptions;
    using GenGenerator = commsdsl::gen::GenGenerator;
    using GenFieldPtr = commsdsl::gen::GenFieldPtr;

    WiresharkGenerator();

    static WiresharkGenerator& wiresharkCast(commsdsl::gen::GenGenerator& generator)
    {
        return static_cast<WiresharkGenerator&>(generator);
    }

    static const WiresharkGenerator& wiresharkCast(const commsdsl::gen::GenGenerator& generator)
    {
        return static_cast<const WiresharkGenerator&>(generator);
    }

    static const std::string& wiresharkFileGeneratedComment();

    std::string wiresharkScopeToName(const std::string& scope) const;
    std::string wiresharkDissectNameFor(const GenElem& elem) const;

    std::string wiresharkInputRelPathPrefix() const;
    std::string wiresharkInputRelPathFor(const GenElem& elem) const;
    std::string wiresharkInputAbsPathFor(const GenElem& elem) const;
    std::string wiresharkInputRelPathFor(const std::string& name) const;
    std::string wiresharkInputAbsPathFor(const std::string& name) const;

protected:
    virtual GenSchemaPtr genCreateSchemaImpl(ParseSchema parseObj, GenElem* parent) override;
    virtual GenNamespacePtr genCreateNamespaceImpl(ParseNamespace parseObj, GenElem* parent) override;
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

    virtual bool genWriteImpl() override;
    virtual const std::string& genCommentPrefixImpl() const override;

private:
    bool wiresharkWriteExtraFilesInternal() const;
};

} // namespace commsdsl2wireshark
